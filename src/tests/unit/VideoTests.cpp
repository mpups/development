#include "VideoTests.h"

#include "../../video/LibAvWriter.h"
#include "../../video/LibAvCapture.h"
#include "../../video/FFmpegCustomIO.h"
#include "../../video/VideoFrame.h"
#include "../../video/FFmpegBufferIO.h"

#include <gtest/gtest.h>

void TestVideo()
{
    // test fourcc code generation:
    int32_t fourcc = LibAvWriter::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, 640*480 );
    ASSERT_EQ( 0, err );

    VideoFrame frame( buffer, PIX_FMT_GRAY8, 640, 480, 480 );

    //  scoped so that LibAvWriter is destroyed at end (and hence file is closed and flushed).
    {
        /** @todo - testing custom IO objects should be separate test: */
        FFMpegFileIO videoOut( "test.avi", false );
        LibAvWriter writer( videoOut );
        ASSERT_TRUE( writer.IsOpen() );

        bool streamCreated = writer.AddVideoStream( 320,240,30, LibAvWriter::FourCc( 'F','M','P','4' ) );
        ASSERT_TRUE( streamCreated );

        for ( int i=0;i<256;++i)
        {
            memset( buffer, i, 640*480 );
            bool frameWritten = writer.PutVideoFrame( frame );
            ASSERT_TRUE( frameWritten );
        }
    }

    {
        // Now try to read back the same video
        FFMpegFileIO videoIn( "test.avi", true );
        LibAvCapture reader( videoIn );
        ASSERT_TRUE( reader.IsOpen() );

        int decodedCount = 0;
        while ( reader.GetFrame() )
        {
            reader.ExtractLuminanceImage( buffer, 640 );
            EXPECT_EQ( buffer[0], decodedCount );
            int64_t timestamp = reader.GetFrameTimestamp_us();
            //std::cout << "Timestamp := " << timestamp << std::endl; /** @todo Implement user settable timestamp and test it here. */
            reader.DoneFrame();
            decodedCount += 1;
        }

        EXPECT_EQ( 256, decodedCount );
    }

    free( buffer );

    // Check unused objects get cleaned up safely:
    char noFile[] = "this_file_should_not_be_created.avi";
    remove( noFile );
    {
        LibAvWriter unusedWriter( noFile ); // Construct with default IO otherwise file might be created
        LibAvCapture reader( "test.avi" );
    }
    // Check no file opened after writer was destroyed:
    struct stat info;
    int fileCreated = stat( noFile, &info );
    EXPECT_EQ( -1, fileCreated ); // should return -1 for non existent file
}

/**
    Test video read/write using FFMpegBufferIO.
*/
void TestBufferIO()
{
    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, 640*480 );
    ASSERT_EQ( 0, err );

    VideoFrame frame( buffer, PIX_FMT_GRAY8, 640, 480, 480 );

    FFMpegBufferIO videoIO( FFMpegBufferIO::WriteBuffer );

    //  scoped so that LibAvWriter is destroyed at end (and hence file is closed and flushed).
    {
        // Write video into memory buffers:
        LibAvWriter writer( videoIO );
        ASSERT_TRUE( writer.IsOpen() );

        bool streamCreated = writer.AddVideoStream( 640, 480, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );
        ASSERT_TRUE( streamCreated );

        for ( int i=0;i<256;++i)
        {
            memset( buffer, i, 640*480 );
            bool frameWritten = writer.PutVideoFrame( frame );
            ASSERT_TRUE( frameWritten );
        }
    }

    {
        // Now try to read video from memory buffers:
        videoIO.ChangeDirection( FFMpegBufferIO::ReadBuffer );

        LibAvCapture reader( videoIO );
        ASSERT_TRUE( reader.IsOpen() );

        int decodedCount = 0;
        while ( reader.GetFrame() )
        {
            reader.ExtractLuminanceImage( buffer, 640 );
            EXPECT_EQ( buffer[0], decodedCount );
            reader.DoneFrame();
            decodedCount += 1;
        }

        EXPECT_EQ( 256, decodedCount );
    }

    free( buffer );
}
