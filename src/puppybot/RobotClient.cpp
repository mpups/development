#include "RobotClient.h"

#include "PacketDemuxer.h"
#include "PacketMuxer.h"

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
    m_demuxer     ( nullptr ),
    m_muxer       ( nullptr ),
    m_packetOffset(0),
    m_joystick    ( "/dev/input/js0" ),
    m_imageBuffer ( nullptr ),

#ifndef ARM_BUILD
    m_display(320,240)
#endif // ARM_BUILD

{
}

RobotClient::~RobotClient()
{
    delete m_demuxer;
    delete m_muxer;
    m_client.Shutdown();
    free( m_imageBuffer );
}

bool RobotClient::Connect( const char* host, int port )
{
    m_client.SetNagleBufferingOff();
    bool connected = m_client.Connect( host, port );
    if ( connected )
    {
        m_demuxer = new PacketDemuxer( m_client );
        m_muxer   = new PacketMuxer( m_client );
    }
    return connected;
}

bool RobotClient::RunCommsLoop()
{
    if ( m_joystick.IsAvailable() )
    {
        m_joystick.Start();
    }

    // Lambda which subscribes to the packets containing video-data:
    PacketSubscription sub = m_demuxer->Subscribe( ComPacket::Type::AvData, [&]( const ComPacket::ConstSharedPacket& packet )
    {
        m_avPackets.Emplace( packet ); // The callback simply queues up all the packets.
    });

    if ( InitialiseVideoStream() == false )
    {
        return false;
    }

    int numFrames = 0;
    uint64_t videoBytes = 0;

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
        gotFrame = ReceiveVideoFrame();
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
                uint64_t bytesRx = 0;//m_videoIO->BytesRead();
                double bits_per_sec = ( bytesRx - videoBytes )*(8.0/secs);
                videoBytes = bytesRx;
                std::clog << "Through-put: " << numFrames/secs << " fps @ " << bits_per_sec/(1024.0*1024.0) << "Mbps" << std::endl;
                numFrames = 0;
                clock_gettime( CLOCK_MONOTONIC, &t1 );
            }
        }
    }

    return true;
}

bool RobotClient::InitialiseVideoStream()
{
    // Create a video reader object that uses socket IO:
    m_videoIO.reset( new FFMpegStdFunctionIO( FFMpegCustomIO::ReadBuffer, std::bind( &RobotClient::FfmpegReadPacket, std::ref(*this), std::placeholders::_1, std::placeholders::_2 ) ) );
    m_streamer.reset( new LibAvCapture( *m_videoIO ) );
    if ( m_streamer->IsOpen() == false )
    {
        message( "Could not create stream capture." );
        return false;
    }

    // Get some frames so we can extract correct image dimensions:
    bool gotFrame = false;
    for ( int i=0;i<2;++i )
    {
        gotFrame = m_streamer->GetFrame();
        m_streamer->DoneFrame();
    }

    if ( gotFrame == false )
    {
        message( "Could not interpret video stream." );
        return false;
    }

    const int w = m_streamer->GetFrameWidth();
    const int h = m_streamer->GetFrameHeight();
    std::clog << "Received frame dimensions: " << w << "x" << h << std::endl;

    // Create a buffer for image data:
    int err = posix_memalign( (void**)&m_imageBuffer, 16, w * h * 3 * sizeof(uint8_t) );
    assert( err == 0 );

#ifndef ARM_BUILD
    SetupImagePostData( w, h );
#endif

    return true;
}

bool RobotClient::ReceiveVideoFrame()
{
    bool gotFrame = m_streamer->GetFrame();
    if ( gotFrame )
    {
        m_streamer->ExtractBgrImage( m_imageBuffer, m_streamer->GetFrameWidth()*3 );
        m_streamer->DoneFrame();
    }
    else
    {
        message( "Could not get frame." );
    }

    return gotFrame;
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

/**
    This is bound to a std::function and passed to an FFMpegStdFunctionIO object.
*/
int RobotClient::FfmpegReadPacket( uint8_t* buffer, int size )
{
    SimpleQueue::LockedQueue lock = m_avPackets.Lock();
    while ( m_demuxer->Ok() && m_avPackets.Empty() )
    {
        m_avPackets.WaitNotEmpty( lock );
    }

    // We were asked for more than packet contains so loop through packets until
    // we have returned what we needed or there are no more packets:
    int required = size;
    while ( required > 0 && m_avPackets.Empty() == false )
    {
        const ComPacket::ConstSharedPacket packet = m_avPackets.Front();
        const int availableSize = packet->GetData().size() - m_packetOffset;

        if ( availableSize <= required )
        {
            // Current packet contains less than required so copy the whole packet
            // and continue:
            std::copy( packet->GetData().begin() + m_packetOffset, packet->GetData().end(), buffer );
            m_packetOffset = 0; // Reset the packet offset so the next packet will be read from beginning.
            m_avPackets.Pop();
            buffer += availableSize;
            required -= availableSize;
        }
        else
        {
            assert( availableSize > required );
            // Current packet contains more than enough to fulfill the request
            // so copy what is required and save the rest for later:
            auto startItr = packet->GetData().begin() + m_packetOffset;
            std::copy( startItr, startItr+required, buffer );
            m_packetOffset += required; // Increment the packet offset by the amount read from this packet.
            required = 0;
        }
    }

    return size - required;
}
