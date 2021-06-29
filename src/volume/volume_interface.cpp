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

#include <string>
#include "src/volume/volume_interface.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/sys_info/space_info.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_meta_intf.h"

namespace pos
{

VolumeInterface::VolumeInterface(VolumeList& volumeList, std::string arrayName)
: volumeList(volumeList),
  arrayName(arrayName),
  eventPublisher(VolumeEventPublisherSingleton::Instance())
{
}

VolumeInterface::~VolumeInterface(void)
{
}

int
VolumeInterface::_CheckVolumeSize(uint64_t volumeSize)
{
    if (volumeSize % SZ_1MB != 0 || volumeSize == 0)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_SIZE_NOT_ALIGNED,
            "The requested size, {} is not aligned to MB", volumeSize);
        return (int)POS_EVENT_ID::VOL_SIZE_NOT_ALIGNED;
    }

    if (SpaceInfo::IsEnough(arrayName, volumeSize) == false)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_SIZE_EXCEEDED,
            "The requested volume size is larger than the remaining space");

        return (int)POS_EVENT_ID::VOL_SIZE_EXCEEDED;
    }

    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeInterface::_SetVolumeQos(VolumeBase* volume, uint64_t maxIops,
        uint64_t maxBw)
{
    if (volume == nullptr)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST,
                "The requested volume does not exist");
        return (int)POS_EVENT_ID::VOL_NOT_EXIST;
    }

    int ret = volume->SetMaxIOPS(maxIops);
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    ret = volume->SetMaxBW(maxBw);
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    return (int)POS_EVENT_ID::SUCCESS;
}

void
VolumeInterface::_PrintLogVolumeQos(VolumeBase* volume, uint64_t originalMaxIops, uint64_t originalMaxBw)
{
    uint64_t maxIops = volume->MaxIOPS();
    uint64_t maxBw = volume->MaxBW();

    if (maxIops != originalMaxIops)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::SUCCESS,
            "Max iops is set on volume {} ({}->{})", volume->GetName(),
            originalMaxIops, maxIops);
    }

    if (maxBw != originalMaxBw)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::SUCCESS,
            "Max bandwidth is set on volume {} ({}->{})",
            volume->GetName(), originalMaxBw, maxBw);
    }
}

int
VolumeInterface::_SaveVolumes(void)
{
    return VolumeMetaIntf::SaveVolumes(volumeList, arrayName);
}

} // namespace pos
