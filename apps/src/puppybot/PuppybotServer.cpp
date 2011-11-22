#include "PuppybotServer.h"

PuppybotServer::PuppybotServer ( int port )
{
    m_server = new TcpSocket();
    m_server->Bind( port );
}

PuppybotServer::~PuppybotServer ()
{
    delete m_con;
    delete m_server;
}

void PuppybotServer::Listen()
{
    m_server->Listen( 0 );      // Wait for connection - no queue
    m_con = m_server->Accept(); // Create connection
    
    if ( m_con )
    {
        m_con->SetBlocking( false );
        m_con->GetPeerAddress( m_clientAddress );
    }
}

const Ipv4Address& PuppybotServer::GetClientAddress()
{
    return m_clientAddress;
}

void PuppybotServer::RunCommsLoop()
{
    
}

