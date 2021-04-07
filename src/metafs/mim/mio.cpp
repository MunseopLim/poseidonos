/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mio.h"

#include "instance_tagid_allocator.h"
#include "mfs_common.h"
#include "mfs_mvm_top.h"
#include "mpio.h"
#include "mvm_req.h"
#include "read_mpio.h"
#include "write_mpio.h"

const MetaIoOpcode Mio::ioOpcodeMap[] = {MetaIoOpcode::Write, MetaIoOpcode::Read};

InstanceTagIdAllocator mioTagIdAllocator;

Mio::Mio(MpioPool* mpioPool)
: originReq(nullptr),
  fileDataChunkSize(0),
  error(0, false),
  retryFlag(false),
  ioCQ(nullptr),
  mpioPool(nullptr),
  userIo(nullptr)
{
    opCode = MetaIoOpcode::Max;
    startLpn = 0;
    numLpns = 0;

    mpioAsyncDoneCallback = AsEntryPointParam1(&Mio::_HandleMpioDone, this);
    _BindMpioPool(mpioPool);
}

Mio::~Mio(void)
{
}

void
Mio::Setup(MetaFsIoReqMsg* ioReq, MetaLpnType baseLpn)
{
    assert(mpioPool != nullptr);

    originReq = ioReq;

    fileDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    opCode = ioOpcodeMap[static_cast<uint32_t>(originReq->reqType)];
    startLpn = baseLpn + (originReq->byteOffsetInFile / fileDataChunkSize);
    numLpns = 1 + ((originReq->byteOffsetInFile + originReq->byteSize) / fileDataChunkSize);
    userIo = ioReq->userIo;

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mio ][SetupMio   ] type={}, req.tagId={}, fileOffset={}, baseLpn={}, startLpn={}",
        originReq->reqType, originReq->tagId, originReq->byteOffsetInFile, baseLpn, startLpn);
}

void
Mio::Reset(void)
{
    MetaAsyncRunnableClass<MetaAsyncCbCxt, MioState, MioStateExecuteEntry>::Init();
    fileDataChunkSize = 0;
    error = std::make_pair(0, false);
    retryFlag = false;

    if (originReq)
    {
        delete originReq;
        originReq = nullptr;
    }
    mpioListCxt.Reset();
}

void
Mio::InitStateHandler(void)
{
    RegisterStateHandler(MioState::Init,
        new MioStateExecuteEntry(MioState::Init, AsMioStateEntryPoint(&Mio::Init, this), MioState::Issued));
    RegisterStateHandler(MioState::Issued,
        new MioStateExecuteEntry(MioState::Issued, AsMioStateEntryPoint(&Mio::Issue, this), MioState::Complete));
    RegisterStateHandler(MioState::Complete,
        new MioStateExecuteEntry(MioState::Complete, AsMioStateEntryPoint(&Mio::Complete, this), MioState::Complete));
}

uint32_t
Mio::GetOriginReqID(void)
{
    return originReq->tagId;
}

void
Mio::_BindMpioPool(MpioPool* mpioPool)
{
    assert(this->mpioPool == nullptr && mpioPool != nullptr);
    this->mpioPool = mpioPool;
}

void
Mio::SetMpioDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier)
{
    this->partialMpioDoneNotifier = partialMpioDoneNotifier;
}

void
Mio::SetMpioDonePoller(mpioDonePollerCb& mpioDonePoller)
{
    this->mpioDonePoller = mpioDonePoller;
}

void
Mio::SetIoCQ(MetaIoQClass<Mio*>* ioCQ)
{
    this->ioCQ = ioCQ;
}

void
Mio::_HandleMpioDone(void* data)
{
    Mpio* mpio = reinterpret_cast<Mpio*>(data);
    assert(mpio->io.opcode < MetaIoOpcode::Max);

    _FinalizeMpio(*mpio);

    if (mpioListCxt.IsAllMpioDone())
    {
        SetNextState(MioState::Complete);
        ExecuteAsyncState();
    }
}

FileSizeType
Mio::GetIOSize(void)
{
    return originReq->byteSize;
}

bool
Mio::IsTargetToSSD(void)
{
    return originReq->targetMediaType == MetaStorageType::SSD;
}

bool
Mio::IsRead(void)
{
    return originReq->reqType == MetaIoReqTypeEnum::Read;
}

MpioType
Mio::_LookupMpioType(MetaIoReqTypeEnum type)
{
    switch (type)
    {
        case MetaIoReqTypeEnum::Read:
            return MpioType::Read;
        case MetaIoReqTypeEnum::Write:
            return MpioType::Write;
        default:
            assert(false);
    }
}

void
Mio::SetLocalAioCbCxt(MioAsyncDoneCb& callback)
{
    aioCbCxt.Init(this, callback);
    SetAsyncCbCxt(&aioCbCxt, false);
}

Mpio&
Mio::_AllocNewMpio(MpioIoInfo& mpioIoInfo, bool partialIO)
{
    assert(mpioPool != nullptr);

    MpioType mpioType = _LookupMpioType(originReq->reqType);

    Mpio* mpio = mpioPool->Alloc(mpioType);
    assert(mpio != nullptr);

    mpio->Setup(originReq->targetMediaType, mpioIoInfo, partialIO, false /*forceSyncIO*/);
    mpio->SetLocalAioCbCxt(mpioAsyncDoneCallback);

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][Alloc      ] type={}, req.tagId={}, mpio_id={}, fileOffsetinChunk={}, fileOffset={}",
        mpioIoInfo.opcode, mpioIoInfo.tagId, mpioIoInfo.mpioId,
        mpioIoInfo.startByteOffset, originReq->byteOffsetInFile);

    return *mpio;
}

void
Mio::_PrepareMpioInfo(MpioIoInfo& mpioIoInfo,
    MetaLpnType lpn, FileSizeType byteOffset, FileSizeType byteSize, FileBufType buf, MetaLpnType lpnCnt, uint32_t mpio_id)
{
    mpioIoInfo.opcode = static_cast<MetaIoOpcode>(originReq->reqType);
    mpioIoInfo.targetMediaType = originReq->targetMediaType;
    mpioIoInfo.targetFD = originReq->fd;
    mpioIoInfo.metaLpn = lpn;
    mpioIoInfo.startByteOffset = byteOffset;
    mpioIoInfo.byteSize = byteSize;
    mpioIoInfo.pageCnt = lpnCnt;
    mpioIoInfo.userBuf = buf;
    mpioIoInfo.tagId = originReq->tagId;
    mpioIoInfo.mpioId = mpio_id;
}

// FIXME: for better parallel execution, let's issue io request for each mpio as soon as mio builds mpio contexta
/* 
The host I/O issued as Mio data structure, and the Mio is consisted with several Mpio moudled by fileDataChunkSize(4032Byte) that the other 64Byte is page control info such as signature, version, and so on. 
*/
void
Mio::_BuildMpioMap(void)
{
    uint32_t totalMpioCnt, curLpn, remainingBytes, byteOffsetInChunk, byteSize;
    FileBufType curUserBuf;

    remainingBytes = originReq->byteSize;
    curLpn = startLpn;
    byteOffsetInChunk = originReq->byteOffsetInFile % fileDataChunkSize;
    curUserBuf = originReq->buf;

    if (byteOffsetInChunk + originReq->byteSize < fileDataChunkSize)
    {
        byteSize = originReq->byteSize;
        totalMpioCnt = 1;
    }
    else
    {
        byteSize = fileDataChunkSize - byteOffsetInChunk;
        if (0 == byteOffsetInChunk)
        {
            totalMpioCnt = (remainingBytes + fileDataChunkSize - 1) / fileDataChunkSize;
        }
        else
        {
            totalMpioCnt = (((remainingBytes - byteSize) + fileDataChunkSize - 1) / fileDataChunkSize) + 1;
        }
    }
    mpioListCxt.SetTotalMpioCntForExecution(totalMpioCnt);

    MpioType ioType = _LookupMpioType(originReq->reqType);

    uint32_t mpio_cnt = 0;
    // issue Mpios
    do
    {
        // no more Mpio operations
        while (mpioPool->IsEmpty(ioType))
        {
            // Complete Process, MpioHandler::BottomhalfMioProcessing()
            mpioDonePoller();
        }

        // Build Mpio data structure
        MpioIoInfo mpioIoInfo;
        _PrepareMpioInfo(mpioIoInfo, curLpn, byteOffsetInChunk, byteSize, curUserBuf, 1 /* LpnCnt */, mpio_cnt++);

        Mpio& mpio = _AllocNewMpio(mpioIoInfo, byteSize != fileDataChunkSize /* partialIO */);
        mpio.SetPartialDoneNotifier(partialMpioDoneNotifier);
        mpioListCxt.PushMpio(mpio);

        mpio.ExecuteAsyncState();

        curLpn++;
        curUserBuf = (void*)((uint8_t*)curUserBuf + byteSize);
        remainingBytes -= byteSize;
        byteOffsetInChunk = 0;

        if (remainingBytes >= fileDataChunkSize)
        {
            byteSize = fileDataChunkSize;
        }
        else
        {
            byteSize = remainingBytes;
        }
    } while (remainingBytes);
}

uint32_t
Mio::_GetDataChunkSize(void)
{
    MetaFsMoMReqMsg req;
    req.reqType = MetaFsMoMReqType::GetDataChunkSize;
    req.fd = originReq->fd;

    mvmTopMgr.ProcessNewReq(req);
    return req.completionData.dataChunkSize;
}

MetaLpnType
Mio::GetStartLpn(void)
{
    return startLpn;
}

MetaLpnType
Mio::GetLpnCnt(void)
{
    return numLpns;
}

bool
Mio::IsTargetStorageSSD(void)
{
    return originReq->targetMediaType == MetaStorageType::SSD;
}

void*
Mio::GetClientAioCbCxt(void)
{
    return originReq->aiocb;
}

bool
Mio::IsSyncIO(void)
{
    return originReq->ioMode == MetaIoModeEnum::Sync;
}

void
Mio::_MarkMpioComplete(Mpio& mpio)
{
    SPIN_LOCK_GUARD_IN_SCOPE(mpioListCxtLock);
    mpioListCxt.MarkMpioCompletion(mpio);
}

const MetaIoOpcode
Mio::GetOpCode(void)
{
    return opCode;
}

const FileFDType
Mio::GetFD(void)
{
    return originReq->fd;
}

uint32_t
Mio::GetFileDataChunkSize(void)
{
    return fileDataChunkSize;
}

bool
Mio::Init(MioState expNextState)
{
    SetNextState(expNextState);
    return true;
}

bool
Mio::Issue(MioState expNextState)
{
    _BuildMpioMap();
    SetNextState(expNextState);

    return false; // not continue to execute. bottom half procedure will dispatch pending mpio
}

MfsError
Mio::GetError(void)
{
    return error;
}

void
Mio::_FinalizeMpio(Mpio& mpio)
{
    // FIXME: obtain last error code (Need to deal with another approach?)

    MfsError rc;
    rc = mpio.GetErrorStatus();
    if (rc.first != 0 || rc.second == true)
    {
        this->error = mpio.GetErrorStatus();
    }

    _MarkMpioComplete(mpio);
}

bool
Mio::Complete(MioState expNextState)
{
    SetNextState(expNextState);

    ioCQ->Enqueue(this);

    return true;
}

void
Mio::NotifyCompletionToClient(void)
{
    if (error.first != 0 || error.second == true)
    {
        originReq->originalMsg->SetError(true);
    }

    originReq->originalMsg->NotifyIoCompletionToClient();
}

bool
Mio::IsFirstAttempt(void)
{
    return retryFlag == false;
}

void
Mio::SetRetryFlag(void)
{
    retryFlag = true;
}
