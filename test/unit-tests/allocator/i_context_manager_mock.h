#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_manager.h"

namespace pos
{
class MockIContextManager : public IContextManager
{
public:
    using IContextManager::IContextManager;
    MOCK_METHOD(int, FlushContextsSync, (), (override));
    MOCK_METHOD(int, FlushContextsAsync, (EventSmartPtr callback), (override));
    MOCK_METHOD(void, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (bool forUser), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, GetNumFreeSegment, (), (override));
    MOCK_METHOD(CurrentGcMode, GetCurrentGcMode, (), (override));
    MOCK_METHOD(int, GetGcThreshold, (CurrentGcMode mode), (override));
};

} // namespace pos
