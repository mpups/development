#include "../../include/RoboLib.h"
#include <glkcore.h>

#include <math.h>
#include <time.h>

const int IMG_WIDTH = 320;
const int IMG_HEIGHT = 240;

#include <arpa/inet.h>

#include "RobotServer.h"
#include "RobotClient.h"

static void message( const char* msg )
{
    std::cerr << msg << std::endl;
}

/**
    Server: receives commands from client, controls robot, and sends video stream to client.

    @note The server loops for ever - when one connection exits it will reset and wait for another one.
**/
int runServer( int argc, char** argv )
{
    message( "Server process here..." );

    RobotServer robot( atoi(argv[1]), "/dev/ttyUSB0" );

    while ( robot.Listen() ) // Wait for 1 new connection (blocking).
    {
        robot.RunCommsLoop(); // Runs communications with new conection.
    }

    message( "Server exiting: Listen() returned false." );

    return EXIT_SUCCESS;
}

/**
    Client: reads joystick commands and sends to sever. Recieves and displays video from robot.
**/
int runClient( int argc, char** argv )
{
    message( "Client process here..." );

    RobotClient robotClient;
    if ( robotClient.Connect( argv[1], atoi(argv[2]) ) )
    {
        bool success = robotClient.RunCommsLoop();
        if ( success )
        {
            return EXIT_SUCCESS;
        }
        else
        {
            return EXIT_FAILURE;
        }
    }
    else
    {
        message( "Could not connect." );
        return EXIT_FAILURE;
    }
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

