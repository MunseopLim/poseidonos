#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_map_flush.h"

namespace pos
{
class MockIMapFlush : public IMapFlush
{
public:
    using IMapFlush::IMapFlush;
    MOCK_METHOD(int, FlushDirtyMpages, (int mapId, EventSmartPtr callback, MpageList dirtyPages), (override));
    MOCK_METHOD(int, FlushAllMaps, (), (override));
    MOCK_METHOD(void, WaitForFlushAllMapsDone, (), (override));
    MOCK_METHOD(int, StoreAllMaps, (), (override));
};

} // namespace pos
