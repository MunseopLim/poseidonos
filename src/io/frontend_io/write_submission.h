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

#include <array>
#include <list>
#include <queue>
#include <utility>
#include <vector>

#include "src/allocator/allocator.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/block_alignment.h"
#include "src/io/general_io/io_controller.h"
#include "src/io/general_io/volume_io.h"
#include "src/scheduler/event.h"

namespace ibofos
{
class EventArgument;
class Allocator;
class VolumeIo;
class RBAStateManager;

class WriteSubmission : public IOController, public Event
{
public:
    WriteSubmission(VolumeIoSmartPtr volumeIo);
    virtual ~WriteSubmission(void);

    virtual bool Execute(void);
    uint32_t GetOriginCore(void);

private:
    RBAStateManager& rbaStateManager;

    using BlockAllocationArray = std::vector<VirtualBlks>;
    using EventIdWithLevel = std::pair<IBOF_EVENT_ID, EventLevel>;

    Allocator* allocator;
    VolumeIoSmartPtr volumeIo;
    uint32_t blockCount;
    uint32_t volumeId;
    uint32_t volumeIoCount;
    uint32_t remainSize;
    BlockAlignment blockAlignment;
    uint32_t allocatedBlockCount;
    uint32_t processedBlockCount;
    std::queue<VolumeIoSmartPtr> splitVolumeIoQueue;

    bool _ProcessOwnedWrite(void);
    void _AllocateFreeWriteBuffer(void);
    void _ReadOldBlock(BlkAddr rba, VirtualBlkAddr& vsa, bool isTail);
    void _AddVirtualBlks(VirtualBlks& virtualBlks);
    VolumeIoSmartPtr _CreateVolumeIo(VirtualBlks& vsaRange);
    void _PrepareSingleBlock(VirtualBlks& vsaRange);
    void _ReadOldHeadBlock(void);
    void _ReadOldTailBlock(void);
    void _PrepareBlockAlignment(void);
    void _WriteDataAccordingToVsaRange(VirtualBlks& vsasRange);
    void _WriteSingleBlock(void);
    void _WriteMultipleBlocks(void);
    void _SubmitVolumeIo(void);
};
} // namespace ibofos
