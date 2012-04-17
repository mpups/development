#include "RobotServer.h"

const int IMG_WIDTH  = 320;
const int IMG_HEIGHT = 240;

/**
    Setup a robot server with specified TCP and serial ports.
    
    TCP is used for a remote tele-link and the serial port is used for differential drive control (if available).
**/
RobotServer::RobotServer( const char* tcpPort, const char* motorSerialPort )
:
    m_serialPort( motorSerialPort ),
    m_drive ( 0 ),
    m_motors( 0 ),
    m_server( 0 ),
    m_con   ( 0 ),
    m_camera( 0 )
{   
    // Setup a server socket for receiving client commands:
    m_server = new TcpSocket();
    m_server->Bind( atoi( tcpPort ) ); // Get port from command line
}

RobotServer::~RobotServer()
{
    delete m_camera;
    delete m_con;
    delete m_server;
    delete m_server;
    delete m_drive;
    delete m_motors;
}

/**
    Blocks until robot gets a connection.
**/
void RobotServer::Listen()
{
    fprintf( stderr, "Waiting for new connection...\n" );
    m_server->Listen( 0 ); // Wait for connection - no queue
    m_con = m_server->Accept(); // Create connection
    m_con->SetBlocking( false );
    
    PostConnectionSetup();
}

/**
    Perform post conection processing.
    
    Attempts to access camera and wheels.
**/
void RobotServer::PostConnectionSetup()
{
    // Setup comms to motors:
    m_motors = new MotionMind( m_serialPort.cStr() );
    if ( m_motors->Available() )
    {
        m_drive = new DiffDrive( *m_motors );
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
        delete m_motors;
        m_motors = 0;
    }

    // Setup camera:
    m_camera = new UnicapCamera();
    size_t imageBufferSize = m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t);
    if ( m_camera->IsOpen() )
    {
        m_camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        delete m_camera;
        m_camera = 0;
    }
}

/**
    Cleans up resources once the comms loop has finished.
**/
void RobotServer::PostCommsCleanup()
{
    // These must be deleted in this order:
    delete m_drive;   
    delete m_motors;
    m_drive = 0;
    m_motors = 0;
    
    if ( m_camera )
    {
        m_camera->StopCapture();
        delete m_camera;
        free( m_lum );
    }
}

/**
    Runs the robot's comms loop until the connection terminates or fails.
**/
void RobotServer::RunCommsLoop()
{
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    int bytesToSend = 0;
    const char* pSend = 0;
    if ( m_con )
    {
        Ipv4Address clientAddress;
        m_con->GetPeerAddress( clientAddress );

        {
            std::string name;
            clientAddress.GetHostName( name );
            fprintf( stderr, "Client %s connected to robot.\n", name.c_str() );
        }

        teljoy = new TeleJoystick( *m_con, m_drive ); // Will start receiving and processing remote joystick cammands immediately.
        GLK::Thread::Sleep( 100 );
        fprintf( stderr, "running: %d\n", teljoy->IsRunning() );

        // Capture images continuously:
        while ( bytesToSend >= 0 && teljoy->IsRunning() )
        {
            if ( m_camera && (bytesToSend == 0) )
            {
                m_camera->GetFrame();
                m_camera->ExtractLuminanceImage( m_lum, m_camera->GetFrameWidth() );
                m_camera->DoneFrame();
                
                IplImage* cvImage = cvCreateImage( cvSize(640,480), IPL_DEPTH_8U, 1 );
                IplImage* cvSmaller = cvCreateImage( cvSize( IMG_WIDTH, IMG_HEIGHT ), IPL_DEPTH_8U, 1 );
                FillIplImage( m_lum, cvImage );
                cvResize( cvImage, cvSmaller );
                SpillIplImage( cvSmaller, m_lum );
                cvReleaseImage( &cvImage );
                cvReleaseImage( &cvSmaller );
                
                bytesToSend =  IMG_WIDTH * IMG_HEIGHT * sizeof(uint8_t);
                pSend = reinterpret_cast<char*>( m_lum );
            }
        
            if ( bytesToSend > 0 )
            {
                int bytesWritten = m_con->Write( pSend, bytesToSend );
                if ( bytesWritten >= 0  )
                {
                    bytesToSend -= bytesWritten;
                    pSend += bytesWritten;
                }
                else
                {
                    fprintf( stderr, "Error writing to socket\n" );
                    bytesToSend = -1;
                }

                GLK::Thread::Sleep( 5 );
            }
            
        } // end while
        
        fprintf( stderr, "Control terminated\n" );
        
    } // end if

    delete teljoy;
    
    PostCommsCleanup();
}

