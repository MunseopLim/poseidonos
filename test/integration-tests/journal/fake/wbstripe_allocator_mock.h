#pragma once

#include "gmock/gmock.h"
#include "src/allocator/i_wbstripe_allocator.h"

namespace pos
{
class WBStripeAllocatorMock : public IWBStripeAllocator
{
public:
    WBStripeAllocatorMock(void) {}
    virtual ~WBStripeAllocatorMock(void) {}

    MOCK_METHOD(int, RestoreActiveStripeTail,
        (uint32_t volumeId, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(int, ReconstructActiveStripe,
        (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe,
        (StripeId wbLsid, VirtualBlkAddr tail), (override));

    virtual Stripe* GetStripe(StripeAddr& lsa) override { return nullptr; }
    virtual StripeId AllocateUserDataStripeId(StripeId vsid) override { return 0; }
    virtual void FreeWBStripeId(StripeId lsid) override {}

    virtual bool ReferLsidCnt(StripeAddr& lsa) override { return true; }
    virtual void DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount) override {}

    virtual void GetAllActiveStripes(uint32_t volumeId) {}
    virtual bool WaitPendingWritesOnStripes(uint32_t volumeId) { return true; }
    virtual bool WaitStripesFlushCompletion(uint32_t volumeId) { return true; }

    virtual void FlushAllActiveStripes(void) override {}
    virtual int FlushPendingActiveStripes(void) { return 0; }

    virtual int PrepareRebuild(void) override { return 0; }
    virtual int StopRebuilding(void) override { return 0; }
};

} // namespace pos
