/**
    Copyright (c) mark Pupilli 2011


    This file specifies and runs all unit tests in robolib.
**/

#include <gtest/gtest.h>

#include "NetworkingTests.h"
#include "ImageTests.h"
#include "VideoTests.h"

TEST( io, networking )
{
    TestIpv4Address();
    TestTcp();
    TestUdp();
}

#ifndef ARM_BUILD
TEST( io, video )
{
    TestVideo();
    //TestBufferIO();
}
#endif

TEST( vision, image )
{
    robo::TestImage();
    robo::TestImageFill();
    robo::SadTest();
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

