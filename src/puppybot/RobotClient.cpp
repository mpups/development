#include "RobotClient.h"

#include "../packetcomms/PacketDemuxer.h"
#include "../packetcomms/PacketMuxer.h"
#include "../robotcomms/VideoClient.h"
#include "../utility/AsyncLooper.h"

#include <arpa/inet.h>

static void message( const char* msg )
{
    std::clog << msg << std::endl;
}

RobotClient::RobotClient()
:
    m_joystick    ( "/dev/input/js0" ),
    m_imageBuffer ( nullptr )

#ifndef ARM_BUILD
    ,m_display(320,240)
#endif // ARM_BUILD
{
}

RobotClient::~RobotClient()
{
    m_client.Shutdown();
    free( m_imageBuffer );
}

bool RobotClient::Connect( const char* host, int port, const std::vector<std::string>& packetTypes )
{
    //m_client.SetNagleBufferingOff();
    bool connected = m_client.Connect( host, port );
    if ( connected )
    {
        m_demuxer.reset( new PacketDemuxer( m_client, packetTypes ) );
        m_muxer.reset( new PacketMuxer( m_client, packetTypes ) );
        SendJoystickData();
    }
    return connected;
}

bool RobotClient::RunCommsLoop()
{
    assert( m_demuxer != nullptr );

    // Send joystick data at 15Hz:
    constexpr float joystickSendRateHz = 15;
    AsyncLooper joyThread( joystickSendRateHz,
                           std::bind( &RobotClient::SendJoystickData, std::ref(*this))
                          );

    // Testing: integrate odometry into position:
    const float wheelBaseInMetres = 0.28f;
    const float countsPerMetre = 6850.f;
    const float metresPerCount = 1.f / countsPerMetre;
    float x  = 0.f;
    float y  = 0.f;
    float th = 0.f;
    int32_t lastCountLeft  = 0;
    int32_t lastCountRight = 0;

    PacketSubscription odometrySubscriber = m_demuxer->Subscribe( "Odometry", [&]( const ComPacket::ConstSharedPacket& packet ) {
        DiffDrive::MotorData odometry;
        std::copy( packet->GetDataPtr(), packet->GetDataPtr()+packet->GetDataSize(), reinterpret_cast<uint8_t*>(&odometry) );
        if ( odometry.valid )
        {
            const float dLeft  = (odometry.leftPos - lastCountLeft) * metresPerCount;
            const float dRight = (odometry.rightPos - lastCountRight) * metresPerCount;

            // Compute delta in robot's local coordinates:
            float d = ( dLeft + dRight ) * .5f;
            float wdt = (dRight - dLeft) / wheelBaseInMetres;
            const float dx =  cos(wdt) * d;
            const float dy = -sin(wdt) * d;

            const float costh = cos(th);
            const float sinth = sin(th);
            x += costh*dx - sinth*dy;
            y += sinth*dx + costh*dy;
            th += wdt;

            //std::cout << "Odometry: left pos := " << odometry.leftPos << " right pos := " << odometry.rightPos << std::endl;
            //std::cout << "Distances : left := " << dLeft << "right : " << dRight << std::endl;
            //std::cout << "costh := " << costh << std::endl;
            //std::cout << "(x,y) - deg := " << x << "," << y << " - " << th * (180/3.141592654) << std::endl;

            lastCountLeft = odometry.leftPos;
            lastCountRight = odometry.rightPos;
        }
        else
        {
            //std::cout << "Odometry packet containing invalid data" << std::endl;
        }
    });

    m_videoClient.reset( new VideoClient( *m_demuxer ) );

    constexpr std::chrono::seconds timeOutForInit = std::chrono::seconds(5);
    if ( m_videoClient->InitialiseVideoStream( timeOutForInit ) )
    {
        LoopToReceiveVideo();
        return true;
    }
    else
    {
        message( "Could not initialise video stream." );
        return false;
    }
}

void RobotClient::LoopToReceiveVideo()
{
    const int w = m_videoClient->GetFrameWidth();
    const int h = m_videoClient->GetFrameHeight();
    std::clog << "Received frame dimensions: " << w << "x" << h << std::endl;

    // Create a buffer for image data:
    int err = posix_memalign( (void**)&m_imageBuffer, 16, w * h * 3 * sizeof(uint8_t) );
    assert( err == 0 );
    SetupImagePostData( w, h );

    int numFrames = 0;

    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    bool gotFrame = true;

#ifndef ARM_BUILD
    while ( m_display.IsRunning() && gotFrame )
#else
    while ( gotFrame )
#endif
    {
        gotFrame = m_videoClient->ReceiveVideoFrame( [this]( LibAvCapture& stream ){
            stream.ExtractBgrImage( m_imageBuffer, stream.GetFrameWidth()*3 );
        });

        if ( gotFrame )
        {
#ifndef ARM_BUILD
            m_display.PostImage( m_postData );
#endif
            numFrames += 1;
            if ( numFrames == 45 ) // Output some rolling averages after certain number of frames
            {
                const auto t2 = std::chrono::steady_clock::now();
                double secs = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()/1000.0;
                double bits_per_sec = m_videoClient->ComputeVideoBandwidthConsumed( secs );

                std::clog << "Through-put: " << numFrames/secs << " fps @ " << bits_per_sec/(1024.0*1024.0) << " Mbps" << std::endl;
                numFrames = 0;
                t1 = std::chrono::steady_clock::now();
            }
        }
        else
        {
            message( "Could not get video frame." );
        }
    }
}

void RobotClient::SendJoystickData()
{
    if ( m_joystick.IsAvailable() )
    {
        robolib::MuxJoystickData(*m_muxer,m_joystick.GetAxis(1),m_joystick.GetAxis(2),32767);
    }
    else
    {
        robolib::MuxJoystickData(*m_muxer,0,0,32767);
    }
}

void RobotClient::SetupImagePostData( int w, int h )
{
#ifndef ARM_BUILD
    // Initialise the post data for sending to the display window:
    m_postData.mode = GLK::ImageWindow::FixedAspectRatio;
    m_postData.w = w;
    m_postData.h = h;
    m_postData.stride = w*3;
    m_postData.ptr = m_imageBuffer;
    m_postData.isColourBgr = true;
#endif
}

