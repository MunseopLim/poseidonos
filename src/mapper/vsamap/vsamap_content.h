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

#include "src/mapper/map/map_content.h"

#include <string>

namespace pos
{
class VSAMapContent : public MapContent
{
public:
    VSAMapContent(void);
    VSAMapContent(int mapId, std::string arrayName);

    virtual int Prepare(uint64_t size, int64_t opt = 0) override;
    virtual MpageList GetDirtyPages(uint64_t start, uint64_t numEntries) override;

    int InMemoryInit(uint64_t numEntries, uint64_t volid);
    virtual VirtualBlkAddr GetEntry(BlkAddr rba);
    virtual int SetEntry(BlkAddr rba, VirtualBlkAddr vsa);
    void ResetEntries(BlkAddr rba, uint64_t cnt);

    virtual int64_t GetNumUsedBlocks(void);
    int InvalidateAllBlocks(void);

private:
    uint64_t _GetNumValidEntries(char* mpage);

    int64_t totalBlks = 0;
    int64_t usedBlks = 0;
};

} // namespace pos
