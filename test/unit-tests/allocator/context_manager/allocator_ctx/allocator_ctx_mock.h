#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

namespace pos
{
class MockAllocatorCtx : public AllocatorCtx
{
public:
    using AllocatorCtx::AllocatorCtx;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (int section, char* buf), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx * ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(StripeId, UpdatePrevLsid, (), (override));
    MOCK_METHOD(void, SetCurrentSsdLsid, (StripeId stripe), (override));
    MOCK_METHOD(void, RollbackCurrentSsdLsid, (), (override));
    MOCK_METHOD(StripeId, GetCurrentSsdLsid, (), (override));
    MOCK_METHOD(StripeId, GetPrevSsdLsid, (), (override));
    MOCK_METHOD(void, SetPrevSsdLsid, (StripeId stripeId), (override));
    MOCK_METHOD(void, SetNextSsdLsid, (SegmentId segId), (override));
    MOCK_METHOD(void, AllocateSegment, (SegmentId segId), (override));
    MOCK_METHOD(void, ReleaseSegment, (SegmentId segId), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (SegmentId startSegId), (override));
    MOCK_METHOD(SegmentId, GetUsedSegment, (SegmentId startSegId), (override));
    MOCK_METHOD(uint64_t, GetNumOfFreeUserDataSegment, (), (override));
    MOCK_METHOD(SegmentState, GetSegmentState, (SegmentId segmentId, bool needlock), (override));
    MOCK_METHOD(void, SetSegmentState, (SegmentId segmentId, SegmentState state, bool needlock), (override));
    MOCK_METHOD(void, SetAllocatedSegmentCount, (int count), (override));
    MOCK_METHOD(int, GetAllocatedSegmentCount, (), (override));
    MOCK_METHOD(int, GetTotalSegmentsCount, (), (override));
    MOCK_METHOD(std::mutex&, GetSegStateLock, (SegmentId segId), (override));
    MOCK_METHOD(std::mutex&, GetAllocatorCtxLock, (), (override));
};

} // namespace pos
