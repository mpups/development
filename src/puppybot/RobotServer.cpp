#include "RobotServer.h"

const int IMG_WIDTH  = 320;
const int IMG_HEIGHT = 240;

#include <time.h>

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

/**
    Setup a robot server with specified TCP and serial ports.
    
    TCP is used for a remote tele-link and the serial port is used for differential drive control (if available).
**/
RobotServer::RobotServer( int tcpPort, const char* motorSerialPort )
:
    m_serialPort( motorSerialPort ),
    m_server( new TcpSocket() )
{
    // Setup a server socket for receiving client commands:
    if ( m_server->Bind( tcpPort ) == false )
    {
        m_server.reset();
    }
}

RobotServer::~RobotServer()
{
}

/**
    Blocks until robot gets a connection.
**/
bool RobotServer::Listen()
{
    if ( m_server )
    {
        fprintf( stderr, "Waiting for new connection...\n" );
        m_server->Listen( 0 ); // Wait for connection - no queue
        m_con.reset( m_server->Accept() ); // Create connection
        m_con->SetBlocking( false );

        PostConnectionSetup();
        return true;
    }
    else
    {
        fprintf( stderr, "Error: Server socket invalid.\n" );
        return false;
    }
}

/**
    Perform post conection processing.
    
    Attempts to access camera and wheels.
**/
void RobotServer::PostConnectionSetup()
{
    // Setup comms to motors:
    m_motors.reset( new MotionMind( m_serialPort.cStr() ) );
    if ( m_motors->Available() )
    {
        m_drive.reset( new DiffDrive( *m_motors ) );
        float amps = 1.5f;
        int32_t currentLimit = roundf( amps/0.02f );
        int32_t pwmLimit = (72*1024)/120; // motor voltage / battery voltage
        
        m_motors->WriteRegister( 1, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 2, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 1, MotionMind::PWMLIMIT, pwmLimit );
        m_motors->WriteRegister( 2, MotionMind::PWMLIMIT, pwmLimit );
    }
    else
    {
        m_motors.reset();
    }

    // Setup camera:
    m_camera.reset( new UnicapCamera() );
    size_t imageBufferSize = m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t);
    if ( m_camera->IsOpen() )
    {
        m_camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        m_camera.reset();
    }

    assert( m_con.get() != nullptr );
    m_muxer.reset( new PacketMuxer( *m_con ) );
    m_demuxer.reset( new PacketDemuxer( *m_con ) );
}

/**
    Cleans up resources once the comms loop has finished.
**/
void RobotServer::PostCommsCleanup()
{
    // These must be deleted in this order:
    m_drive.reset();
    m_motors.reset();
    
    if ( m_camera )
    {
        m_camera->StopCapture();
        m_camera.reset();
        free( m_lum );
    }
}

/**
    Runs the robot's comms loop until the connection terminates or fails.
**/
void RobotServer::RunCommsLoop()
{
    if ( m_muxer != nullptr && m_demuxer != nullptr )
    {
        Ipv4Address clientAddress;
        m_con->GetPeerAddress( clientAddress );

        {
            std::string name;
            clientAddress.GetHostName( name );
            fprintf( stderr, "Client %s connected to robot.\n", name.c_str() );
        }

        // Setup a TeleJoystick object:
        TeleJoystick teljoy( *m_demuxer, m_drive.get() ); // Will start receiving and processing remote joystick cammands immediately.
        teljoy.Go();
        while ( teljoy.IsRunning() == false )
        {
            GLK::Thread::Sleep( 20 );
        }

        // Start capturing and transmitting images:
        if ( m_camera )
        {
            StreamVideo( teljoy );
        }
        else
        {
            // No video so this thread can just sleep while joystick control runs:
            while ( teljoy.IsRunning() ) {
                sleep( 100 );
            }
        }

        fprintf( stderr, "Control terminated\n" );
    } // end if

    m_muxer.reset();
    m_demuxer.reset();
    m_con->Shutdown();

    PostCommsCleanup();
}

/**
  @param joy This is only used to ensure that the streaming loop exits when joystick task exits.

    Assumptions:
        m_con is not null.
*/
void RobotServer::StreamVideo( TeleJoystick& joy )
{
    assert( m_con );

    // Lambda function that enqueues video packets via the Muxing system:
    FFMpegStdFunctionIO videoIO( FFMpegCustomIO::WriteBuffer, [this]( uint8_t* buffer, int size ) {
        m_muxer->EmplacePacket( ComPacket::Type::AvData, buffer, size );
        return m_muxer->Ok() ? size : -1;
    });
    LibAvWriter streamer( videoIO );

    // Setup an MPEG4 video stream for half-size video:
    int streamWidth = 320; // m_camera->GetFrameWidth()
    int streamHeight = 240; // m_camera->GetFrameHeight()
    streamer.AddVideoStream( streamWidth, streamHeight, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );

    struct timespec t1;
    struct timespec t2;

    int w = m_camera->GetFrameWidth();
    int h = m_camera->GetFrameHeight();

    bool sentOk = true;
    clock_gettime( CLOCK_MONOTONIC, &t1 );

    // Wrap the camera's internal buffer with a VideoFrame object (this assumes the camera's buffer never gets moved/reallocated):
    VideoFrame frame( m_camera->UnsafeBufferAccess(), PIX_FMT_YUV420P, w/2, h/2, w/2 ); // YUV420P is native for mpg4

    // Get-frame must be last in this condition because if it suceeds DoneFrame() must be called:
    while ( sentOk && joy.IsRunning() && m_camera->GetFrame() )
    {
        clock_gettime( CLOCK_MONOTONIC, &t2 );

        //halfscale_yuyv422_to_yuv420p( w, h, m_camera->UnsafeBufferAccess(), yuv420p );
        sentOk = streamer.PutVideoFrame( frame );

        m_camera->DoneFrame();
        sentOk &= !streamer.IoError();

        double grabTime = milliseconds(t2) - milliseconds(t1);

        fprintf( stderr, "%f %f %f %f %f\n", grabTime, m_camera->GetFrameTimestamp_us()/1000.0,streamer.lastConvertTime_ms, streamer.lastEncodeTime_ms, streamer.lastPacketWriteTime_ms );
        clock_gettime( CLOCK_MONOTONIC, &t1 );
    }
}

