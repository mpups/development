#include "RobotClient.h"

#include "../packetcomms/PacketDemuxer.h"
#include "../packetcomms/PacketMuxer.h"
#include "VideoClient.h"

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

static void message( const char* msg )
{
    std::clog << msg << std::endl;
}

RobotClient::RobotClient()
:
    m_joystick    ( "/dev/input/js0" ),
    m_imageBuffer ( nullptr ),

#ifndef ARM_BUILD
    m_display(320,240)
#endif // ARM_BUILD
{
}

RobotClient::~RobotClient()
{
    m_client.Shutdown();
    free( m_imageBuffer );
}

bool RobotClient::Connect( const char* host, int port )
{
    m_client.SetNagleBufferingOff();
    bool connected = m_client.Connect( host, port );
    if ( connected )
    {
        m_demuxer.reset( new PacketDemuxer( m_client ) );
        m_muxer.reset( new PacketMuxer( m_client ) );
    }
    return connected;
}

bool RobotClient::RunCommsLoop()
{
    assert( m_demuxer != nullptr );

    if ( m_joystick.IsAvailable() )
    {
        m_joystick.Start();
    }

    m_videoClient.reset( new VideoClient( *m_demuxer ) );

    if ( m_videoClient->InitialiseVideoStream() == false )
    {
        message( "Could not initialise video stream." );
        return false;
    }

    const int w = m_videoClient->GetFrameWidth();
    const int h = m_videoClient->GetFrameHeight();
    // Create a buffer for image data:
    int err = posix_memalign( (void**)&m_imageBuffer, 16, w * h * 3 * sizeof(uint8_t) );
    assert( err == 0 );
#ifndef ARM_BUILD
    SetupImagePostData( w, h );
#endif

    int numFrames = 0;

    struct timespec t1;
    struct timespec t2;
    clock_gettime( CLOCK_MONOTONIC, &t1 );

    bool gotFrame = true;

#ifndef ARM_BUILD
    while ( m_display.IsRunning() && gotFrame )
#else
    while ( gotFrame )
#endif
    {
        /// @todo - having SendJoystickData() in same thread as ReceiveVideoFrame()
        /// means that if  ReceiveVideoFrame() blocks (because the server is not sending video),
        /// then no joystick data is sent from the client which, in-turn, causes the server to
        /// shutdown for safety because it is receiving no control.
        SendJoystickData();
        gotFrame = m_videoClient->ReceiveVideoFrame( [this]( LibAvCapture& stream ){
            stream.ExtractBgrImage( m_imageBuffer, stream.GetFrameWidth()*3 );
        });

        if ( gotFrame )
        {
#ifndef ARM_BUILD
            m_display.PostImage( m_postData );
#endif
            numFrames += 1;
            if ( numFrames == 45 ) // Output rolling averages after certain number of frames
            {
                clock_gettime( CLOCK_MONOTONIC, &t2 );

                double secs = (milliseconds(t2) - milliseconds(t1))/1000.0;
                double bits_per_sec = m_videoClient->ComputeVideoBandwidthConsumed( secs );

                std::clog << "Through-put: " << numFrames/secs << " fps @ " << bits_per_sec/(1024.0*1024.0) << "Mbps" << std::endl;
                numFrames = 0;
                clock_gettime( CLOCK_MONOTONIC, &t1 );
            }
        }
        else
        {
            message( "Could not get video frame." );
        }
    }

    return true;
}

void RobotClient::SendJoystickData()
{
    constexpr int dataSize = 3;
    int32_t joyData[dataSize] = { htonl(0), htonl(0), htonl(1024) };

    if ( m_joystick.IsAvailable() )
    {
        // Read joystick and send
        joyData[0] = htonl( m_joystick.GetAxis(1) ); // left hat-stick on ps3 controller
        joyData[1] = htonl( m_joystick.GetAxis(2) ); // right hat-stick on ps3 controller
        joyData[2] = htonl( 32767 );
    }

    m_muxer->EmplacePacket( ComPacket::Type::Joystick, reinterpret_cast<uint8_t*>(joyData), dataSize*sizeof(int32_t) );
}

void RobotClient::SetupImagePostData( int w, int h )
{
    // Initialise the post data for sending to the display window:
    m_postData.mode = GLK::ImageWindow::FixedAspectRatio;
    m_postData.w = w;
    m_postData.h = h;
    m_postData.stride = w*3;
    m_postData.ptr = m_imageBuffer;
    m_postData.isColourBgr = true;
}
