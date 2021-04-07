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

#include "src/volume/volume_list.h"

#include <string>

#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
VolumeList::VolumeList()
{
    volCnt = 0;

    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        items[i] = nullptr;
    }
}

VolumeList::~VolumeList(void)
{
    Clear();
}

void
VolumeList::Clear()
{
    std::unique_lock<std::mutex> lock(listMutex);
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr)
        {
            delete items[i];
            items[i] = nullptr;
        }
    }
    volCnt = 0;
}

int
VolumeList::Add(VolumeBase* volume)
{
    if (volCnt == MAX_VOLUME_COUNT)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::VOL_CNT_EXCEEDED, "Excced maximum number of volumes");
        return (int)IBOF_EVENT_ID::VOL_CNT_EXCEEDED;
    }

    std::unique_lock<std::mutex> lock(listMutex);
    int id = _NewID();
    if (id < 0)
    {
        return (int)IBOF_EVENT_ID::VOLID_ALLOC_FAIL;
    }
    volume->ID = id;
    items[id] = volume;
    volCnt++;

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::SUCCESS, "Volume added to the list, VOL_CNT: {}, VOL_ID: {}", volCnt, id);
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeList::Add(VolumeBase* volume, int id)
{
    std::unique_lock<std::mutex> lock(listMutex);
    if (items[id] == nullptr)
    {
        volume->ID = id;
        items[id] = volume;
        volCnt++;
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::VOL_ADDED, "Volume added to the list, VOL_CNT: {}, VOL_ID: {}", volCnt, id);
        return (int)IBOF_EVENT_ID::SUCCESS;
    }

    IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::VOL_SAMEID_EXIST, "The same ID volume exists");
    return (int)IBOF_EVENT_ID::VOL_SAMEID_EXIST;
}

int
VolumeList::Remove(int volId)
{
    if (volId < 0 || volId >= MAX_VOLUME_COUNT)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::INVALID_INDEX, "Invalid index error");
        return (int)IBOF_EVENT_ID::INVALID_INDEX;
    }

    std::unique_lock<std::mutex> lock(listMutex);
    VolumeBase* target = items[volId];
    if (target == nullptr)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return (int)IBOF_EVENT_ID::VOL_NOT_EXIST;
    }

    delete target;
    items[volId] = nullptr;
    volCnt--;

    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::VOL_REMOVED, "Volume removed from the list VOL_CNT {}", volCnt);
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeList::_NewID()
{
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] == nullptr)
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::SUCCESS, "Volume New ID: {}", i);
            return i;
        }
    }
    return -1;
}

int
VolumeList::GetID(std::string volName)
{
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr && items[i]->GetName() == volName)
        {
            return i;
        }
    }
    return -1;
}

VolumeBase*
VolumeList::GetVolume(std::string volName)
{
    int volId = GetID(volName);
    if (volId >= 0)
    {
        return items[volId];
    }
    return nullptr;
}

VolumeBase*
VolumeList::GetVolume(int volId)
{
    VolumeBase* volume = nullptr;

    if (likely((0 <= volId) && (volId < MAX_VOLUME_COUNT)))
    {
        volume = items[volId];
    }

    return volume;
}

VolumeBase*
VolumeList::Prev(int& index)
{
    for (int i = index - 1; i >= 0; i--)
    {
        if (items[i] != nullptr)
        {
            index = i;
            return items[i];
        }
    }

    return nullptr;
}

VolumeBase*
VolumeList::Next(int& index)
{
    for (int i = index + 1; i < MAX_VOLUME_COUNT; i++)
    {
        if (items[i] != nullptr)
        {
            index = i;
            return items[i];
        }
    }

    return nullptr;
}

} // namespace ibofos
