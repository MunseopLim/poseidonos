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
#include <tuple>
#include <vector>

#include "src/allocator/include/allocator_const.h"
#include "src/mapper/reversemap/reverse_map.h"

namespace pos
{
using DataBufferIter = std::vector<void*>::iterator;

class Stripe
{
public:
    Stripe(void) = default;
    explicit Stripe(bool withDataBuffer);
    Stripe(bool withDataBuffer, std::string arrayName);
    virtual ~Stripe(void);
    virtual void Assign(StripeId vsid, StripeId lsid, ASTailArrayIdx tailarrayidx);

    virtual uint32_t GetAsTailArrayIdx(void);
    virtual StripeId GetVsid(void);
    virtual void SetVsid(StripeId virtsid);

    virtual StripeId GetWbLsid(void);
    virtual void SetWbLsid(StripeId wbAreaLsid);

    virtual StripeId GetUserLsid(void);
    virtual void SetUserLsid(StripeId userAreaLsid);

    virtual int Flush(EventSmartPtr callback);
    virtual void UpdateReverseMap(uint32_t offset, BlkAddr rba, uint32_t volumeId);
    virtual int ReconstructReverseMap(uint32_t volumeId, uint64_t blockCount);
    virtual int LinkReverseMap(ReverseMapPack* revMapPackToLink);
    virtual int UnLinkReverseMap(void);
    virtual std::tuple<BlkAddr, uint32_t> GetReverseMapEntry(uint32_t offset);
    virtual void UpdateVictimVsa(uint32_t offset, VirtualBlkAddr vsa);
    virtual VirtualBlkAddr GetVictimVsa(uint32_t offset);

    virtual bool IsFinished(void);
    virtual void SetFinished(bool state);

    virtual uint32_t GetBlksRemaining(void);
    virtual uint32_t DecreseBlksRemaining(uint32_t amount);

    virtual void Refer(void);
    virtual void Derefer(uint32_t blockCount);
    virtual bool IsOkToFree(void);

    virtual void AddDataBuffer(void* buf);
    virtual DataBufferIter DataBufferBegin(void);
    virtual DataBufferIter DataBufferEnd(void);
    virtual bool IsGcDestStripe(void);

private:
    ASTailArrayIdx asTailArrayIdx;
    StripeId vsid; // SSD LSID, Actually User Area LSID
    StripeId wbLsid;
    StripeId userLsid;
    ReverseMapPack* revMapPack;

    std::atomic<bool> finished;
    std::atomic<uint32_t> remaining; // #empty block(s) left, on this stripe
    std::atomic<uint32_t> referenceCount;
    std::vector<void*> dataBuffer;
    std::vector<VirtualBlkAddr> oldVsaList;
    uint32_t totalBlksPerUserStripe;
    bool withDataBuffer;
};

} // namespace pos
