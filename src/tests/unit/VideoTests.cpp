#include "VideoTests.h"

#include "../../video/LibAvWriter.h"
#include "../../video/LibAvCapture.h"

#include <gtest/gtest.h>

void TestVideo()
{
    // test fourcc code generation:
    int32_t fourcc = LibAvWriter::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    LibAvWriter writer( "test.avi" );
    ASSERT_TRUE( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( 320,240,30, LibAvWriter::FourCc( 'F','M','P','4' ) );
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
            ASSERT_TRUE( frameWritten );
        }

        // Now try to read back the same video
        LibAvCapture reader( "test.avi" );
        ASSERT_TRUE( reader.IsOpen() );

        int decodedCount = 0;
        while ( reader.GetFrame() )
        {
            reader.ExtractLuminanceImage( buffer, 640 );
            EXPECT_EQ( buffer[0], decodedCount );
            //std::cout << "Timestamp := " << reader.GetFrameTimestamp() << std::endl;
            reader.DoneFrame();
            decodedCount += 1;
        }

        EXPECT_EQ( 256, decodedCount );

        free( buffer );
    }

    // Check unused objects get cleaned up safely:
    {
        LibAvWriter unusedWriter( "this_file_should_not_be_created.txt" );
        LibAvCapture reader( "test.avi" );
    }
    // Check no file opened after writer was destroyed:
    struct stat info;
    int err = stat( "this_file_should_not_be_created.txt", &info );
    ASSERT_EQ( -1, err ); // should return -1 for non existent file
    EXPECT_EQ( ENOENT, errno );
}

