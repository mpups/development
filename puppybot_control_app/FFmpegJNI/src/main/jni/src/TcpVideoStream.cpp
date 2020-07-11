#include "TcpVideoStream.h"

#include "FrameReceiver.h"
#include "BitmapFrameReceiver.h"

#include <PacketDemuxer.h>
#include <VideoClient.h>

inline double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

void* TcpVideoStream::streaming_thread( void* data )
{
    TcpVideoStream* stream = reinterpret_cast<TcpVideoStream*>(data);

    // Get the java environment for this new thread:
    JNIEnv *env;
    stream->m_jvm->AttachCurrentThread( &env, 0 );

    // Run the main streaming loop:
    stream->m_streaming = true;
    stream->StreamingLoop( env );
    stream->m_streaming = false;

    stream->m_jvm->DetachCurrentThread();

    stream->SignalFrame(); // Need to signal frame so that other threads waiting and testing IsStreaming() don't block forever

    return 0;
}

/**
    Create an input video stream (i.e. for receiving video).

    @todo Make it so that the constructor establishes the network connection but does not begin streaming the video - this would
    require the server-side to receive flow control commands.
*/
TcpVideoStream::TcpVideoStream( TcpSocket& socket, int timeoutInSeconds )
:
    m_meanFrameRate (0.0),
    m_frameCount    (0),
    m_initialised    (false),
    m_streaming     (false)
{
    pthread_mutex_init( &m_mutex, 0 );
    pthread_cond_init( &m_cond, 0 );

    __android_log_print( ANDROID_LOG_DEBUG, "FFmpegJNI", "Checking socket..." );
    if ( socket.IsValid() )
    {
        socket.SetBlocking( true );

        __android_log_print( ANDROID_LOG_DEBUG, "FFmpegJNI", "Creating comms muxers..." );
        m_demuxer.reset( new PacketDemuxer( socket, g_packetIds ) );
        m_videoClient.reset( new VideoClient( *m_demuxer ) );

        __android_log_print( ANDROID_LOG_DEBUG, "FFmpegJNI", "Initialising the video stream..." );
        m_initialised = m_videoClient->InitialiseVideoStream( std::chrono::seconds(timeoutInSeconds) );
        if ( m_initialised == false )
        {
            __android_log_write( ANDROID_LOG_DEBUG, "FFmpegJNI", "Video stream could not be read." );
        }
        else
        {
            int w = m_videoClient->GetFrameWidth();
            int h = m_videoClient->GetFrameHeight();
            __android_log_print( ANDROID_LOG_DEBUG, "FFmpegJNI", "Frame size := %dx%d", w, h );

            PacketSubscription avinfo = m_demuxer->Subscribe("AvInfo",[this](const ComPacket::ConstSharedPacket& pkt)
			{
				AvInfo info;
				Deserialise( pkt, info.stamp, info.frameNumber );
				m_avinfo = info;
			});

            m_subscription.reset( new PacketSubscription(std::move(avinfo)) );

            return; // success so return without calling DestroyComponents()
        }
    }
    else
    {
        __android_log_write( ANDROID_LOG_DEBUG, "FFmpegJNI", "Could not connect." );
    }

    // If we reach here there was an error so release all components:
    DestroyComponents();
}

TcpVideoStream::~TcpVideoStream()
{
    DestroyComponents();

    pthread_mutex_destroy( &m_mutex );
    pthread_cond_destroy( &m_cond );
}

/**
    Spawn a thread to continuously receive images.

    As soon as the sending side accepts a socket connection it will start sending images hence
    this should be called 'as soon as possible' after construction (recommended to call it immediately).
    @todo - add some control so that streaming doesn't begin until a 'start' command is sent.

    @return true if the thread was successfully created.

    @note a return value of true only means that the thread was successfully created - the actual video
    stream creation might still fail, in which case the thread will stop.
*/
bool TcpVideoStream::StartStreaming( JavaVM* jvm )
{
    if ( !IsInitialised() )
    {
        return false;
    }

    m_jvm     = jvm;
    int err   = pthread_create( &m_thread, 0, TcpVideoStream::streaming_thread, this );
    return err == 0;
}

void TcpVideoStream::StopStreaming()
{
    m_terminate = true;
    pthread_join( m_thread, 0 );
}

void TcpVideoStream::DestroyComponents()
{
    m_subscription.reset();
    m_videoClient.reset();
    m_demuxer.reset();
}

/**
    @return true if the video client was able to correctly interpret the incoming stream.
*/
bool TcpVideoStream::IsInitialised() const
{
    return m_initialised;
}

/**
    @return true if the streaming thread is running.
*/
bool TcpVideoStream::IsStreaming() const
{
    return m_streaming;
}

/**
    Block until a new frame is ready. The streamed video is received by
    a separate thread - this function can be called by a monitoring
    thread so that it can determine when a new frame is ready to be
    processed or displayed.

    @return the frame number for the frame that is ready.
*/
int32_t TcpVideoStream::WaitForFrame()
{
    int32_t rval;
    pthread_mutex_lock( &m_mutex );
    pthread_cond_wait( &m_cond, &m_mutex );
    rval = m_frameCount;
    pthread_mutex_unlock( &m_mutex );
    return rval;
}

/**
    Get frame dimensions.
*/
void TcpVideoStream::GetFrameSize( int& w, int& h ) const
{
    if ( m_videoClient.get() != nullptr )
    {
        w = m_videoClient->GetFrameWidth();
        h = m_videoClient->GetFrameHeight();
    }
    else
    {
        w = h = 0;
    }
}

int64_t TcpVideoStream::GetFrameCount() const
{
	return m_avinfo.frameNumber;
}

double TcpVideoStream::GetStamp() const
{
	return m_avinfo.stamp.tv_sec + (m_avinfo.stamp.tv_nsec*0.000000001);
}

float TcpVideoStream::GetFrameRate() const
{
	return m_meanFrameRate;
}

void TcpVideoStream::StreamingLoop( JNIEnv* env )
{
    m_terminate = false;

    struct timespec t1;
    struct timespec t2;
    clock_gettime( CLOCK_MONOTONIC, &t1 );

    m_frameCount = 0;
    int numFrames = 0;
    bool gotFrame = true;
    while ( m_terminate == false && gotFrame == true )
    {
        gotFrame = m_videoClient->ReceiveVideoFrame( [&]( LibAvCapture& capture ) {
            if ( m_frameReceiver )
            {
                m_frameReceiver->LockReceiver();
                uint8_t* buffer = m_frameReceiver->GetBuffer();
                if ( buffer )
                {
                    // @todo - receiver could require different formats so API should probably expose PixelFormat:
                    capture.ExtractRgb565Image( m_frameReceiver->GetBuffer(), m_frameReceiver->GetStride() );
                }
                m_frameReceiver->UnlockReceiver();

                numFrames += 1;
                SignalFrame();
            }
        });

        if ( numFrames == MEAN_FRAME_WINDOW )
        {
            clock_gettime( CLOCK_MONOTONIC, &t2 );
            double secs = (milliseconds(t2) - milliseconds(t1))/1000.0;
            m_meanFrameRate = numFrames/secs;

            numFrames = 0;
            clock_gettime( CLOCK_MONOTONIC, &t1 );

            __android_log_print( ANDROID_LOG_DEBUG, "FFmpegJNI", "Frame rate := %f Hz", m_meanFrameRate );
        }
    }
}

/**
    Increment the frame count and signal that a new frame is ready.
*/
void TcpVideoStream::SignalFrame()
{
    pthread_mutex_lock( &m_mutex );
    m_frameCount += 1;
    pthread_cond_signal( &m_cond );
    pthread_mutex_unlock( &m_mutex );
}

/**
    Set a frame receiver object. Frames from the video stream will start being passed to the receiver immediately.

    @note In current implementation this has to be set before you call StartStreaming() otherwise SetJNIEnv() will not be called.

    @returns the old receiver object is returned so it can be freed by the caller. It could be null if none was set.
*/
std::unique_ptr<FrameReceiver> TcpVideoStream::SetFrameReceiver( std::unique_ptr<FrameReceiver>&& receiver )
{
    std::unique_ptr<FrameReceiver> old = std::move(m_frameReceiver);
    m_frameReceiver = std::move(receiver);
    return old;
}

