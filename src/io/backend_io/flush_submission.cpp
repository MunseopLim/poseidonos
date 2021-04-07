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

#include "flush_submission.h"

#include <list>

#include "src/allocator/allocator.h"
#include "src/allocator/stripe.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_request.h"

namespace ibofos
{
#if defined QOS_ENABLED_BE
FlushSubmission::FlushSubmission(Stripe* inputStripe)
: Event(false, BackendEvent_Flush),
  stripe(inputStripe)
{
    SetEventType(BackendEvent_Flush);
}
#else
FlushSubmission::FlushSubmission(Stripe* inputStripe)
: Event(false),
  stripe(inputStripe)
{
}
#endif

FlushSubmission::~FlushSubmission(void)
{
}

bool
FlushSubmission::Execute(void)
{
    Allocator& allocator = *AllocatorSingleton::Instance();
    StripeId logicalStripeId = allocator.AllocateUserDataStripeId(stripe->GetVsid());

    std::list<BufferEntry> bufferList;
    uint64_t blocksInStripe = 0;
    for (auto it = stripe->DataBufferBegin(); it != stripe->DataBufferEnd(); ++it)
    {
        BufferEntry bufferEntry(*it, BLOCKS_IN_CHUNK);
        bufferList.push_back(bufferEntry);
        blocksInStripe += BLOCKS_IN_CHUNK;
    }

    stripe->SetUserLsid(logicalStripeId);
    CallbackSmartPtr callback(new StripeMapUpdateRequest(stripe));

    LogicalBlkAddr startLSA = {
        .stripeId = logicalStripeId,
        .offset = 0};

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::FLUSH_DEBUG_SUBMIT;

    IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, IbofEventId::GetString(eventId), stripe->GetVsid(), startLSA.stripeId, blocksInStripe);

    IOSubmitHandlerStatus errorReturned = IOSubmitHandler::SubmitAsyncIO(
        IODirection::WRITE,
        bufferList,
        startLSA, blocksInStripe,
        USER_DATA,
        callback);

    return (IOSubmitHandlerStatus::SUCCESS == errorReturned || IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP == errorReturned);
}
} // namespace ibofos
