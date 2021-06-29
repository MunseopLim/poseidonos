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
#include <cstdint>

#include "event.h"
#include "src/include/io_error_type.h"
#include "src/include/smart_ptr_type.h"
#include "src/lib/bitmap.h"

namespace pos
{
class SystemTimeoutChecker;

class Callback : public Event
{
public:
    Callback(bool isFrontEnd, uint32_t weight = 1);
    ~Callback(void) override;

    bool Execute(void) final;
    void SetWaitingCount(uint32_t inputWaitingCount);
    void SetCallee(CallbackSmartPtr callee);
    void InformError(IOErrorType inputIOErrorType);

protected:
    virtual uint32_t _GetErrorCount(void);
    IOErrorType _GetMostCriticalError(void);

private:
    bool _IsInvalidCallee(CallbackSmartPtr inputCallee);
    bool _IsCalleeSet(void);
    virtual bool _DoSpecificJob(void) = 0;
    void _InvokeCallee(void);
    virtual bool _RecordCallerCompletionAndCheckOkToCall(uint32_t transferredErrorCount,
        BitMapMutex& inputErrorBitMap, uint32_t transferredWeight);

    std::atomic<uint32_t> errorCount;
    BitMapMutex errorBitMap;
    std::atomic<uint32_t> completionCount;
    uint32_t waitingCount;
    uint32_t weight;
    CallbackSmartPtr callee;
    SystemTimeoutChecker* timeoutChecker;
    void* returnAddress;

    static const uint32_t CALLER_FRAME;
    static const uint64_t DEFAULT_TIMEOUT_NS;
};
} // namespace pos
