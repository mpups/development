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
        int port = m_addr->GetPort();
        cerr << "Connected to PuppyBot '" << serverName << "' on port " << port << endl;

        // Create a secondary UDP connection to PuppyBot (this will be used to send control packets):
        GLK::Thread::Sleep(1000);
        std::cerr << "Sending UDP message..." << std::endl;
        UdpSocket udpControl;
        udpControl.SendTo( *m_addr, "1234", 4 );

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

