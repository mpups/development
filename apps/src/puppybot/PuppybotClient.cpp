#include "PuppybotClient.h"

#include <iostream>

PuppybotClient::PuppybotClient()
:
    m_server     (0),
    m_udpChannel (0)
{
    
}

PuppybotClient::~PuppybotClient()
{
    delete m_udpChannel;
    delete m_server;
}

bool PuppybotClient::Connect( const Ipv4Address& serverAddress )
{
    delete m_server;
    m_server = new TcpSocket();

    if ( m_server->Connect( serverAddress ) )
    {
        m_server->GetPeerAddress( m_addr );
        PostConnectionSetup();

        return true;
    }
    else
    {
        std::cerr << "Could not connect to PuppyBot." << std::endl;
    }

    return false;
}

const Ipv4Address& PuppybotClient::GetServerAddress()
{
    return m_addr;
}

void PuppybotClient::RunCommsLoop()
{
}

void PuppybotClient::PostConnectionSetup()
{
    char msg[8] = "";
    int n = m_server->Read( msg, 7 );
    msg[n] = '\0';
    std::cerr << "Read msg: " << msg << std::endl;
    m_server->Write( "thanks", 6 );

    std::string serverName;
    m_addr.GetHostName( serverName );

    // Create a secondary UDP connection to PuppyBot (this will be used to send control packets):
    delete m_udpChannel;
    m_udpChannel = new UdpSocket();
    m_udpChannel->Connect( m_addr );
    std::cerr << "Sending UDP messages on port " << m_addr.GetPort() << std::endl;
    int bytes = m_udpChannel->Write( "1234", 4 );
    std::cerr << "Sent " << bytes << " bytes" << std::endl;
}

