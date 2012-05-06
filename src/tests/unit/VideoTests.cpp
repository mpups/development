#include "VideoTests.h"

#include "../../video_capture/LibAvWriter.h"

#include <gtest/gtest.h>

void TestVideoWrite()
{
    // test fourcc code generation:
    int32_t fourcc = LibAvWriter::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    LibAvWriter writer( "test.avi" );
    ASSERT_TRUE( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( 320,240,30, LibAvWriter::FourCc( 'F','F','V','1') );
    ASSERT_TRUE( streamCreated );

    bool frameWritten = writer.PutGreyFrame( 0, 640, 480, 480 );
    ASSERT_TRUE( frameWritten );
}

void TestVideoRead()
{
}
