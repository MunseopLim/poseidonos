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

#include "stripemap_content.h"

#include "src/include/branch_prediction.h"

#include <string>

namespace pos
{
StripeMapContent::StripeMapContent(int mapId, std::string arrayName)
: MapContent(mapId)
{
    filename = "StripeMap.bin";
    this->arrayName = arrayName;
}

int
StripeMapContent::Prepare(uint64_t numEntries, int64_t opt)
{
    SetPageSize(arrayName);

    header.entriesPerMpage = header.mpageSize / sizeof(StripeAddr);

    uint64_t numPages = DivideUp(numEntries, header.entriesPerMpage);
    InitHeaderInfo(numPages);
    int ret = Init(numPages);

    return ret;
}

StripeAddr
StripeMapContent::GetEntry(StripeId vsid)
{
    uint32_t pageNr = vsid / header.entriesPerMpage;

    char* mpage = map->GetMpage(pageNr);

    if (unlikely(mpage == nullptr))
    {
        StripeAddr unmapped = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
        POS_TRACE_ERROR(EID(MPAGE_NULLPTR), "mpage is nullptr when vsid:{}", vsid);
        return unmapped;
    }
    else
    {
        uint32_t entNr = vsid % header.entriesPerMpage;
        return ((StripeAddr*)mpage)[entNr];
    }
}

int
StripeMapContent::SetEntry(StripeId vsid, StripeAddr entry)
{
    uint32_t pageNr = vsid / header.entriesPerMpage;

    map->GetMpageLock(pageNr);
    char* mpage = map->GetMpage(pageNr);

    if (mpage == nullptr)
    {
        mpage = map->AllocateMpage(pageNr);
        if (unlikely(mpage == nullptr))
        {
            map->ReleaseMpageLock(pageNr);
            return -EID(STRIPEMAP_SET_FAILURE);
        }
        header.SetMapAllocated(pageNr);
    }

    StripeAddr* mpageMap = (StripeAddr*)mpage;
    uint32_t entNr = vsid % header.entriesPerMpage;
    mpageMap[entNr] = entry;

    header.touchedPages->SetBit(pageNr);

    map->ReleaseMpageLock(pageNr);
    return 0;
}

MpageList
StripeMapContent::GetDirtyPages(uint64_t start, uint64_t numEntries)
{
    assert(numEntries == 1);

    MpageList dirtyList;
    dirtyList.insert(start / header.entriesPerMpage);
    return dirtyList;
}

} // namespace pos
