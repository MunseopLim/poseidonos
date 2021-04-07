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

#include "stripe.h"

#include "src/array/array.h"
#include "src/device/ioat_api.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.h"
#include "src/include/meta_const.h"
#include "src/volume/volume_list.h"

namespace ibofos
{
Stripe::Stripe(void)
: asTailArrayIdx(UINT32_MAX),
  vsid(UINT32_MAX),
  wbLsid(UINT32_MAX),
  userLsid(UINT32_MAX),
  revMapPack(nullptr),
  finished(true),
  remaining(0),
  referenceCount(0)
{
}

void
Stripe::UpdateReverseMap(uint32_t offset, BlkAddr rba, VolumeId volumeId)
{
    if (rba != INVALID_RBA && volumeId >= MAX_VOLUME_COUNT)
    {
        throw IBOF_EVENT_ID::STRIPE_INVALID_VOLUME_ID;
    }
    revMapPack->SetReverseMapEntry(offset, rba, volumeId);
}

int
Stripe::LinkReverseMap(ReverseMapPack* revMapPackToLink)
{
    if (unlikely(revMapPack != nullptr))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REVMAP_PACK_ALREADY_LINKED,
            "Stripe object for wbLsid:{} is already linked to ReverseMapPack",
            wbLsid);
        return -(int)IBOF_EVENT_ID::REVMAP_PACK_ALREADY_LINKED;
    }

    revMapPack = revMapPackToLink;
    return 0;
}

int
Stripe::UnLinkReverseMap(void)
{
    if (likely(revMapPack != nullptr))
    {
        revMapPack->UnLinkVsid();
        revMapPack = nullptr;
        return 0;
    }
    else
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP,
            "Stripe object for wbLsid:{} is not linked to reversemap but tried to Unlink",
            wbLsid);
        return -(int)IBOF_EVENT_ID::ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP;
    }
}

void
Stripe::Assign(StripeId inputVsid, StripeId inputLsid)
{
    vsid = inputVsid;
    wbLsid = inputLsid;
    Array& arrayManager = *ArraySingleton::Instance();
    const PartitionLogicalSize* udSize =
        arrayManager.GetSizeInfo(PartitionType::USER_DATA);
    uint32_t totalBlksPerUserStripe = udSize->blksPerStripe;

    remaining.store(totalBlksPerUserStripe, memory_order_release);
    finished = false;
}

bool
Stripe::IsOkToFree(void)
{
    bool isOkToFree = (0 == referenceCount);
    return isOkToFree;
}

void
Stripe::Derefer(uint32_t blockCount)
{
    if (unlikely(blockCount > referenceCount))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::ALLOCATOR_WRONG_STRIPE_REFERENCE_COUNT;
        IBOF_TRACE_ERROR((int)eventId, "Wrong stripe reference count");
        referenceCount = 0;
        return;
    }
    referenceCount -= blockCount;
}

void
Stripe::Refer(void)
{
    referenceCount++;
}

StripeId
Stripe::GetWbLsid(void)
{
    return wbLsid;
}

void
Stripe::SetWbLsid(StripeId wbAreaLsid)
{
    wbLsid = wbAreaLsid;
}

StripeId
Stripe::GetUserLsid()
{
    return userLsid;
}

void
Stripe::SetUserLsid(StripeId userAreaLsid)
{
    userLsid = userAreaLsid;
}

StripeId
Stripe::GetVsid()
{
    return vsid;
}

void
Stripe::SetVsid(StripeId virtsid)
{
    vsid = virtsid;
}

void
Stripe::SetAsTailArrayIdx(ASTailArrayIdx idx)
{
    asTailArrayIdx = idx;
}

uint32_t
Stripe::GetAsTailArrayIdx(void)
{
    return asTailArrayIdx;
}

int
Stripe::Flush(EventSmartPtr callback)
{
    if (likely(revMapPack != nullptr))
    {
        return revMapPack->Flush(this, callback);
    }
    else
    {
        IBOF_TRACE_ERROR(EID(ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP),
            "Stripe object for wbLsid:{} is not linked to reversemap but tried to Flush",
            wbLsid);
        return -EID(ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP);
    }
}

DataBufferIter
Stripe::DataBufferBegin(void)
{
    return dataBuffer.begin();
}

DataBufferIter
Stripe::DataBufferEnd(void)
{
    return dataBuffer.end();
}

void
Stripe::AddDataBuffer(void* buf)
{
    dataBuffer.push_back(buf);
}

uint32_t
Stripe::GetBlksRemaining(void)
{
    return remaining.load(memory_order_acquire);
}

uint32_t
Stripe::DecreseBlksRemaining(uint32_t amount)
{
    uint32_t remainingBlocksAfterDecrease =
        remaining.fetch_sub(amount, memory_order_acq_rel) - amount;
    return remainingBlocksAfterDecrease;
}

bool
Stripe::IsFinished(void)
{
    return (finished == true);
}

void
Stripe::SetFinished(bool state)
{
    finished = state;
}

int
Stripe::ReconstructReverseMap(VolumeId volumeId, uint64_t blockCount)
{
    return revMapPack->ReconstructMap(volumeId, vsid, wbLsid, blockCount);
}

} // namespace ibofos
