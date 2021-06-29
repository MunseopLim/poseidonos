#include "src/io/general_io/array_unlocking.h"
#include "test/unit-tests/array/service/io_locker/i_io_locker_mock.h"

#include <gtest/gtest.h>

using ::testing::_;
namespace pos
{
TEST(ArrayUnlocking, ArrayUnlocking_Constructor)
{
    // Given : User Data and simple partition type
    PartitionType partitionType = PartitionType::USER_DATA;
    StripeId stripeId = 0;
    const std::string str = "array";
    MockIIOLocker mockIIOLocker;
    // When : Constructor
    ArrayUnlocking arrayUnlocking(partitionType, stripeId, str, &mockIIOLocker);
}

TEST(ArrayUnlocking, ArrayUnlocking_DoSpecificJob)
{
    // Given : User Data and simple partition type
    PartitionType partitionType = PartitionType::USER_DATA;
    StripeId stripeId = 0;
    const std::string str = "array";
    MockIIOLocker mockIIOLocker;
    // When : Constructor
    ArrayUnlocking arrayUnlocking(partitionType, stripeId, str, &mockIIOLocker);
    Callback *callback = &arrayUnlocking;
    // Then : Execute and check result
    EXPECT_CALL(mockIIOLocker, Unlock(_, _));
    bool actual = callback->Execute();
    ASSERT_EQ(actual, true);
}

} // namespace pos
