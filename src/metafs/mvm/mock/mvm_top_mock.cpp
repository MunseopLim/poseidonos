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

#include "mvm_top_mock.h"

#include "mdpage.h"
#include "meta_volume.h"
#include "metafs_config.h"

namespace pos
{
MockMetaVolManager mockMetaVolMgr;
MetaFsMVMTopManager& mvmTopMgr = mockMetaVolMgr;

MockMetaVolManager::MockMetaVolManager(void)
{
}

MockMetaVolManager&
MockMetaVolManager::GetInstance(void)
{
    return mockMetaVolMgr;
}

void
MockMetaVolManager::Init(MetaVolumeType volType, MetaLpnType maxVolPageNum)
{
    SetModuleInit();
}

bool
MockMetaVolManager::Bringup(void)
{
    SetModuleReady();
    return true;
}

bool
MockMetaVolManager::Open(bool isNPOR)
{
    return true;
}

bool
MockMetaVolManager::Close(bool& resetCxt /*output*/)
{
    SetModuleInactive();

    resetCxt = true;
    return true;
}

bool
MockMetaVolManager::CreateVolume(MetaVolumeType volType)
{
    return true;
}

POS_EVENT_ID
MockMetaVolManager::ProcessNewReq(MetaFsFileControlRequest& reqMsg)
{
    if (reqMsg.reqType == MetaFsFileControlType::FileCreate)
    {
        _CreateDummyFile(reqMsg.fileByteSize);
    }
    else if (reqMsg.reqType == MetaFsFileControlType::FileOpen)
    {
        reqMsg.completionData.openfd = dummyInode.data.basic.field.fd;
    }
    else if (reqMsg.reqType == MetaFsFileControlType::GetDataChunkSize)
    {
        reqMsg.completionData.dataChunkSize = dummyInode.data.basic.field.dataChunkSize;
    }
    else if (reqMsg.reqType == MetaFsFileControlType::GetFileSize)
    {
        reqMsg.completionData.fileSize = dummyInode.data.basic.field.fileByteSize;
    }
    else if (reqMsg.reqType == MetaFsFileControlType::GetTargetMediaType)
    {
        reqMsg.completionData.targetMediaType = dummyInode.data.basic.field.ioAttribute.media;
    }
    else if (reqMsg.reqType == MetaFsFileControlType::GetFileBaseLpn)
    {
        reqMsg.completionData.fileBaseLpn = dummyInode.data.basic.field.pagemap.baseMetaLpn;
    }
    else
    {
        // you hit here because you haven't implement code to handle given reqType above
        assert(false);
    }
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MockMetaVolManager::_CreateDummyFile(FileSizeType fileByteSize)
{
    FileDescriptorType NewFd = 0;

    _CreateFileInode(NewFd, fileByteSize);

    return POS_EVENT_ID::SUCCESS;
}

void
MockMetaVolManager::_CreateFileInode(FileDescriptorType fd, FileSizeType fileByteSize)
{
    dummyInode.data.basic.field.fileByteSize = fileByteSize;
    dummyInode.data.basic.field.fd = fd;
    dummyInode.data.basic.field.dataChunkSize = _GetDefaultDataChunkSize();
    dummyInode.data.basic.field.ioAttribute.media = MetaStorageType::SSD;
    dummyInode.data.basic.field.pagemap.baseMetaLpn = 0;
    dummyInode.data.basic.field.pagemap.pageCnt = (fileByteSize + dummyInode.data.basic.field.dataChunkSize - 1) / dummyInode.data.basic.field.dataChunkSize;
}

uint32_t
MockMetaVolManager::_GetDefaultDataChunkSize(void)
{
    return MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}
} // namespace pos
