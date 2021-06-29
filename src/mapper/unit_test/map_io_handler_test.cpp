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

#include "src/mapper/map/map_flush_handler.h"
#include "src/mapper/unit_test/map_io_handler_test.h"

namespace pos
{

void
MapIoHandlerTest::SetUp(void)
{
    MapperTestFixture::SetUp();
    _CreateRandVolume(0);
    _MountVolume(0);
    _SetupMockEventScheduler();

    map = new Map(NUM_PAGES_IN_MAP, MPAGE_SIZE);
    mapHeader = new MapHeader();
    mapIoHandler = new MapIoHandler(map, mapHeader);

    mapHeader->bitmap = new BitMap(NUM_PAGES_IN_MAP);
    mapHeader->bitmap->ResetBitmap();

    mapHeader->touchedPages = new BitMap(NUM_PAGES_IN_MAP);
    mapHeader->touchedPages->ResetBitmap();
}

void
MapIoHandlerTest::TearDown(void)
{
    delete map;
    delete mapHeader;

    delete mapIoHandler->touchedPages;
    delete mapIoHandler;

    _UnmountVolume(0);
    _DeleteVolume(0);
    MapperTestFixture::TearDown();
    _ResetMockEventScheduler();
}

void
MapIoHandlerTest::_GetTouchedPages(BitMap* validPages)
{
    mapIoHandler->touchedPages = new BitMap(NUM_PAGES_IN_MAP);
    mapIoHandler->_GetTouchedPages(validPages);
}

void
MapIoHandlerTest::_CheckFlushingPagesBitmap(void)
{
    BitMap* flushingPages = mapIoHandler->touchedPages;

    EXPECT_TRUE(flushingPages->GetNumBitsSet() == mapHeader->touchedPages->GetNumBitsSet());
    EXPECT_TRUE(flushingPages->GetNumEntry() == mapHeader->touchedPages->GetNumEntry());
    EXPECT_TRUE(flushingPages->GetNumEntry() != 0);
    EXPECT_TRUE(memcmp(flushingPages->GetMapAddr(),
                    mapHeader->touchedPages->GetMapAddr(),
                    mapHeader->bitmap->GetNumEntry() * BITMAP_ENTRY_SIZE) == 0);
}

TEST_F(MapIoHandlerTest, GetDirtyPagesWithEmptyMap)
{
    _GetTouchedPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

TEST_F(MapIoHandlerTest, GetDirtyPagesWithFlushedMap)
{
    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum++)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    _GetTouchedPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

TEST_F(MapIoHandlerTest, GetDirtyPages)
{
    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum += 2)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    for (int pageNum = 0; pageNum < NUM_PAGES_IN_MAP; pageNum += 4)
    {
        mapHeader->bitmap->SetBit(pageNum);
    }

    _GetTouchedPages(mapHeader->bitmap);
    _CheckFlushingPagesBitmap();
}

// [VSAMap Validity /w POR]
// Store VSA Map -> Unmount (NPO Case) -> Load VSA Map
TEST_F(MapIoHandlerTest, StoreVSAMap)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.StoreVSAMap ********************");
    uint64_t test_count = SZ_1GB / pos::BLOCK_SIZE;
    uint64_t test;

    for (test = 0; test < test_count; test++)
    {
        BlkAddr rba = test;
        _SetVSAs(TEST_VOL_ID, rba, (StripeId)(test / 128), (BlkOffset)(test % 128));
    }

    mapperSUT->StoreAllMaps(); // store stripMap & vsamap[vol]

    _SimulateSPOR();

    for (test = 0; test < test_count; test++)
    {
        BlkAddr rba = test;
        _GetAndCompareVSA(TEST_VOL_ID, rba, (StripeId)(test / 128), (BlkOffset)(test % 128));
    }
}

// [StripMap Validity /w POR]
// Store Stripe Map -> Unmount (NPO Case) -> Load Stripe Map
TEST_F(MapIoHandlerTest, StoreStripeMap)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.StoreStripeMap ********************");
    int test;
    for (test = 0; test < 100; test++)
    {
        StripeLoc loc = (test % 2 ? IN_USER_AREA : IN_WRITE_BUFFER_AREA);
        _SetLSA(test, test * 2, loc);
    }

    mapperSUT->StoreAllMaps();

    _SimulateSPOR();

    for (test = 0; test < 100; test++)
    {
        StripeLoc loc = (test % 2 ? IN_USER_AREA : IN_WRITE_BUFFER_AREA);
        _GetAndCompareLSA((StripeId)test, test * 2, (StripeLoc)loc);
    }
}

TEST_F(MapIoHandlerTest, FlushSingleDirtyPage)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushSingleDirtyPage ********************");
    MpageList dirtyPages;
    numMapsToFlush = 1;
    numMapsFlushed = 0;
    flushCompleted = false;

    BlkAddr rba = 0;
    _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, 3, 3, dirtyPages);

    _FlushDirtyPagesGiven(TEST_VOL_ID, dirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _GetAndCompareVSA(TEST_VOL_ID, rba, 3, 3);

    _SimulateSPOR();

    _GetAndCompareVSA(TEST_VOL_ID, rba, 3, 3);
}

TEST_F(MapIoHandlerTest, FlushDirtyPages)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushDirtyPages ********************");
    MpageList dirtyPages;
    numMapsToFlush = 1;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128, dirtyPages);
    }

    _FlushDirtyPagesGiven(TEST_VOL_ID, dirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        _GetAndCompareVSA(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }
}

TEST_F(MapIoHandlerTest, FlushDirtyMaps)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushDirtyMaps ********************");
    MpageList vsaMapDirtyPages;
    MpageList stripeMapDirtyPages;

    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128, vsaMapDirtyPages);
    }

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _StripeMapDirtyUpdateTester(vsid, lsid, IN_WRITE_BUFFER_AREA, stripeMapDirtyPages);
    }

    _FlushDirtyPagesGiven(TEST_VOL_ID, vsaMapDirtyPages);
    _FlushDirtyPagesGiven(STRIPE_MAP_ID, stripeMapDirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        _GetAndCompareVSA(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _GetAndCompareLSA(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

TEST_F(MapIoHandlerTest, AsyncFlushMap)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsyncFlushMap ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _SetVSAs(TEST_VOL_ID, rba, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _SetLSA(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }

    _FlushTouchedPages(TEST_VOL_ID);
    _FlushTouchedPages(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        _GetAndCompareVSA(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _GetAndCompareLSA(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

TEST_F(MapIoHandlerTest, AsyncFlushEmptyMap)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsyncFlushEmptyMap ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    _FlushTouchedPages(TEST_VOL_ID);
    _FlushTouchedPages(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }
}

TEST_F(MapIoHandlerTest, AsnyFlushMapSeveralMpages)
{
    POS_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsnyFlushMapSeveralMpages ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    int numMpagesToTest = 500;

    for (int testCnt = 0; testCnt < numMpagesToTest; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)(testCnt * (PAGE_SIZE / 8));
        _SetVSAs(TEST_VOL_ID, rba, (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < numMpagesToTest; ++testCnt)
    {
        StripeId vsid = (StripeId)(testCnt * (PAGE_SIZE / 4));
        StripeId lsid = (StripeId)testCnt % 128;
        _SetLSA(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }

    _FlushTouchedPages(TEST_VOL_ID);
    _FlushTouchedPages(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < numMpagesToTest; testCnt++)
    {
        _GetAndCompareVSA(TEST_VOL_ID, (BlkAddr)(testCnt * (PAGE_SIZE / 8)), (StripeId)testCnt / 128, (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < numMpagesToTest; testCnt++)
    {
        StripeId vsid = (StripeId)(testCnt * (PAGE_SIZE / 4));
        StripeId lsid = (StripeId)testCnt % 128;
        _GetAndCompareLSA(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

} // namespace pos
