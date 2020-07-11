/**
 * Copyright (C) Mark Pupilli 2012
 *
 * Library which exposes ffmpeg functionality through the JNI.
 *
 * @todo - Add output streaming
 * */

#include <JoystickControl.h>
#include <arpa/inet.h>

#include "ffmpegjni.h"
#include "TcpVideoStream.h"
#include "BitmapFrameReceiver.h"
#include "StreamManager.h"

inline void logi( const char* tag, const char* info )
{
    __android_log_write(ANDROID_LOG_INFO, tag, info );
}

char g_logBuffer[256] = "";
JavaVM* g_jvm = 0;
StreamManager g_manager;
std::unique_ptr<TcpSocket> g_socket;
std::unique_ptr<PacketMuxer> g_muxer;

void GlobalCommsShutdown()
{
	g_muxer.reset();
	g_socket->Shutdown();
	g_socket.reset();
}

extern "C" {

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    g_jvm = vm;
    return JNI_VERSION_1_2;
}

/**
    Create a stream connected to the IP address specified as a Java string object.

	@param addressString IP address of server
	@param port Port on which server resides
	@param timeoutInSeconds If no video packets are received for this duration in seconds then streaming will be terminated (by this end).
    @return a valid ID for the new stream if successful, or < 0 on error.
*/
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_createStream( JNIEnv* env, jobject object, jstring addressString, jint port, jint timeoutInSeconds )
{
    if ( g_manager.GetStreamCount() != 0 )
    {
        return (jint) -1; // Only supports a single stream at the moment.
    }

    logi( __PRETTY_FUNCTION__, "Connecting socket." );
    const char* ipString = env->GetStringUTFChars( addressString, 0 );
    g_socket.reset( new TcpSocket() );
    bool connected = g_socket->Connect( ipString, port );
    env->ReleaseStringUTFChars( addressString, ipString );

    if ( connected == false )
    {
        logi( __PRETTY_FUNCTION__, "Could not connect socket." );
        return (jint) -1;
    }

    g_muxer.reset( new PacketMuxer( *g_socket, g_packetIds ) );

    logi( __PRETTY_FUNCTION__, "Creating video client." );
    int newIndex = g_manager.NewStream( *g_socket, timeoutInSeconds );
    if ( newIndex == -1 )
    {
        logi( __PRETTY_FUNCTION__, "Could not connect video stream." );
        return (jint) -1;
    }

    snprintf( g_logBuffer, sizeof(g_logBuffer), "Created video stream %d", newIndex );
    logi( __PRETTY_FUNCTION__, g_logBuffer );
    return (jint) newIndex;
}

/**
    @note Can return 0 if stream info is not available yet.
*/
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameWidth( JNIEnv* env, jobject object, jint streamId )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
        int w,h;
        stream->GetFrameSize(w,h);
        return w;
    }

    logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    return 0;
}

/**
    @note Can return 0 if stream info is not available yet.
*/
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameHeight( JNIEnv* env, jobject object, jint streamId )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
        int w,h;
        stream->GetFrameSize(w,h);
        return h;
    }

    logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    return 0;
}

JNIEXPORT jlong JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameCount( JNIEnv* env, jobject object, jint streamId )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
    	return (jlong)stream->GetFrameCount();
    }

    return 0;
}

JNIEXPORT jdouble JNICALL
Java_pups_ffmpegjni_VideoStream_getStamp( JNIEnv* env, jobject object, jint streamId )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
    	return (jdouble)stream->GetStamp();
    }

    return -1.0;
}

JNIEXPORT jfloat JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameRate( JNIEnv* env, jobject object, jint streamId )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
    	return (jfloat)stream->GetFrameRate();
    }
    else
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    }

    return 0.f;
}

/**
    Setup the specified stream so that frames are copied into to the specified bitmap as they are received.
    Extraction happens asynchronously and the bitmap's buffer is locked for the duration of frame extraction.
    To synchronise with frame extraction call waitForFrame().

    @todo Double buffer the frame extraction (but this needs to be handled at a much lower level).

    @todo Implement other frame receivers - e.g. OpenGL version which writes the frame directly into a texture?

    @param streamId Stream id, if this does not correspond to a vlid stream then the call has no effect.
    @param jbitmap frames captured by the given stream will be decompressed to the bitmap's buffer.
*/
JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_setFrameReceiver( JNIEnv* env, jobject object, jint streamId, jobject jbitmap )
{
	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
        std::unique_ptr<FrameReceiver> receiver( new BitmapFrameReceiver( g_jvm, jbitmap ) );
        stream->SetFrameReceiver( std::move(receiver) );
    }
}

/**
    Start the streaming thread. If a frame receiver has been set (e.g. by calling setFrameReceiver())
    then frames will be begin to be delivered to the receiver.
*/
JNIEXPORT jboolean JNICALL
Java_pups_ffmpegjni_VideoStream_startStreaming( JNIEnv* env, jobject object, jint streamId )
{
    if ( streamId < 0 )
    {
        return (jboolean) false;
    }

	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream == nullptr )
    {
        return (jboolean) false;
    }

    bool started = stream->StartStreaming( g_jvm );
    if ( started )
    {
        logi( __PRETTY_FUNCTION__, "Streaming thread started." );
    }
    return (jboolean) started;
}

/**
    Shutdown the video stream and free resources for a streamId returned from createStream.

    @param streamId a valid stream id.
*/
JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_deleteStream( JNIEnv* env, jobject object, jint streamId )
{
    if ( streamId < 0 )
    {
        return;
    }

    StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream != nullptr )
    {
        stream->StopStreaming();

        snprintf( g_logBuffer, sizeof(g_logBuffer), "Stopping stream %d", streamId );
        logi( __PRETTY_FUNCTION__, g_logBuffer );

        logi( __PRETTY_FUNCTION__, "Deleting stream." );
        g_manager.DeleteStream( streamId );

        logi( __PRETTY_FUNCTION__, "Shutting down socket." );
        GlobalCommsShutdown();
    }
}

/**
    Block until a frame is ready.

    @return the frame number of the frame that is ready.
*/
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_waitForFrame( JNIEnv* env, jobject object, jint streamId )
{
    if ( streamId < 0 )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
        return (jint) -1;
    }

	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream == nullptr )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    	return (jint) -1;
    }


    if ( stream->IsInitialised() == false )
    {
    	logi( __PRETTY_FUNCTION__, "Stream is not initialised" );
    	return (jint) -1;
    }

    return (jint)stream->WaitForFrame();
}

JNIEXPORT jboolean JNICALL
Java_pups_ffmpegjni_VideoStream_isStreaming( JNIEnv* env, jobject object, jint streamId )
{
    if ( streamId < 0 )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
        return (jint) -1;
    }

	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream == nullptr )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    	return (jint) -1;
    }

    return (jboolean) stream->IsStreaming();
}

JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_sendJoystickUpdate( JNIEnv* env, jobject object, jint streamId, jint x, jint y, jint max )
{
    if ( streamId < 0 )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
        return;
    }

	StreamManager::StreamPtr& stream = g_manager[streamId];
    if ( stream == nullptr )
    {
    	logi( __PRETTY_FUNCTION__, "Invalid stream ID" );
    	return;
    }

    if ( g_socket == nullptr || g_socket->IsValid() == false )
    {
    	logi( __PRETTY_FUNCTION__, "Socket is not connected." );
    	return;
    }

    robolib::MuxJoystickData(*g_muxer,x,y,max);
}

} // end extern "C"
