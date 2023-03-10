#include <gmock/gmock.h>
#include <map>

#include "src/metadata/meta_event_factory.h"

namespace pos
{
class MockMetaEventFactory : public MetaEventFactory
{
public:
    using MetaEventFactory::MetaEventFactory;
    MOCK_METHOD(CallbackSmartPtr, CreateBlockMapUpdateEvent, (VolumeIoSmartPtr volumeIo), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateStripeMapUpdateEvent, (StripeSmartPtr stripe), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateGcMapUpdateEvent, (StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt)), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateFreedSegmentCtxUpdateEvent, (SegmentCtx* segmentCtx, SegmentId targetSegmentId), (override));
};

} // namespace pos
