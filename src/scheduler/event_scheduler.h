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

#include <sched.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#if defined QOS_ENABLED_BE
#include <queue>

#include "src/scheduler/event.h"
#endif

#include "event.h"

namespace ibofos
{
class Event;
class ISchedulerPolicy;
class EventQueue;
class EventWorker;
class QosManager;
class SchedulerQueue;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Schedule events to Event worker
 *           Single core dedicated
 */
/* --------------------------------------------------------------------------*/
class EventScheduler
{
public:
    EventScheduler(uint32_t workerCountInput,
        cpu_set_t schedulerCPUInput,
        cpu_set_t eventCPUSetInput);
    ~EventScheduler(void);

    void Initialize(void);
    void EnqueueEvent(EventSmartPtr input);
#if defined QOS_ENABLED_BE
    std::mutex queueLock[BackendEvent_Count];
    std::queue<EventSmartPtr> DequeueEvents(void);
#endif
    void Run(void);

    Event* GetNextEvent(void);

private:
    void _BuildCpuSet(cpu_set_t& cpuSet);
    ISchedulerPolicy* policy;
#if defined QOS_ENABLED_BE
    SchedulerQueue* eventQueue[BackendEvent_Count];
    int32_t oldWeight[BackendEvent_Count] = {0};
    int32_t runningWeight[BackendEvent_Count] = {0};
#else
    EventQueue* eventQueue;
#endif
    std::atomic<bool> exit;
    uint32_t workerCount;
    std::vector<EventWorker*> workerArray;
    std::thread* schedulerThread;
    cpu_set_t schedulerCPUSet;
    std::vector<cpu_set_t> cpuSetVector;
};

} // namespace ibofos
