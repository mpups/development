#include "VideoTests.h"

#include "../../video_capture/LibAvWriter.h"
#include "../../video_capture/LibAvCapture.h"

#include <gtest/gtest.h>

void TestVideo()
{
    // test fourcc code generation:
    int32_t fourcc = LibAvWriter::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    LibAvWriter writer( "test.avi" );
    ASSERT_TRUE( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( 320,240,60, LibAvWriter::FourCc( 'F','F','V','1' ) );
    ASSERT_TRUE( streamCreated );

    if ( streamCreated )
    {
        uint8_t* buffer;
        int err = posix_memalign( (void**)&buffer, 16, 640*480 );
        ASSERT_EQ( 0, err );

        for ( int i=0;i<256;++i)
        {
            memset( buffer, i, 640*480 );
            bool frameWritten = writer.PutGreyFrame( buffer, 640, 480, 480 );
            EXPECT_TRUE( frameWritten );
        }

        // Now try to read back the same video
        LibAvCapture reader( "test.avi" );
        ASSERT_TRUE( reader.IsOpen() );

        for ( unsigned int i=0;i<256;++i)
        {
            reader.GetFrame();
            reader.ExtractLuminanceImage( buffer, 640 );
            EXPECT_EQ( buffer[0], i );
            //std::cout << "Timestamp := " << reader.GetFrameTimestamp() << std::endl;
            reader.DoneFrame();
        }

        free( buffer );
    }
}

