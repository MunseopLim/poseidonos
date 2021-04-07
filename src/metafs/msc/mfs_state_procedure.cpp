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

#include "mfs_state_procedure.h"

#include "meta_file_util.h"
#include "mfs_common.h"
#include "mfs_geometry.h"
#include "mfs_mbr_mgr.h"
#include "mfs_mim_top.h"
#include "mfs_mvm_top.h"
#include "mfs_state_proc_helper.h"
#include "mss.h"

MetaFsStateProcedure::MetaFsStateProcedure(void)
{
    // Add procedure with the format below
    // * ADD_PROCEDURE(procLookupTable, <MetaFsSystemState>)
    ADD_PROCEDURE(procLookupTable, PowerOn);
    ADD_PROCEDURE(procLookupTable, Init);
    ADD_PROCEDURE(procLookupTable, Ready);
    ADD_PROCEDURE(procLookupTable, Create);
    ADD_PROCEDURE(procLookupTable, Open);
    ADD_PROCEDURE(procLookupTable, Active);
    ADD_PROCEDURE(procLookupTable, Quiesce);
    ADD_PROCEDURE(procLookupTable, Shutdown);
}

MetaFsStateProcedureFuncPointer
MetaFsStateProcedure::DispatchProcedure(MetaFsSystemState state)
{
    assert(procLookupTable[state] != nullptr);
    return procLookupTable[state];
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_PowerOn(void)
{
    return IBOF_EVENT_ID::SUCCESS;
}
IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Init(void)
{
    MetaFsStorageIoInfoList& mediaInfoList = mfsMBRMgr.GetAllStoragePartitionInfo();
    for (auto& item : mediaInfoList)
    {
        MetaVolumeType volumeType = MetaFsUtilLib::ConvertToVolumeType(item.mediaType);
        MetaLpnType maxVolumeLpn = item.totalCapacity / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

        if (MetaStorageType::SSD == item.mediaType)
        {
            maxVolumeLpn -= mfsMBRMgr.GetRegionSizeInLpn(); // considered due to MBR placement for SSD volume
        }
        mvmTopMgr.Init(volumeType, maxVolumeLpn);
    }
    mimTopMgr.Init();

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Ready(void)
{
    mvmTopMgr.Bringup();

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Create(void)
{
    MetaFsStorageIoInfoList& mediaInfoList = mfsMBRMgr.GetAllStoragePartitionInfo();

    for (auto& item : mediaInfoList)
    {
        IBOF_EVENT_ID rc;
        rc = metaStorage->CreateMetaStore(item.mediaType, item.totalCapacity, true);
        if (rc != IBOF_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
                "Failed to mount meta storage subsystem");
            return IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }

        MetaVolumeType volumeType = MetaFsUtilLib::ConvertToVolumeType(item.mediaType);

        if (false == mvmTopMgr.CreateVolume(volumeType))
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
                "Error occurred to create volume (volume id={})",
                (int)volumeType);
            return IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }
    }
    if (true != mfsMBRMgr.CreateMBR())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Error occurred to create MFS MBR");

        return IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
    }

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Open(void)
{
    bool isNPOR = false;
    // FIXME:
    // get media info
    // Filesystem MBR is only stored in SSD volume
    if (!metaStorage->IsReady())
    {
        MetaFsStorageIoInfoList& mediaInfoList = mfsMBRMgr.GetAllStoragePartitionInfo();

        for (auto& item : mediaInfoList)
        {
            IBOF_EVENT_ID rc;
            rc = metaStorage->CreateMetaStore(item.mediaType, item.totalCapacity);
            if (rc != IBOF_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
                    "Failed to mount meta storage subsystem");
                return IBOF_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
            }
        }
    }

    if (true == mfsMBRMgr.LoadMBR())
    {
        if (false == mfsMBRMgr.IsValidMBRExist())
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_FOUND,
                "Filesystem MBR has been corrupted or Filesystem cannot be found.");
            return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
        }
        isNPOR = mfsMBRMgr.GetPowerStatus();

        if (isNPOR == true)
        {
            MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
                "This open is NPOR case!!!");
        }
    }
    else
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_LOAD_FAILED,
            "Error occurred while loading filesystem MBR");
        return IBOF_EVENT_ID::MFS_META_LOAD_FAILED;
    }

    if (false == mvmTopMgr.Open(isNPOR))
    {
        return IBOF_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED;
    }

    uint64_t epochSignature = mfsMBRMgr.GetEpochSignature();
    mimTopMgr.SetMDpageEpochSignature(epochSignature);
    mimTopMgr.Bringup();

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    if (false == mvmTopMgr.Compaction(isNPOR))
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "compaction method returns false");
    }
#endif

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Quiesce(void)
{
    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Shutdown(void)
{
    bool resetCxt = false;
    try
    {
        if (false == mvmTopMgr.Close(resetCxt /* output */)) // close volumes, and clear context
        {
            if (resetCxt == true)
            { // Reset MetaFS DRAM Context
                throw IBOF_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED;
            }
            else
            {
                throw IBOF_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE;
            }
        }

        mfsMBRMgr.SetPowerStatus(true /*NPOR status*/);

        // FIXME: remove this code when MFS supports RAID1 along with FT layer
        if (true != mfsMBRMgr.SaveContent())
        {
            throw IBOF_EVENT_ID::MFS_META_SAVE_FAILED;
        }

        if (IBOF_EVENT_ID::SUCCESS != metaStorage->Close())
        {
            throw IBOF_EVENT_ID::MFS_META_STORAGE_CLOSE_FAILED;
        }

        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta filesystem is in shutdown state. Now it's safe to turn system power off..");
        throw IBOF_EVENT_ID::SUCCESS;
    }
    catch (IBOF_EVENT_ID event)
    {
        if (true == resetCxt)
        { // reset module context
            mimTopMgr.Close();
            mfsMBRMgr.InvalidMBR();
        }
        return event;
    }
}

IBOF_EVENT_ID
MetaFsStateProcedure::_ProcessSystemState_Active(void)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta filesystem is in active state");
    return IBOF_EVENT_ID::SUCCESS;
}
