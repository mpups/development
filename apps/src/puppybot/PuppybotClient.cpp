#include "PuppybotClient.h"

#include <RoboLib.h>

#include <iostream>
using namespace std;

PuppybotClient::PuppybotClient()
:
    m_server ( 0 ),
    m_addr   ( new Ipv4Address() )
{
    
}

PuppybotClient::~PuppybotClient()
{
    delete m_server;
    delete m_addr;
}

bool PuppybotClient::Connect( const Ipv4Address& serverAddress )
{
    delete m_server;
    m_server = new TcpSocket();

    if ( m_server->Connect( serverAddress ) )
    {
        m_server->GetPeerAddress( *m_addr );
        string serverName;

        m_addr->GetHostName( serverName );
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

        return true;
    }
    else
    {
        cerr << "Could not connect to PuppyBot." << endl;
    }

    return false;
}

void PuppybotClient::RunCommsLoop()
{
    
}

