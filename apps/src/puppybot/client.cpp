#include <RoboLib.h>

#include <iostream>

using namespace std;

/*
    Client connects to server host and port defined on command line.
*/
int main( int argc, char** argv )
{

    if ( argc != 3 )
    {
        cerr << "Usage: " << argv[0] << " HOST PORT" << endl;
        return EXIT_FAILURE;
    }

    // Connect to PuppyBot using TCP (PuppyBot is the server):
    TcpSocket server;
    Ipv4Address serverAddress( argv[1], atoi( argv[2] ) );
    if ( server.Connect( serverAddress ) )
    {
        server.GetPeerAddress( serverAddress );
        string serverName;

        serverAddress.GetHostName( serverName );
        cerr << "Connected to PuppyBot '" << serverName << "'" << endl;

        // Create a secondary UDP connection to PuppyBot (this will be used to send control packets):
        int controlPort = 3000; // @todo - this port should be read from the server configuration packet received over the first TCP connection:
        UdpSocket udpControl;
        if ( udpControl.Connect( serverName.c_str(), controlPort ) )
        {
            cerr << "Setup control channel to PuppyBot " << serverName << ":" << controlPort << endl;
            udpControl.Write( "123", 3 );
            udpControl.Shutdown();
        }

    }
    else
    {
        cerr << "Could not connect to PuppyBot." << endl;
    }

    return EXIT_SUCCESS;
}
