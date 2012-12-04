#include "VideoTests.h"

#include "../../video/LibAvWriter.h"
#include "../../video/LibAvCapture.h"
#include "../../video/FFmpegCustomIO.h"
#include "../../video/VideoFrame.h"
#include "../../video/FFmpegBufferIO.h"
#include "../../video/FFmpegStdFunctionIO.h"

#include <gtest/gtest.h>

const int FRAME_WIDTH   = 640;
const int FRAME_HEIGHT  = 480;
const int STREAM_WIDTH  = 320;
const int STREAM_HEIGHT = 240;

/**
    Creates a LibAvWriter which writes a test video using the
    specified IO object.
*/
void RunWriter( FFMpegCustomIO& videoIO )
{
    // Write video into memory buffers:
    LibAvWriter writer( videoIO );
    ASSERT_TRUE( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( STREAM_WIDTH, STREAM_HEIGHT, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );
    ASSERT_TRUE( streamCreated );

    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, 640*480 );
    ASSERT_EQ( 0, err );

    VideoFrame frame( buffer, PIX_FMT_GRAY8, FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH );

    for ( int i=0;i<256;++i)
    {
        memset( buffer, i, FRAME_WIDTH*FRAME_HEIGHT );
        bool frameWritten = writer.PutVideoFrame( frame );
        ASSERT_TRUE( frameWritten );
    }

    free( buffer );
}

void RunReader( FFMpegCustomIO& videoIn )
{
    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, FRAME_WIDTH*FRAME_HEIGHT );
    ASSERT_EQ( 0, err );

    // Now try to read back the same video
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

    free( buffer );
}

/**
    Test video read/write using FFMpegFileIO.
*/
void TestVideo()
{
    // test fourcc code generation:
    int32_t fourcc = LibAvWriter::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    FFMpegFileIO videoOut( "test.avi", false );
    RunWriter( videoOut );

    FFMpegFileIO videoIn( "test.avi", true );
    RunReader( videoIn );

    // Check unused objects get cleaned up safely:
    char noFile[] = "this_file_should_not_be_created.avi";
    ::remove( noFile );
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
    FFMpegBufferIO videoIO( FFMpegCustomIO::WriteBuffer );
    RunWriter( videoIO );
    videoIO.ChangeDirection( FFMpegCustomIO::ReadBuffer );
    RunReader( videoIO );
}

void TestStdFunctionIO()
{
    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, FRAME_WIDTH*FRAME_HEIGHT );
    ASSERT_EQ( 0, err );

    FFMpegStdFunctionIO videoOut( FFMpegCustomIO::WriteBuffer, [](uint8_t* buffer, int size){ std::cerr << "Write of " << size << " bytes requested." << std::endl; return size; } );
    RunWriter( videoOut ); // We only test the writer because The lambda funciton above doesn't actually write any data for the reader.
}
