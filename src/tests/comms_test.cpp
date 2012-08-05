#include "../../include/RoboLib.h"
#include <glkcore.h>

#ifndef ARM_BUILD
#include <glk.h>
#endif

#include <math.h>
#include <time.h>

const int IMG_WIDTH = 320;
const int IMG_HEIGHT = 240;

#include <arpa/inet.h>

#include "RobotServer.h"

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

static void message( const char* msg )
{
    std::cerr  << msg << std::endl;
}

/**
    Server: receives commands from client, controls robot, and sends video stream to client.

    @note The server loops for ever - when one connection exits it will reset and wait for another one.
**/
int runServer( int argc, char** argv )
{
    message( "Server process here..." );

    RobotServer robot( atoi(argv[1]), "/dev/ttyUSB0" );

    while ( true )
    {
        robot.Listen(); // Wait for 1 new connection.
        robot.RunCommsLoop(); // Runs communications with new conection.
    }
}

/**
    Client: reads joystick commands and sends to sever. Recieves and displays video from robot.
**/
int runClient( int argc, char** argv )
{
    message( "Client process here..." );

    // Try to access the joystick:
    Joystick js( "/dev/input/js0" );
    if ( js.IsAvailable() )
    {
        js.Start();
    }

    TcpSocket client;
    client.SetNagleBufferingOff(); // The joystick commands are small packets for which we want low latency, so turn off Nagle.

    if ( client.Connect( argv[1], atoi( argv[2] ) ) )
    {
        // Create a video reader object that uses socket IO:
        FFMpegSocketIO videoIO( client, false );
        LibAvCapture streamer( videoIO );
        if ( streamer.IsOpen() == false )
        {
            message( "Could not create stream capture." );
            return EXIT_FAILURE;
        }

        // Get some frames so we can extract correct image dimensions:
        bool gotFrame = true;
        for ( int i=0;i<2;++i )
        {
            gotFrame = streamer.GetFrame();
            streamer.DoneFrame();
        }

        if ( gotFrame == false )
        {
            message( "Could not interpret video stream." );
            return EXIT_FAILURE;
        }

        int w = streamer.GetFrameWidth();
        int h = streamer.GetFrameHeight();
        std::cerr << "Received frame dimensions: " << w << "x" << h << std::endl;

        // Create a buffer for image data:
        uint8_t* imageBuffer;
        int err = posix_memalign( (void**)&imageBuffer, 16, w * h * 3 * sizeof(uint8_t) );
        assert( err == 0 );

#ifndef ARM_BUILD
        // Setup a display window:
        GLK::ImageWindow display(320,240); // Used to display images received from robot.
        GLK::ImageWindow::ImageData postData; // this struct describes the image data we post for display
        postData.mode = GLK::ImageWindow::FixedAspectRatio;
        postData.w = w;
        postData.h = h;
        postData.stride = w*3;
        postData.ptr = imageBuffer;
        postData.isColourBgr = true;
#endif

        int32_t joyData[3] = { 0,0,1 };
        int numFrames = 0;
        uint64_t videoBytes = 0;

        struct timespec t1;
        struct timespec t2;
        clock_gettime( CLOCK_MONOTONIC, &t1 );

#ifndef ARM_BUILD
        while ( display.IsRunning() && gotFrame )
#else
        while ( gotFrame )
#endif
        {
            if ( js.IsAvailable() )
            {
                // Read joystick and send
                joyData[0] = htonl( js.GetAxis(1) ); // left hat-stick on ps3 controller
                joyData[1] = htonl( js.GetAxis(2) ); // right hat-stick on ps3 controller
                joyData[2] = htonl( 32767 );
            }
            client.Write( reinterpret_cast<char*>( joyData ), 3*sizeof(int32_t) ); // have to write data anyway or server side will complain

            gotFrame = streamer.GetFrame();
            if ( gotFrame )
            {
                streamer.ExtractBgrImage( imageBuffer, w*3 );
                streamer.DoneFrame();

                numFrames += 1;

                if ( numFrames == 45 ) // Output rolling averages after certain number of frames
                {
                    clock_gettime( CLOCK_MONOTONIC, &t2 );

                    double secs = (milliseconds(t2) - milliseconds(t1))/1000.0;
                    uint64_t bytesRx = videoIO.BytesRead();
                    double bits_per_sec = ( bytesRx - videoBytes )*(8.0/secs);
                    videoBytes = bytesRx;
                    std::cerr << "Through-put: " << numFrames/secs << " fps @ " << bits_per_sec/(1024.0*1024.0) << "Mbps" << std::endl;
                    numFrames = 0;
                    clock_gettime( CLOCK_MONOTONIC, &t1 );
                }

#ifndef ARM_BUILD
                display.PostImage( postData );
#else
                std::cerr << "got frame " << std::endl; // on ARM build simply indicate that we have successfully got a frame
#endif
            }
            else
            {
                message( "Could not get frame." );
            }
        }

        client.Shutdown();
        free( imageBuffer );
        return EXIT_SUCCESS;
    }
    else
    {
        message( "Could not connect." );
    }

    return EXIT_FAILURE;
}

/**
    @note Number of command line arguments decides whether process will be server or client.
*/
int main( int argc, char** argv )
{   
    if ( argc == 2 )
    {
        // Only a port specified so this will be a server process:
        return runServer( argc, argv );
    }
    else if ( argc == 3 )
    {
        // Address and port specified so this will be a client process:
        return runClient( argc, argv );
    }
}

