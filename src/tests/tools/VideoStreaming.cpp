// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <RoboLib.h>

int runServer( int argc, char** argv )
{
    std::cerr << "Starting video-streaming server" << std::endl;
    TcpSocket server;

    return EXIT_SUCCESS;
}

int runClient( int argc, char** argv )
{
    std::cerr << "Starting video-streaming client" << std::endl;
    TcpSocket client;

    return EXIT_SUCCESS;
}

/*
    Server process captures live video and streams it over a socket.

    The client process displays the video.
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
        // IP and port specified so process should behave as client:
        return runClient( argc, argv );
    }

    std::cerr << "Usage: " << argv[0] << " ADDRESS [PORT]" << std::endl;
    return EXIT_FAILURE;
}

