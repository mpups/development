#include "UtilityTests.h"

#include "../../utility/AsyncLooper.h"

#include <gtest/gtest.h>
#include <unistd.h>

void TestAsyncLooper()
{
    constexpr float hertz = 2.f;
    int loopCount = 0;
    bool created = AsyncLooper( hertz,
    [&] {
        loopCount += 1;
    }).ThreadWasCreated();
    ASSERT_TRUE( created );
    ASSERT_TRUE( loopCount >= 1 );
}
