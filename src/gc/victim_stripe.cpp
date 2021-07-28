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

#include "src/gc/victim_stripe.h"
#include "src/mapper/include/mapper_const.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/event_scheduler/callback.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/include/meta_const.h"
#include "src/volume/volume_service.h"
#include "src/include/branch_prediction.h"

namespace pos
{
VictimStripe::VictimStripe(IArrayInfo* array, ReverseMapPack* revMapPack_)
: myLsid(UNMAP_STRIPE),
  dataBlks(0),
  chunkIndex(0),
  blockOffset(0),
  validBlockCnt(0),
  isLoaded(false),
  array(array),
  revMapPack(revMapPack_)
{
    dataBlks = array->GetSizeInfo(PartitionType::USER_DATA)->blksPerStripe;
    if (nullptr == revMapPack)
    {
        IReverseMap* iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(array->GetName());
        revMapPack = iReverseMap->AllocReverseMapPack(false);
    }
}

VictimStripe::~VictimStripe(void)
{
    validBlkInfos.clear();
    if (nullptr != revMapPack)
    {
        delete revMapPack;
    }
}
void
VictimStripe::Load(StripeId _lsid, CallbackSmartPtr callback)
{
    _InitValue(_lsid);
    _LoadReverseMap(callback);
}

void
VictimStripe::_InitValue(StripeId _lsid)
{
    myLsid = _lsid;
    validBlkInfos.clear();
    blkInfoList.clear();

    chunkIndex = 0;
    blockOffset = 0;
    validBlockCnt = 0;
    isLoaded = false;
}

void
VictimStripe::_LoadReverseMap(CallbackSmartPtr callback)
{
    revMapPack->LinkVsid(myLsid);
    revMapPack->Load(callback);
}

bool
VictimStripe::LoadValidBlock(void)
{
    if (isLoaded == true)
    {
        return true;
    }

    IVSAMap* iVSAMap = MapperServiceSingleton::Instance()->GetIVSAMap(array->GetName());
    IStripeMap* iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(array->GetName());
    IVolumeManager* volumeManager
        = VolumeServiceSingleton::Instance()->GetVolumeManager(array->GetName());

    for (; blockOffset < dataBlks; blockOffset++)
    {
        if (chunkIndex != blockOffset / BLOCKS_IN_CHUNK)
        {
            chunkIndex++;
            if (false == blkInfoList.empty())
            {
                validBlkInfos.push_back(blkInfoList);
                blkInfoList.clear();
            }
        }

        BlkInfo blkInfo;
        std::tie(blkInfo.rba, blkInfo.volID) = revMapPack->GetReverseMapEntry(blockOffset);

        if ((MAX_VOLUME_COUNT <= blkInfo.volID) || (INVALID_RBA <= blkInfo.rba))
        {
            continue;
        }


        int shouldRetry = CALLER_EVENT;
        blkInfo.vsa = iVSAMap->GetVSAInternal(blkInfo.volID, blkInfo.rba, shouldRetry);

        if (NEED_RETRY == shouldRetry)
        {
            return false;
        }
        if ((CALLER_EVENT == shouldRetry) && (true == IsUnMapVsa(blkInfo.vsa)))
        {
            continue;
        }

        if ((UNMAP_STRIPE <= blkInfo.vsa.stripeId) || (UNMAP_OFFSET <= blkInfo.vsa.offset))
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::GC_GET_UNMAP_VSA,
                "loaded Unmap VSA, volId:{}, rba:{}, stripeId:{}, vsaOffset:{}",
                blkInfo.volID, blkInfo.rba, blkInfo.vsa.stripeId, blkInfo.vsa.offset);
            continue;
        }

        StripeAddr lsa = iStripeMap->GetLSA(blkInfo.vsa.stripeId);
        if (true == IsUnMapStripe(lsa.stripeId))
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::GC_GET_UNMAP_LSA,
                "Get Unmap LSA, volId:{}, rba:{}, vsaStripeId:{}, vsaOffset:{}, lsaStripeId:{}",
                blkInfo.volID, blkInfo.rba, blkInfo.vsa.stripeId, blkInfo.vsa.offset, lsa.stripeId);
            continue;
        }

        if ((lsa.stripeId == myLsid) && (blockOffset == blkInfo.vsa.offset))
        {
            if (unlikely(static_cast<int>(POS_EVENT_ID::SUCCESS)
                != volumeManager->IncreasePendingIOCountIfNotZero(blkInfo.volID, VolumeStatus::Unmounted)))
            {
                break;
            }

            blkInfoList.push_back(blkInfo);
            validBlockCnt++;
        }
    }

    if (false == blkInfoList.empty())
    {
        validBlkInfos.push_back(blkInfoList);
        blkInfoList.clear();
    }

    isLoaded = true;
    revMapPack->UnLinkVsid();

    return true;
}

} // namespace pos
