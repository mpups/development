#include <RoboLib.h>

#include "PuppybotClient.h"

#include <iostream>

/*
    Client connects to server host and port defined on command line.
*/
int main( int argc, char** argv )
{
    if ( argc != 3 )
    {
        std::cerr << "Usage: " << argv[0] << " HOST PORT" << std::endl;
        return EXIT_FAILURE;
    }

    Ipv4Address serverAddress( argv[1], atoi( argv[2] ) );
    PuppybotClient client;
    client.Connect( serverAddress );

    // Spawn a communiations thread:

    return EXIT_SUCCESS;
}

