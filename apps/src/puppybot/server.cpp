#include <RoboLib.h>

#include "PuppybotServer.h"

#include <iostream>

/*
    Server listens for connections on port supplied to command line.
*/
int main( int argc, char** argv )
{
    if ( argc != 2 )
    {
        std::cerr << "Usage: " << argv[0] << " PORT" << std::endl;
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    PuppybotServer robot( port );

    std::string clientName;

    while ( true )
    {
        std::cerr << "Waiting for connection on port " << port << "..." <<  std::endl;

        robot.Listen();       // Wait for a single new connection.

        robot.GetClientAddress().GetHostName( clientName );
        std::cerr << "Client " << clientName << " connected to PuppyBot." << std::endl;

        robot.RunCommsLoop(); // Run communications until connection terminates.
    }

    return EXIT_SUCCESS;
}

