#include "src/allocator/context_manager/context_manager.h"

#include <gtest/gtest.h>

#include <mutex>

#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/file_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_meta_flush_completed_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(ContextManager, ContextManager_)
{
}

TEST(ContextManager, Init_TestCaseFormatFormat)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");

    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // given 1. : Format+Format
    std::mutex segCtxLock, allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));

    EXPECT_CALL(*allocCtx, Init);
    EXPECT_CALL(*wbStripeCtx, Init);
    EXPECT_CALL(*segCtx, Init);
    EXPECT_CALL(*fileMan, Init);
    EXPECT_CALL(*reCtx, Init);

    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(0)).WillOnce(Return(0));
    // when 1.
    ctxManager.Init();
}

TEST(ContextManager, Init_TestCaseLoadLoad)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");

    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // given 2. : Load+Load
    EXPECT_CALL(*allocCtx, Init);
    EXPECT_CALL(*wbStripeCtx, Init);
    EXPECT_CALL(*segCtx, Init);
    EXPECT_CALL(*fileMan, Init);
    EXPECT_CALL(*reCtx, Init);

    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(1)).WillOnce(Return(1));
    // when 2.
    ctxManager.Init();
}

TEST(ContextManager, Init_TestCaseLoadFormat)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");

    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // given 3. : Load+Format
    std::mutex allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));

    EXPECT_CALL(*allocCtx, Init);
    EXPECT_CALL(*wbStripeCtx, Init);
    EXPECT_CALL(*segCtx, Init);
    EXPECT_CALL(*fileMan, Init);
    EXPECT_CALL(*reCtx, Init);

    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(1)).WillOnce(Return(0));
    // when 3.
    ctxManager.Init();
    /*
    // given 4. : Fail
    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(-1));
    // when 4.
    EXPECT_DEATH(ctxManager.Init(), "");

    // given 5. : Format+Fail
    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(0)).WillOnce(Return(-1));
    // when 5.
    EXPECT_DEATH(ctxManager.Init(), "");

    // given 6. : Load+Fail
    EXPECT_CALL(*fileMan, LoadSync).WillOnce(Return(1)).WillOnce(Return(-1));
    // when 6.
    EXPECT_DEATH(ctxManager.Init(), "");
    */
}

TEST(ContextManager, Close_TestIfAllCalled)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*allocCtx, Dispose);
    EXPECT_CALL(*wbStripeCtx, Dispose);
    EXPECT_CALL(*segCtx, Dispose);
    EXPECT_CALL(*fileMan, Dispose);
    EXPECT_CALL(*reCtx, Dispose);

    // when
    ctxManager.Dispose();
}

TEST(ContextManager, FlushContextsSync_IfSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock, allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));
    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(0)).WillOnce(Return(0));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    EXPECT_EQ(0, ret);
}

TEST(ContextManager, FlushContextsSync_IfFailFirstFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsSync_IfFailSecondFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock, allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));
    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(0)).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsAsync_IfAlreadyFlushing)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, true, nullptr, nullptr, "");

    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    EXPECT_EQ((int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS, ret);
}

TEST(ContextManager, FlushContextsAsync_IfSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock, allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));
    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(0)).WillOnce(Return(0));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    EXPECT_EQ(0, ret);
}

TEST(ContextManager, FlushContextsAsync_IfFailFirstFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsAsync_IfFailSecondFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    std::mutex segCtxLock, allocCtxLock, wbLsidBitmapLock;
    EXPECT_CALL(*segCtx, GetSegmentCtxLock).WillOnce(ReturnRef(segCtxLock));
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*wbStripeCtx, GetAllocWbLsidBitmapLock).WillOnce(ReturnRef(wbLsidBitmapLock));
    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(0)).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    EXPECT_LE(-1, ret);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountSmallerThanMax)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(10));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsNotZero)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    std::mutex segStateLock;
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(100));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateFree)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    std::mutex segStateLock;
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateSSD)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    std::mutex segStateLock;
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateNVRAM)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");
    std::mutex segStateLock;
    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::NVRAM));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, AllocateFreeSegment_TestFreeSegmentAllocationByState)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // given 1. for rebuild segment allocation
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(5));
    // when 1.
    ctxManager.AllocateFreeSegment(false);

    // given 2. for user segment allocation
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    // when 2.
    ctxManager.AllocateFreeSegment(true);

    // given 3. for user segment allocation, first failed, second success
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(5)).WillOnce(Return(11));
    EXPECT_CALL(*reCtx, IsRebuildTargetSegment).WillOnce(Return(true)).WillOnce(Return(false));
    // when 3.
    ctxManager.AllocateFreeSegment(true);
}

TEST(ContextManager, AllocateGCVictimSegment_TestGCVictimAllocationByStateAndValidBlockCount)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(5);
    addrInfo.SetblksPerSegment(20);
    addrInfo.SetnumUserAreaSegments(5);

    std::mutex segStateLock;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");

    // given 1.
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(15)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(13)).WillOnce(Return(10));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD));

    // when 1.
    int ret = ctxManager.AllocateGCVictimSegment();
    // then 2.
    EXPECT_EQ(4, ret);

    // given 2.
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock)).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(20)).WillOnce(Return(0)).WillOnce(Return(20)).WillOnce(Return(20)).WillOnce(Return(20));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD));

    // when 2.
    ret = ctxManager.AllocateGCVictimSegment();
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);
}

TEST(ContextManager, GetNumOfFreeSegment_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(50));
    // when
    int ret = ctxManager.GetNumOfFreeSegment(true);
    // then
    EXPECT_EQ(50, ret);
}

TEST(ContextManager, GetCurrentGcMode_TestGetGcModeByNumberOfFreeSegment)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    ctxManager.GetGcCtx()->SetGcThreshold(10);
    ctxManager.GetGcCtx()->SetUrgentThreshold(5);

    // given 1.
    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(11));
    // when 1.
    CurrentGcMode ret = ctxManager.GetCurrentGcMode();
    // then 1.
    EXPECT_EQ(MODE_NO_GC, ret);

    // given 2.
    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(10));
    // when 2.
    ret = ctxManager.GetCurrentGcMode();
    // then 2.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    // given 3.
    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(9));
    // when 3.
    ret = ctxManager.GetCurrentGcMode();
    // then 3.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    // given 4.
    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(5));
    // when 4.
    ret = ctxManager.GetCurrentGcMode();
    // then 4.
    EXPECT_EQ(MODE_URGENT_GC, ret);

    // given 5.
    EXPECT_CALL(*allocCtx, GetNumOfFreeSegment).WillOnce(Return(4));
    // when 5.
    ret = ctxManager.GetCurrentGcMode();
    // then 5.
    EXPECT_EQ(MODE_URGENT_GC, ret);
}

TEST(ContextManager, GetGcThreshold_TestSimpleGetterByMode)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    ctxManager.GetGcCtx()->SetGcThreshold(10);
    ctxManager.GetGcCtx()->SetUrgentThreshold(5);

    // when 1.
    int ret = ctxManager.GetGcThreshold(MODE_NORMAL_GC);
    // then 1.
    EXPECT_EQ(10, ret);

    // when 2.
    ret = ctxManager.GetGcThreshold(MODE_URGENT_GC);
    // then 2.
    EXPECT_EQ(5, ret);
}

TEST(ContextManager, FreeUserDataSegment_TestWhenSegmentStateChangedHowFreeUserDataSegmentWorks)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(50);
    addrInfo.SetblksPerSegment(100);
    addrInfo.SetnumUserAreaSegments(20);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, &addrInfo, nullptr, "");
    std::mutex segStateLock;

    // given 1. release free segment
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).WillOnce(Return(50));
    // when 1.
    ctxManager.FreeUserDataSegment(5);

    // given 2. release victim segment as free
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::VICTIM));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).WillOnce(Return(50));
    // when 2.
    ctxManager.FreeUserDataSegment(5);

    // given 3. release nvram segment as free
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::NVRAM));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).Times(0);
    // when 3.
    ctxManager.FreeUserDataSegment(5);

    // given 4. just exit with free state
    EXPECT_CALL(*allocCtx, GetSegStateLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).Times(0);
    // when 4.
    ctxManager.FreeUserDataSegment(5);
}

TEST(ContextManager, GetStoredContextVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // when 1.
    ctxManager.GetStoredContextVersion(SEGMENT_CTX);

    // when 2.
    ctxManager.GetStoredContextVersion(ALLOCATOR_CTX);
}

TEST(ContextManager, AllocateRebuildTargetSegment_TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*reCtx, GetRebuildTargetSegment).WillOnce(Return(5));

    // when
    int ret = ctxManager.AllocateRebuildTargetSegment();

    // then
    EXPECT_EQ(5, ret);
}

TEST(ContextManager, ReleaseRebuildSegment__TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*reCtx, ReleaseRebuildSegment).WillOnce(Return(0));

    // when
    int ret = ctxManager.ReleaseRebuildSegment(5);

    // then
    EXPECT_EQ(0, ret);
}

TEST(ContextManager, NeedRebuildAgain_TestSimpleByPassFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*reCtx, ReleaseRebuildSegment).WillOnce(Return(true));

    // when
    bool ret = ctxManager.ReleaseRebuildSegment(5);

    // then
    EXPECT_EQ(true, ret);
}

TEST(ContextManager, GetContextSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, GetSectionAddr).WillOnce(Return((char*)100));

    // when
    char* ret = ctxManager.GetContextSectionAddr(0, 0);

    // then
    EXPECT_EQ((char*)100, ret);
}

TEST(ContextManager, GetContextSectionSize_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, GetSectionSize).WillOnce(Return(1000));

    // when
    int ret = ctxManager.GetContextSectionSize(0, 0);

    // then
    EXPECT_EQ(1000, ret);
}

TEST(ContextManager, GetRebuildCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    MockRebuildCtx* ret = reinterpret_cast<MockRebuildCtx*>(ctxManager.GetRebuildCtx());
    // then
    EXPECT_EQ(reCtx, ret);
}

TEST(ContextManager, GetContextReplayer_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>(nullptr, nullptr, nullptr, nullptr);
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, ctxReplayer, false, nullptr, nullptr, "");
    // when
    ContextReplayer* ret = ctxManager.GetContextReplayer();
    // then
    EXPECT_EQ(ctxReplayer, ret);
}

TEST(ContextManager, TestCallbackFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    EventSmartPtr flushCallback(new MockCheckpointMetaFlushCompleted((CheckpointHandler*)this, 0));
    ctxManager.SetCallbackFunc(flushCallback);

    // given 1.
    char* buf = new char[100];
    AsyncMetaFileIoCtx* ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;

    EXPECT_CALL(*segCtx, IsSegmentCtxIo).WillOnce(Return(true));
    EXPECT_CALL(*segCtx, FinalizeIo);
    // when 1.
    ctxManager.TestCallbackFunc(ctx, 2);

    // given 2.
    buf = new char[100];
    ctx = new AsyncMetaFileIoCtx();
    ctx->buffer = buf;

    EXPECT_CALL(*segCtx, IsSegmentCtxIo).WillOnce(Return(false));
    EXPECT_CALL(*allocCtx, FinalizeIo);
    // when 1.
    ctxManager.TestCallbackFunc(ctx, 1);
}

TEST(ContextManager, SetNextSsdLsid_TestCheckReturnedSegmentId)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");

    // given 1.
    std::mutex allocCtxLock;
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(5));
    EXPECT_CALL(*reCtx, IsRebuildTargetSegment).WillOnce(Return(false));
    EXPECT_CALL(*allocCtx, SetNextSsdLsid(5));
    // when 1.
    int ret = ctxManager.SetNextSsdLsid();
    // then 1.
    EXPECT_EQ(0, ret);

    // given 2.
    EXPECT_CALL(*allocCtx, GetAllocatorCtxLock).WillOnce(ReturnRef(allocCtxLock));
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    // when 2.
    ret = ctxManager.SetNextSsdLsid();
    // then 2.
    EXPECT_EQ((int)-EID(ALLOCATOR_NO_FREE_SEGMENT), ret);
}

TEST(ContextManager, GetSegmentCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    SegmentCtx* ret = ctxManager.GetSegmentCtx();
    // then
    EXPECT_EQ(segCtx, ret);
}

TEST(ContextManager, GetAllocatorCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    AllocatorCtx* ret = ctxManager.GetAllocatorCtx();
    // then
    EXPECT_EQ(allocCtx, ret);
}

TEST(ContextManager, GetWbStripeCtx_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    WbStripeCtx* ret = ctxManager.GetWbStripeCtx();
    // then
    EXPECT_EQ(wbStripeCtx, ret);
}

TEST(ContextManager, GetCtxLock_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    ctxManager.GetCtxLock();
}

TEST(ContextManager, NeedRebuildAgain_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr, nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, false, nullptr, nullptr, "");
    // when
    ctxManager.NeedRebuildAgain();
}

} // namespace pos
