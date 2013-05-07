#include "UtilityTests.h"

#include "../../utility/SimpleAsyncFunction.h"

#include <gtest/gtest.h>
#include <unistd.h>

void TestSimpleAsyncFunction()
{
    constexpr int HundredMilliSecs = 100000;

    // Check a lambda gets executed:
    {
        bool executed = false;
        bool created = SimpleAsyncFunction( [&]() {
            executed = true;
        }).ThreadWasCreated();
        ASSERT_TRUE( created );
        ASSERT_TRUE( executed );
    }

    // Check join happens on destruction:
    bool running = true;
    { // Scope of the async execution:
        // Note the object needs to be given a name here - otherwise it
        // is immediately destructed and go is never set to false.
        volatile bool go = true;
        SimpleAsyncFunction func( [&]() {
            while ( go == true )
            {
            }
            usleep( HundredMilliSecs ); // sleep so that assertion gets chance to run if thread is not joined.
            running = false;
        });
            EXPECT_TRUE( func.ThreadWasCreated() );
        go = false;
    }
    // Join should happen at end of scope above (hence running should be set to false).
    ASSERT_FALSE( running );
}
