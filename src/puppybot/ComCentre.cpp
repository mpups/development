#include "ComCentre.h"

#include <arpa/inet.h>

#include <iostream>

ComCentre::ComCentre( Socket& socket )
:
    m_sender        ( std::bind(&ComCentre::Send, std::ref(*this)) ),
    m_receiver      ( std::bind(&ComCentre::Receive, std::ref(*this)) ),
    m_sendThread    ( m_sender ),
    m_receiveThread ( m_receiver ),
    m_numPosted     (0),
    m_numSent       (0),
    m_transport     ( socket ),
    m_transportError( false )
{
    m_transport.SetBlocking( true );
    m_sendThread.Start();
    m_receiveThread.Start();
}

ComCentre::~ComCentre()
{
    m_sendThread.Join();
    m_receiveThread.Join();
}

/**
    Return false if ther ehave been any communication errors, true otherwise.
*/
bool ComCentre::Ok() const
{
    return m_transportError == false;
}

void ComCentre::Send()
{
    //std::cerr << "Send thread started." << std::endl;

    while ( m_transportError == false )
    {
        GLK::MutexLock lock( m_txLock );
        if ( m_numPosted == m_numSent )
        {
            // Wait until new data is posted (don't care to which queue it is
            // posted, hence one condition variable for all queues):
            std::cerr << "Wating for packet post..." << std::endl;
            m_txReady.Wait( m_txLock );
        }

        std::cerr << "Sending packets..." << std::endl;

        // Send in priority order:
        SendAll( m_txQueues[ ComPacket::Type::AvInfo ] );
        SendAll( m_txQueues[ ComPacket::Type::AvData ] );
        SendAll( m_txQueues[ ComPacket::Type::Odometry ] );
    }
}

void ComCentre::Receive()
{
    //std::cerr << "Receive thread started." << std::endl;
    ComPacket packet;
    while ( m_transportError == false )
    {
        ReceivePacket( packet );
        m_rxQueues[ packet.GetType() ].push( std::move(packet) );
    }
}

/**
    Used externally to post a packet to the comms system.
    The packet will be queued and sent later.

    @param packet An rvalue reference to a packet that will be posted on the queue.
    If it is not an rvalue reference then after calling this the packet object will become
    an invalid packet.
*/
void ComCentre::PostPacket( ComPacket&& packet )
{
    GLK::MutexLock lock( m_txLock );
    // Each packet type goes onto a separate queue:
    m_txQueues[ packet.GetType() ].push( std::move(packet) ); /// @todo - std::move() on the packet to avoid copy?
    m_numPosted += 1;
    std::cerr << "Packet posted." << std::endl;
    m_txReady.WakeAll();
}

/**
    Send all the packets from the FIFO container. The container will
    be empty after calling this function, regardless of whether there
    were any errors from the transport layer.

    @note Assumes you have acquired the appropriate
    lock to access the specified PacketContainer.
*/
void ComCentre::SendAll( PacketContainer& packets )
{
    while ( packets.empty() == false )
    {
        SendPacket( packets.front() );
        packets.pop();
        m_numSent += 1;
    }
}

/**
    Used internally to send a packet over the transport layer.
*/
void ComCentre::SendPacket( const ComPacket& packet )
{
    // Write the type as an unsigned 32-bit integer in network byte order:
    uint32_t type = htonl( static_cast<uint32_t>( packet.GetType() ) );
    m_transport.Write( reinterpret_cast<const char*>(&type), sizeof(uint32_t) );

    // Write the data size:
    uint32_t size = htonl( packet.GetDataSize() );
    m_transport.Write( reinterpret_cast<const char*>(&size), sizeof(uint32_t) );

    // Write the byte data:
    int sent = m_transport.Write( reinterpret_cast<const char*>( packet.GetDataPtr() ), packet.GetDataSize() );
    if ( sent == -1 )
    {
        m_transportError = true;
    }
    std::cerr << "Packet sent. (" << sent << ")" << std::endl;
}

void ComCentre::ReceivePacket( ComPacket& packet )
{
    uint32_t type = 0;
    uint32_t size = 0;
    m_transport.Read( reinterpret_cast<char*>(&type), sizeof(uint32_t) );
    m_transport.Read( reinterpret_cast<char*>(&size), sizeof(uint32_t) );
    type = ntohl( type );
    size = ntohl( size );

    ComPacket p( static_cast<ComPacket::Type>(type), size );
    m_transport.Read( reinterpret_cast<char*>(p.GetDataPtr()), p.GetDataSize() );
    std::swap( p, packet );
}

