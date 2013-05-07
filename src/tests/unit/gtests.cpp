/**
    Copyright (c) mark Pupilli 2011


    This file specifies and runs all unit tests in robolib.
**/

#include <gtest/gtest.h>

#include "ImageTests.h"
#include "NetworkingTests.h"
#include "UtilityTests.h"
#include "PacketCommsTests.h"

TEST( vision, image )
{
    robo::TestImage();
    robo::TestImageFill();
    robo::SadTest();
}

TEST( network, ip )
{
    TestTcp();
    TestUdp();
    TestIpv4Address();
}

TEST( robolib, utililty )
{
    TestSimpleAsyncFunction();
}

TEST( robolib, packetcomms )
{
    TestComPacket();
    TestSimpleQueue();
    TestPacketMuxer();
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

