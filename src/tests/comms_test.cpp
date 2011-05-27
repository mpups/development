#include "../../include/RoboLib.h"
#include <glkcore.h>

#include <math.h>

const int IMG_WIDTH = 320;
const int IMG_HEIGHT = 240;

#include <arpa/inet.h>

#include "RobotServer.h"

/**
    Program behaves as server:
    Controls robot and receives commands from cient.
**/
void runServer( int argc, char** argv )
{
    fprintf( stderr, "server process here...\n" );

    RobotServer robot( argv[1], "/dev/ttyUSB0" );

    while ( true )
    {
        robot.Listen(); // Wait for 1 new connection.
        robot.RunCommsLoop(); // Runs communications with new conection.
    }
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
                            display.PostImage( image, IMG_WIDTH, IMG_HEIGHT, ImageWindow::FixedAspectRatio );
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

