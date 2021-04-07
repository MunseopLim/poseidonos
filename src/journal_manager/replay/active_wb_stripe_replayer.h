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

#include <map>
#include <vector>

#include "active_stripe_address.h"
#include "pending_stripe.h"
#include "src/allocator/active_stripe_index_info.h"
#include "src/include/address_type.h"
#include "stripe_info.h"

namespace ibofos
{
class Allocator;
class Array;
class ReplayStripe;

class ActiveWBStripeReplayer
{
public:
    ActiveWBStripeReplayer(Allocator* allocator, PendingStripeList& pendingStripeList);
    virtual ~ActiveWBStripeReplayer(void);

    int Replay(void);
    void Update(StripeInfo info);

private:
    int _FindWbufIndex(StripeInfo stripeInfo);
    void _AddToPendingFlushList(int volId, StripeId wbLsid, VirtualBlkAddr tail);
    void _ResetWbufTail(int index);
    void _UpdateWbufTail(int index, ActiveStripeAddr addr);
    bool _IsFlushedStripe(StripeInfo stripeInfo);
    ActiveStripeAddr _ReplayStripesExceptActive(int index);

    static const int INDEX_NOT_FOUND = -1;

    using PendingActiveStripeList = std::vector<ActiveStripeAddr>;

    std::vector<VirtualBlkAddr> readTails;
    std::vector<PendingActiveStripeList> foundActiveStripes;

    PendingStripeList& pendingStripes;
    Allocator* allocator;
};

} // namespace ibofos
