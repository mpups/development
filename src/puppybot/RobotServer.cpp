#include "RobotServer.h"
#include "../network/Ipv4Address.h"
#include "../network/TcpSocket.h"
#include "../packetcomms/PacketSerialisation.h"

#include <mutex>
#include <chrono>
#include <condition_variable>

const int IMG_WIDTH  = 320;
const int IMG_HEIGHT = 240;

#include <time.h>

static double ToSeconds( struct timespec& t )
{
    return t.tv_sec + (0.000000001*t.tv_nsec );
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
bool RobotServer::Listen(const std::vector<std::string>& packetTypes)
{
    if ( m_server )
    {
        fprintf( stderr, "Waiting for new connection...\n" );
        m_server->Listen( 0 ); // Wait for connection - no queue
        m_con.reset( m_server->Accept() ); // Create connection
        m_con->SetBlocking( false );

        PostConnectionSetup(packetTypes);
        return true;
    }
    else
    {
        std::clog << "Error: Server socket invalid." << std::endl;
        return false;
    }
}

/**
    Perform post conection processing.
    
    Attempts to access camera and wheels.
**/
void RobotServer::PostConnectionSetup(const std::vector<std::string>& packetTypes)
{
    // Setup comms to motors:
    m_motors.reset( new MotionMind( m_serialPort.c_str() ) );
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
    if ( m_camera->IsOpen() == false )
    {
        m_camera.reset();
        std::clog << "Warning: Could not open camera. Joystick control mode only." << std::endl;
    }

    assert( m_con.get() != nullptr );
    m_muxer.reset( new PacketMuxer( *m_con, packetTypes ) );
    m_demuxer.reset( new PacketDemuxer( *m_con, packetTypes ) );
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
        m_camera.reset();
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
        auto muxerPair = std::make_pair( std::ref(*m_muxer), std::ref(*m_demuxer) );
        TeleJoystick teljoy( muxerPair, m_drive.get() ); // Will start receiving and processing remote joystick cammands immediately.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Start capturing and transmitting images:
        if ( m_camera )
        {
            StreamVideo( teljoy );
        }
        else
        {
            // No video so this thread can just sleep while joystick control runs:
            while ( teljoy.IsRunning() ) {
                sleep( 5 );
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
    assert( m_con != nullptr );

    // Lambda that enqueues video packets via the Muxing system:
    FFMpegStdFunctionIO videoIO( FFMpegCustomIO::WriteBuffer, [&]( uint8_t* buffer, int size ) {
        m_muxer->EmplacePacket( "AvData", reinterpret_cast<VectorStream::CharType*>(buffer), size );
        return m_muxer->Ok() ? size : -1;
    });
    LibAvWriter streamer( videoIO );

    // Setup an MPEG4 video stream for half-size video:
    int w = m_camera->GetFrameWidth();
    int h = m_camera->GetFrameHeight();
    int streamWidth = w/2;
    int streamHeight = h/2;
    streamer.AddVideoStream( streamWidth, streamHeight, 30, video::FourCc( 'F','M','P','4' ) );

    // Allocate double buffers for video conversion:
    uint8_t* m_buffer[2];
    timespec m_timeBuffer[2];
    const std::size_t bufferSize = m_camera->GetFormatBufferSize();
    int err = posix_memalign( (void**)&(m_buffer[0]), 16, bufferSize );
    assert( err == 0 );
    err = posix_memalign( (void**)&(m_buffer[1]), 16, bufferSize );
    assert( err == 0 );

    // Capture control variables:
    std::mutex bufferLock;
    std::condition_variable bufferReady;
    int framesConverted = 0;
    int framesCompressed = 0;

    // This lambda will be called back from a separate frame capture thread.
    // It converts the video frame and then signals the main loop that a new
    // frame is ready to be compressed:
    m_camera->SetCaptureCallback( [&]( const uint8_t* buffer, const timespec& time ) {

        halfscale_yuyv422_to_yuv420p( 640, 480, buffer, m_buffer[0] );
        m_timeBuffer[0] = time;

        { // Start of buffer lock scope.
          // Double buffered so only need to lock mutex while we swap buffers and
          // increment the frame count:
            std::lock_guard<std::mutex> guard(bufferLock);
            framesConverted += 1;
            std::swap( m_buffer[0], m_buffer[1] );
            std::swap( m_timeBuffer[0], m_timeBuffer[1] );
            bufferReady.notify_one();
        }
    });

    m_camera->StartCapture(); // This must not be called before SetCaptureCallback().

    {
        bool sentOk = true;
        std::unique_lock<std::mutex> locker(bufferLock);
        while ( sentOk && joy.IsRunning() )
        {
            // Wait for a new frame to be captured:
            bufferReady.wait_for(locker, std::chrono::milliseconds(500));
            const bool frameReady = framesCompressed != framesConverted;

            if ( frameReady )
            {
                framesCompressed += 1;

                // Send the frame info:
                Serialise( *m_muxer, "AvInfo", m_timeBuffer[1], framesCompressed );

                // Wrap the conversion buffer in a video frame object (YUV420P is native for mpg4):
                VideoFrame frame( m_buffer[1], PIX_FMT_YUV420P, streamWidth, streamHeight, streamWidth );
                sentOk = streamer.PutVideoFrame( frame );
                sentOk &= !streamer.IoError();
            }
        }
    } // extra scope for lock guard

    // Stop the camera and free the conversion buffers:
    m_camera->StopCapture();
    free( m_buffer[0] );
    free( m_buffer[1] );
}
