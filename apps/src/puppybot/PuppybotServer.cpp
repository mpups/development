#include "PuppybotServer.h"

PuppybotServer::PuppybotServer ( uint16_t port )
:
    m_udpChannel (0),
    m_port (port)
{
    m_server = new TcpSocket();
    m_server->Bind( m_port );
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
    // test tcp:
    m_con->Write( "welcome", 7 );
    char msg[8] = "";
    int n = m_con->Read( msg, 6 );
    msg[n] = '\0';
    std::cerr << "Read msg: " << msg << std::endl;

    // Create a connected udp channel for low latency packets:
    delete m_udpChannel;
    m_udpChannel = new UdpSocket();

    // For UDP we can use the same port as TCP.
    if ( m_udpChannel->Bind( m_port ) )
    {
        char msg[] = "########";
        std::cerr << "Waiting to receive UDP message on port " << m_port << std::endl;
        int bytes = m_udpChannel->Read( msg, 6 );
        msg[bytes] = '\0';
        std::cerr << "Received a " << bytes << " byte msg on UDP channel: " << msg << std::endl;
    }
    else
    {
        std::cerr << "Error: Could not open UDP channel." << std::endl;
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

