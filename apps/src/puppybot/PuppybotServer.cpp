#include "PuppybotServer.h"

PuppybotServer::PuppybotServer ( int port )
:
    m_udpChannel (0)
{
    m_server = new TcpSocket();
    m_server->Bind( port );
}

PuppybotServer::~PuppybotServer ()
{
    delete m_con;
    delete m_server;
    delete m_udpChannel;
}

void PuppybotServer::Listen()
{
    m_server->Listen( 0 );      // Wait for connection - no queue
    m_con = m_server->Accept(); // Create connection
    
    if ( m_con )
    {
        m_con->SetBlocking( false );
        m_con->GetPeerAddress( m_clientAddress );
        PostConnectionSetup();
    }
}

const Ipv4Address& PuppybotServer::GetClientAddress()
{
    return m_clientAddress;
}

void PuppybotServer::PostConnectionSetup()
{
    // Create a connected udp channel for low latency packets:
    delete m_udpChannel;
    m_udpChannel = new UdpSocket();

    // For UDP we can use the same port as TCP.
    if ( m_udpChannel->Bind( m_clientAddress.GetPort() ) )
    {
        char msg[] = "####";
        std::cerr << "Waiting to receive UDP message..." << std::endl;
        int bytes = m_udpChannel->Read( msg, 4 );
        std::cerr << "Received a " << bytes << "byte msg on UDP channel: " << msg << std::endl;
    }
    else
    {
        std::cerr << "Could not open UDP channel on port " << m_clientAddress.GetPort() << std::endl;
    }

    SendHandshakePacket();
}

/**
    
*/
void PuppybotServer::SendHandshakePacket()
{
}

/**
    
*/
void PuppybotServer::RunCommsLoop()
{
    
}

