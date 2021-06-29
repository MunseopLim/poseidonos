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

#include "io_locker.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
void
IOLocker::Register(string array)
{
    if (_Find(array) == nullptr)
    {
        StripeLocker* locker = new StripeLocker();
        lockers.emplace(array, locker);
    }
}

void
IOLocker::Unregister(string array)
{
    StripeLocker* locker = _Find(array);
    if (locker != nullptr)
    {
        _Erase(array);
        delete locker;
        locker = nullptr;
    }
}

bool
IOLocker::TryLock(string array, StripeId val)
{
    StripeLocker* locker = _Find(array);
    if (locker == nullptr)
    {
        return false;
    }

    return locker->TryLock(val);
}

void
IOLocker::Unlock(string array, StripeId val)
{
    StripeLocker* locker = _Find(array);
    if (locker != nullptr)
    {
        locker->Unlock(val);
    }
}

bool
IOLocker::TryChange(string array, LockerMode mode)
{
    StripeLocker* locker = _Find(array);
    if (locker == nullptr)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
            "Locker not found, fatal error if array is not broken");
        return true;
    }

    return locker->TryModeChanging(mode);
}

StripeLocker*
IOLocker::_Find(string array)
{
    if (array == "" && lockers.size() == 1)
    {
        return lockers.begin()->second;
    }
    auto it = lockers.find(array);
    if (it == lockers.end())
    {
        return nullptr;
    }

    return it->second;
}

void
IOLocker::_Erase(string array)
{
    if (array == "" && lockers.size() == 1)
    {
        lockers.clear();
    }
    else
    {
        lockers.erase(array);
    }
}
} // namespace pos
