#include "../../include/RoboLib.h"
#include <glkcore.h>

#include <math.h>

#include "../opencv/opencv_utils.h"

const int IMG_WIDTH = 320;
const int IMG_HEIGHT = 240;

/**
    Program behaves as server:
    Controls robot and receives commands from cient.
**/
void runServer( int argc, char** argv )
{
    fprintf( stderr, "server process here...\n" );

    // Setup comms to motors:
    DiffDrive*  drive;
    MotionMind* motors = new MotionMind( "/dev/ttyUSB0" );
    
    if ( motors->Available() )
    {
        drive = new DiffDrive( *motors );
        float amps = 1.5f;
        int32_t currentLimit = roundf( amps/0.02f );
        int32_t pwmLimit = (72*1024)/120; // motor voltage / battery voltage
        
        motors->WriteRegister( 1, MotionMind::AMPSLIMIT, currentLimit );
        motors->WriteRegister( 2, MotionMind::AMPSLIMIT, currentLimit );
        motors->WriteRegister( 1, MotionMind::PWMLIMIT, pwmLimit );
        motors->WriteRegister( 2, MotionMind::PWMLIMIT, pwmLimit );
    }
    else
    {
        delete motors;
        drive  = 0;
        motors = 0;
    }

    // Setup camera:
    UnicapCamera* camera = new UnicapCamera();
    uint8_t* m_lum;
    size_t imageBufferSize = camera->GetFrameWidth() * camera->GetFrameHeight() * sizeof(uint8_t);
    if ( camera->IsAvailable() )
    {
        camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        delete camera;
        camera = 0;
    }

    // Setup a server socket for receiving client commands:
    Socket s;
    s.Bind( atoi( argv[1] ) ); // Get port from command line
    s.Listen( 1 ); // Wait for connection
    Socket* con = s.Accept(); // Create connection
    con->SetBlocking( false );
        
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    int bytesToSend = 0;
    const char* pSend = 0;
    if ( con && drive )
    {
        fprintf( stderr, "Client connected to robot.\n" );
        
        teljoy = new TeleJoystick( *con, *drive ); // Will start receiving and processing remote joystick cammands immediately.
        GLK::Thread::Sleep( 100 );
        fprintf( stderr, "running: %d\n", teljoy->IsRunning() );
            
        // Capture images continuously:
        while ( teljoy->IsRunning() )
        {       
            if ( camera && (bytesToSend == 0) )
            {
                camera->GetFrame();
                camera->ExtractLuminanceImage( m_lum );
                camera->DoneFrame();
                
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
                int bytesWritten = con->Write( pSend, bytesToSend );
                bytesToSend -= bytesWritten;
                pSend += bytesWritten;
            }
            
            GLK::Thread::Sleep( 10 );
        } // end while
        
        fprintf( stderr, "Control terminated\n" );
        
    } // end if

    delete teljoy;

    if ( camera )
    {
        camera->StopCapture();
        delete camera;
        free( m_lum );
    }

    delete motors;
    delete drive;
    delete con; 
}

/**
    Program behaves as client: reads joystick commands and sends to sever.
**/
void runClient( int argc, char** argv )
{
        fprintf( stderr, "Client process here...\n" );
#ifndef ARM_BUILD
        ImageWindow display;
#endif        
        Joystick js( "/dev/input/js0" );

        if ( js.IsAvailable() )
        {
            js.Start();
    
            Socket client;
            client.SetNagleBufferingOff();
            if ( client.Connect( argv[1], atoi( argv[2] ) ) )
            {                            
                int n = 0;
                while ( n >= 0 )
                {
                    int jx = js.GetAxis(1);
                    int jy = js.GetAxis(2);
                    int max = 32767;
                    int data[3] = { jx, jy, max };
                    client.SetBlocking( true );
                    client.Write( reinterpret_cast<char*>( data ), 3*sizeof(int) );
                    client.SetBlocking( false );
                                    
                    static uint8_t image[IMG_WIDTH*IMG_HEIGHT];
                    int nThisTime = IMG_WIDTH*IMG_HEIGHT - n;
                    if ( nThisTime > IMG_WIDTH*(IMG_HEIGHT/4) ) { nThisTime = IMG_WIDTH*(IMG_HEIGHT/4); }
                    n += client.Read( reinterpret_cast<char*>( image ) + n, nThisTime );

                    if ( n == IMG_WIDTH*IMG_HEIGHT )
                    {
#ifndef ARM_BUILD
                        if ( display.IsRunning() )
                        {                           
                            display.PostImage( image );
                        }
#endif
                        n = 0;
                    }
                    GLK::Thread::Sleep( 50 );
                }
            }
        }
        else
        {
            fprintf( stderr, "Stopped: No joystick.\n" );
        }
}

int main( int argc, char** argv )
{   
    if ( argc == 2 )
    {
        // Only a port specified so this will be a server process:
        runServer( argc, argv );
    }
    else if ( argc == 3 )
    {
        // IP and port specified so process should behave as client:
        runClient( argc, argv );
    }
}

