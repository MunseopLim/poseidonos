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

#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "src/spdk_wrapper/free_buffer_pool.h"
#include "src/include/address_type.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/lib/bitmap.h"
#include "src/allocator_service/allocator_service.h"
#include "src/gc/victim_stripe.h"

namespace pos
{

class CopierMeta
{
public:
    CopierMeta(IArrayInfo* array, const PartitionLogicalSize* udSize = nullptr,
               BitMapMutex* inUseBitmap_ = nullptr, GcStripeManager* gcStripeManager_ = nullptr,
               std::vector<std::vector<VictimStripe*>>* victimStripes_ = nullptr,
               std::vector<FreeBufferPool*>* gcBufferPool_ = nullptr);
    virtual ~CopierMeta(void);

    virtual void* GetBuffer(StripeId stripeId);
    virtual void ReturnBuffer(StripeId stripeId, void* buffer);

    virtual void SetStartCopyStripes(void);
    virtual void SetStartCopyBlks(uint32_t blocks);
    virtual void SetDoneCopyBlks(uint32_t blocks);
    virtual uint32_t GetStartCopyBlks(void);
    virtual uint32_t GetDoneCopyBlks(void);
    virtual void InitProgressCount(void);

    virtual uint32_t SetInUseBitmap(void);
    virtual bool IsSynchronized(void);
    virtual bool IsAllVictimSegmentCopyDone(void);
    virtual bool IsCopyDone(void);
    virtual bool IsReadytoCopy(uint32_t index);

    virtual uint32_t GetStripePerSegment(void);
    virtual uint32_t GetBlksPerStripe(void);
    virtual VictimStripe* GetVictimStripe(uint32_t victimSegmentIndex, uint32_t stripeOffset);

    virtual GcStripeManager* GetGcStripeManager(void);
    virtual std::string GetArrayName(void);

    static const uint32_t GC_BUFFER_COUNT = 512;
    static const uint32_t GC_CONCURRENT_COUNT = 16;
    static const uint32_t GC_VICTIM_SEGMENT_COUNT = 2;
private:
    void _CreateBufferPool(uint64_t maxBufferCount, uint32_t bufferSize);
    void _CreateVictimStripes(IArrayInfo* array);

    std::atomic<uint32_t> requestStripeCount;
    std::atomic<uint32_t> requestBlockCount;
    std::atomic<uint32_t> doneBlockCount;

    BitMapMutex* inUseBitmap;
    GcStripeManager* gcStripeManager;

    uint32_t stripesPerSegment;
    uint32_t blksPerStripe;

    uint32_t victimSegmentIndex = 0;

    uint32_t copyIndex = 0;
    bool firstGc = true;
    std::atomic_flag copyLock = ATOMIC_FLAG_INIT;
    std::string arrayName;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    std::vector<FreeBufferPool*>* gcBufferPool;
};

} // namespace pos
