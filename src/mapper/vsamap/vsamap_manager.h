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

#include "src/journal_service/journal_service.h"
#include "src/mapper/include/mapper_const.h"
#include "src/mapper/i_map_manager.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/vsamap/i_vsamap_internal.h"
#include "src/mapper/vsamap/vsamap_api.h"
#include "src/mapper/vsamap/vsamap_content.h"
#include "src/sys_event/volume_event.h"
#include "src/volume/volume_service.h"

#include <condition_variable>
#include <map>
#include <string>
#include <vector>

namespace pos
{

const uint64_t UNKNOWN_SIZE_BECAUSEOF_INTERNAL_LOAD = 0;

enum class VolState
{
    EXIST_UNLOADED,     // Unloaded
    JUST_CREATED,       // Loaded
    BACKGROUND_MOUNTED, // Loaded
    FOREGROUND_MOUNTED, // Loaded
};

using VolMountStateIter = std::map<int, VolState>::iterator;

class VSAMapManager : public IMapManagerInternal, public IVSAMapInternal, public VolumeEvent
{
public:
    VSAMapManager(MapperAddressInfo* info, std::string arrayName);
    virtual ~VSAMapManager(void);

    void Init(void);
    int StoreMaps(void);
    int FlushMaps(void);
    void Close(void);
    VSAMapContent*& GetVSAMapContent(int volID);
    bool AllMapsAsyncFlushed(void);
    void SetVolumeManagerObject(IVolumeManager* volumeManagerToUse);

    IVSAMap* GetIVSAMap(void);
    VSAMapAPI* GetVSAMapAPI(void);

    void MapAsyncFlushDone(int mapId) override;

    int EnableInternalAccess(int volID, int caller) override;
    std::atomic<int>& GetLoadDoneFlag(int volumeId) override;

    bool VolumeCreated(std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeUnmounted(std::string volName, int volID, std::string arrayName) override;
    bool VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName) override;
    void VolumeDetached(vector<int> volList, std::string arrayName) override;

private:
    bool _PrepareVsaMapAndHeader(int volID, uint64_t& volSizeByte, bool isUnknownVolSize);
    bool _PrepareInMemoryData(int volID, uint64_t volSizeByte);
    bool _VSAMapFileCreate(int volID);
    int _VSAMapFileAsyncLoad(int volID);
    int _VSAMapFileAsyncLoadNoWait(int volID);
    bool _VSAMapFileStore(int volID);
    void _WakeUpAfterLoadDone(int volID);
    void _AfterLoadDone(int volID);
    bool _IsVolumeExist(int volID, VolMountStateIter& iter);
    bool _LoadVolumeMeta(std::string volName, int volID, uint64_t volSizeByte, bool isUnknownVolSize);
    void _WaitForMapAsyncFlushed(int volId);
    void _HandleVolumeCreationFail(int volID);

    std::map<int, VolState> volumeMountState;
    std::recursive_mutex volMountStateLock[MAX_VOLUME_COUNT];

    std::mutex volAsyncLoadLock; // for cvLoadDone.wait()
    std::condition_variable cvLoadDone;
    std::atomic<int> loadDoneFlag[MAX_VOLUME_COUNT];
    AsyncLoadCallBack loadDoneWakeUp;
    AsyncLoadCallBack loadDone;

    std::map<int, MapFlushState> mapFlushStatus;

    IVolumeManager* volumeManager;
    VSAMapAPI* vsaMapAPI;
    JournalService* journalService;
};

} // namespace pos
