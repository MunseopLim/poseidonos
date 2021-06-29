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
#include <vector>
#include <string>

#include "src/network/nvmf_target.hpp"
#include "src/network/nvmf_volume.hpp"

using namespace std;
namespace pos
{
struct volumeListInfo
{
    string arrayName;
    string subnqn;
    vector<int> vols;
};

class NvmfVolumePos final : public NvmfVolume
{
public:
    NvmfVolumePos(void);
    ~NvmfVolumePos(void);

    void VolumeCreated(struct pos_volume_info* info) override;
    void VolumeDeleted(struct pos_volume_info* info) override;
    void VolumeMounted(struct pos_volume_info* info) override;
    void VolumeUnmounted(struct pos_volume_info* info) override;
    void VolumeUpdated(struct pos_volume_info* info) override;
    void VolumeDetached(vector<int>& volList, string arrayName) override;

    static uint32_t VolumeDetachCompleted(void);
    static bool WaitRequestedVolumesDetached(uint32_t volCnt);

private:
    static NvmfTarget target;
    static atomic<bool> detachFailed;
    static atomic<uint32_t> volumeDetachedCnt;

    static void _VolumeCreateHandler(void* arg1, void* arg2);
    static void _VolumeMountHandler(void* arg1, void* arg2);
    static void _VolumeUnmountHandler(void* arg1, void* arg2);
    static void _VolumeDeleteHandler(void* arg1, void* arg2);
    static void _VolumeUpdateHandler(void* arg1, void* arg2);
    static void _VolumeDetachHandler(void* arg1, void* arg2);
    static void _NamespaceDetachedHandler(void* cbArg, int status);
    static void _NamespaceDetachedAllHandler(void* cbArg, int status);
    static void _CompleteVolumeUnmount(struct pos_volume_info* vInfo);
};

} // namespace pos
