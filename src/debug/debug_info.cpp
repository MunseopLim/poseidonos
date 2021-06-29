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

#include "src/debug/debug_info.h"

#include <vector>

#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/device/device_manager.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/uram/uram_drv.h"
#include "src/dump/dump_manager.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/io/general_io/command_timeout_handler.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/master_context/config_manager.h"
#include "src/mbr/mbr_manager.h"
#include "src/master_context/version_provider.h"
#include "src/metafs/include/metafs_service.h"
#include "src/qos/qos_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_service.h"

namespace pos
{

DebugInfo* debugInfo;

DebugInfo::DebugInfo(void)
:   affinityManager(nullptr),
    allocatorService(nullptr),
    arrayManager(nullptr),
    commandTimeoutHandler(nullptr),
    configManager(nullptr),
    deviceManager(nullptr),
    dumpManager(nullptr),
    eventScheduler(nullptr),
    flushCmdManager(nullptr),
    garbageCollector(nullptr),
    ioDispatcher(nullptr),
    logger(nullptr),
    mapperService(nullptr),
    metaFsService(nullptr),
    qosManager(nullptr),
    rbaStateService(nullptr),
    reporter(nullptr),
    spdk(nullptr),
    unvmeDrv(nullptr),
    uramDrv(nullptr),
    versionProvider(nullptr),
    volumeEventPublisher(nullptr),
    volumeService(nullptr)
{
}

DebugInfo::~DebugInfo(void)
{
}

void
DebugInfo::Update(void)
{
    affinityManager = AffinityManagerSingleton::Instance();
    deviceManager = DeviceManagerSingleton::Instance();
    reporter = ReporterSingleton::Instance();
    logger = LoggerSingleton::Instance();
    dumpManager = DumpManagerSingleton::Instance();
    mapperService = MapperServiceSingleton::Instance();
    arrayManager = ArrayMgr::Instance();
    allocatorService = AllocatorServiceSingleton::Instance();
    uramDrv = UramDrvSingleton::Instance();
    unvmeDrv = UnvmeDrvSingleton::Instance();
    configManager = ConfigManagerSingleton::Instance();
    spdk = SpdkSingleton::Instance();
    rbaStateService = RBAStateServiceSingleton::Instance();
    volumeEventPublisher = VolumeEventPublisherSingleton::Instance();
    eventScheduler = EventSchedulerSingleton::Instance();
    ioDispatcher = IODispatcherSingleton::Instance();
    qosManager = QosManagerSingleton::Instance();
    flushCmdManager = FlushCmdManagerSingleton::Instance();
    versionProvider = VersionProviderSingleton::Instance();
    commandTimeoutHandler = CommandTimeoutHandlerSingleton::Instance();
    metaFsService = MetaFsServiceSingleton::Instance();
    volumeService = VolumeServiceSingleton::Instance();
}
} // namespace pos
