#include "../../include/RoboLib.h"
#include <glkcore.h>

#include <math.h>

#include "../opencv/opencv_utils.h"

const int IMG_WIDTH = 320;
const int IMG_HEIGHT = 240;

#include <arpa/inet.h>

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

    // Setup a server socket for receiving client commands:
    Socket s;
    s.Bind( atoi( argv[1] ) ); // Get port from command line
    s.Listen( 1 ); // Wait for connection
    Socket* con = s.Accept(); // Create connection
    con->SetBlocking( false );

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
        
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    int bytesToSend = 0;
    const char* pSend = 0;
    if ( con )
    {
        fprintf( stderr, "Client connected to robot.\n" );
        
        teljoy = new TeleJoystick( *con, drive ); // Will start receiving and processing remote joystick cammands immediately.
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
    Recieves images from robot.
**/
void runClient( int argc, char** argv )
{
    fprintf( stderr, "Client process here...\n" );
#ifndef ARM_BUILD
    ImageWindow display; // Used to display images received from robot.
#endif

    // Try to access the joystick:
    Joystick js( "/dev/input/js0" );

    if ( js.IsAvailable() )
    {
        js.Start();
    
        Socket client;
        client.SetNagleBufferingOff(); // The joystick commands a re small packets for which we want low latency, so turn off Nagle.
        
        if ( client.Connect( argv[1], atoi( argv[2] ) ) )
        {
            // Connected to robot
            int n = 0;
            while ( n >= 0 ) // Continue until we get a read error
            {
                // Read joystick and send
                int32_t jx = js.GetAxis(1); // left hat-stick on ps3 controller
                int32_t jy = js.GetAxis(2); // right hat-stick on ps3 controller
                int32_t max = 32767;
                int32_t data[3] = { htonl(jx), htonl(jy), htonl(max) };
                client.SetBlocking( true );
                client.Write( reinterpret_cast<char*>( data ), 3*sizeof(int) );
                client.SetBlocking( false );

                // Try to receive some image data.
                // NOTE: we receive the image in parts so the joystick remains responsive.
                static uint8_t image[IMG_WIDTH*IMG_HEIGHT];
                int nThisTime = IMG_WIDTH*IMG_HEIGHT - n;
                // Limit the amount of image we are willing to recieve in one go:
                if ( nThisTime > IMG_WIDTH*(IMG_HEIGHT/4) )
                {
                    nThisTime = IMG_WIDTH*(IMG_HEIGHT/4);
                }
                
                int bytesRead = client.Read( reinterpret_cast<char*>( image ) + n, nThisTime );
                if ( bytesRead > 0 )
                {
                    n += bytesRead;
                    if ( n == IMG_WIDTH*IMG_HEIGHT )
                    {
#ifndef ARM_BUILD
                        // Show the image by posting pointer to the ImageWindow object:
                        if ( display.IsRunning() )
                        {                           
                            display.PostImage( image );
                        }
#endif
                        n = 0;
                    }
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

