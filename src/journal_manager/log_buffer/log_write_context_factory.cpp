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

#include "log_write_context_factory.h"

#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/allocator/wb_stripe_manager/stripe.h"

namespace pos
{
LogWriteContextFactory::LogWriteContextFactory(void)
: notifier(nullptr),
  sequenceController(nullptr)
{
}

LogWriteContextFactory::~LogWriteContextFactory(void)
{
}

void
LogWriteContextFactory::Init(LogBufferWriteDoneNotifier* target, CallbackSequenceController* sequencer)
{
    notifier = target;
    sequenceController = sequencer;
}

LogWriteContext*
LogWriteContextFactory::CreateBlockMapLogWriteContext(VolumeIoSmartPtr volumeIo,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    uint32_t volId = volumeIo->GetVolumeId();
    BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint64_t numBlks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);

    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    int wbIndex = volumeIo->GetVolumeId();
    StripeAddr writeBufferStripeAddress = volumeIo->GetLsidEntry(); // TODO(huijeong.kim): to only have wbLsid

    BlockWriteDoneLogHandler* log = new BlockWriteDoneLogHandler(volId, startRba,
        numBlks, startVsa, wbIndex, writeBufferStripeAddress);

    MapPageList dirtyMap;
    dirtyMap.emplace(volId, dirty);

    MapUpdateLogWriteContext* logWriteContext
        = new MapUpdateLogWriteContext(log, dirtyMap, callbackEvent, notifier, sequenceController);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateStripeMapLogWriteContext(Stripe* stripe,
    StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent)
{
    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe->GetUserLsid()};

    StripeMapUpdatedLogHandler* log = new StripeMapUpdatedLogHandler(stripe->GetVsid(), oldAddr, newAddr);

    MapPageList dirtyMap;
    dirtyMap.emplace(STRIPE_MAP_ID, dirty);

    MapUpdateLogWriteContext* logWriteContext
        = new MapUpdateLogWriteContext(log, dirtyMap, callbackEvent, notifier, sequenceController);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateGcStripeFlushedLogWriteContext(
    GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent)
{
    GcStripeFlushedLogHandler* log = new GcStripeFlushedLogHandler(mapUpdates);

    MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log,
        dirty, callbackEvent, notifier, sequenceController);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateVolumeDeletedLogWriteContext(int volId,
    uint64_t contextVersion, EventSmartPtr callback)
{
    LogHandlerInterface* log = new VolumeDeletedLogEntry(volId, contextVersion);
    LogWriteContext* logWriteContext = new LogWriteContext(log, callback, notifier);

    return logWriteContext;
}

} // namespace pos
