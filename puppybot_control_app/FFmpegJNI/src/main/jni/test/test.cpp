#include "ffmpegjni.h"

#include <assert.h>

/**
    Work of genuis test which uses a crudely hacked up mock JavaVM:
*/
int main( int argc, char** argv )
{
    // Create some necessary mock objects:
    JNIEnv mockEnv;
    JavaVM mockVm(&mockEnv);
    jobject mockObject;

    // Call OnLoad to force any setup that might occurr:
    JNI_OnLoad( &mockVm, nullptr );

    jstring streamAddress;
    streamAddress.str = std::string( "localhost" );
    jint port = 2000;
    jint streamId = Java_pups_ffmpegjni_VideoStream_createStream( &mockEnv, mockObject, streamAddress, port, 5 );

    if ( streamId < 0 )
    {
        std::cerr << argv[0] << ": createStream() returned " << streamId << ", did you start a test server?" << std::endl;
    }
    assert( streamId >= 0 );

    jint w = Java_pups_ffmpegjni_VideoStream_getFrameWidth( &mockEnv, mockObject, streamId );
    assert( w != 0 );
    jint h = Java_pups_ffmpegjni_VideoStream_getFrameHeight( &mockEnv, mockObject, streamId );
    assert( h != 0 );
    std::cerr << argv[0] << ": frame dimensions := " << w << "x" << h << std::endl;

    jobject mockBitmapObject;
    Java_pups_ffmpegjni_VideoStream_setFrameReceiver( &mockEnv, mockObject, streamId, mockBitmapObject );

    // All seems to be setup ok so begin streaming:
    jboolean started = Java_pups_ffmpegjni_VideoStream_startStreaming( &mockEnv, mockObject, streamId );

    // Must call this before looping so that the frame number is valid AND we do not
    // test IsStreaming() before the streaming thread has actually started.
    jint lastFrame = Java_pups_ffmpegjni_VideoStream_waitForFrame( &mockEnv, mockObject, streamId );

    // Receive some frames:
    int testFrameCount = 1011;
    while ( testFrameCount != 0 &&
            Java_pups_ffmpegjni_VideoStream_isStreaming( &mockEnv, mockObject, streamId )
          )
    {
        Java_pups_ffmpegjni_VideoStream_sendJoystickUpdate( &mockEnv, mockObject, streamId, 0, 0, 11 );
        jint frameNumber = Java_pups_ffmpegjni_VideoStream_waitForFrame( &mockEnv, mockObject, streamId );
        if ( frameNumber > lastFrame )
        {
//          m_handler.sendEmptyMessage( NEWIMAGE );
            lastFrame = frameNumber;
        }

        testFrameCount -= 1;
    }

    // Delete the stream
    Java_pups_ffmpegjni_VideoStream_deleteStream( &mockEnv, mockObject, streamId );

    std::cerr << "Frames received := " << lastFrame<< std::endl;

    return EXIT_SUCCESS;
}

