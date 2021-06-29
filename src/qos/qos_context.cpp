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

#include "src/qos/qos_context.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosContext::QosContext(void)
{
    Reset();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosContext::~QosContext(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::SetTotalConnection(uint32_t volId, uint32_t value)
{
    totalConnection[volId] = value;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosContext::GetTotalConnection(uint32_t volId)
{
    return totalConnection[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::UpdateReactorCoreList(uint32_t reactorCore)
{
    reactorCoreList.push_back(reactorCore);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::vector<uint32_t>&
QosContext::GetReactorCoreList(void)
{
    return reactorCoreList;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::Reset(void)
{
    userPolicy.Reset();
    parameters.Reset();
    correction.Reset();
    resourceState.Reset();
    activeVolumeMap.clear();
    activeReactorVolumeMap.clear();
    timestamp = 0;
    elapsedTime = 0.0;
    volMinPolicy = false;
    userPolicyChange = false;
    correctionChange = false;
    resourceStateChange = false;
    applyCorrection = false;
    qosCorrectionCycle = 0;
    reactorCoreList.clear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosUserPolicy&
QosContext::GetQosUserPolicy(void)
{
    return userPolicy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosParameters&
QosContext::GetQosParameters(void)
{
    return parameters;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosCorrection&
QosContext::GetQosCorrection(void)
{
    return correction;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosResource&
QosContext::GetQosResource(void)
{
    return resourceState;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::ResetActiveVolume(void)
{
    activeVolumeMap.clear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::InsertActiveVolume(uint32_t volId)
{
    activeVolumeMap[volId] = 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::map<uint32_t, uint32_t>&
QosContext::GetActiveVolumes(void)
{
    return activeVolumeMap;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::ResetActiveReactorVolume(void)
{
    activeReactorVolumeMap.clear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::InsertActiveReactorVolume(uint32_t reactor, uint32_t volId)
{
    std::pair<uint32_t, uint32_t> reactorVolumePair = std::make_pair(reactor, volId);
    activeReactorVolumeMap[reactorVolumePair] = 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::map<uint32_t, map<uint32_t, uint32_t>>&
QosContext::GetActiveVolumeReactors(void)
{
    return activeVolReactorMap;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::InsertActiveVolumeReactor(std::map<uint32_t, map<uint32_t, uint32_t>> map)
{
    activeVolReactorMap = map;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosContext::GetActiveReactorVolumeCount(void)
{
    return activeReactorVolumeMap.size();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosContext::IsVolumeMinPolicyInEffect(void)
{
    AllVolumeUserPolicy allVolPolicy = userPolicy.GetAllVolumeUserPolicy();
    if (true == allVolPolicy.IsMinPolicyInEffect())
    {
        volMinPolicy = true;
    }
    else
    {
        volMinPolicy = false;
    }
    return volMinPolicy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::SetApplyCorrection(bool apply)
{
    applyCorrection = apply;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosContext::GetApplyCorrection(void)
{
    return applyCorrection;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosContext::IncrementCorrectionCycle(void)
{
    qosCorrectionCycle++;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosContext::IsCorrectionCycleOver(void)
{
    if (qosCorrectionCycle >= M_QOS_CORRECTION_CYCLE)
    {
        qosCorrectionCycle = 0;
        return true;
    }
    else
    {
        return false;
    }
}
} // namespace pos
