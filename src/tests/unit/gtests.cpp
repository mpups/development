/**
    Copyright (c) mark Pupilli 2011


    This file specifies and runs all unit tests in robolib.
**/

#include <gtest/gtest.h>

#include "NetworkingTests.h"

TEST( io, networking )
{
    TestIpv4Address();
    TestTcp();
    TestUdp();
}   

/**
    Runs all the tests listed above.    
**/
int main( int argc, char** argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    int v = RUN_ALL_TESTS();
    return v;
}

