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
    m_transport.SetBlocking( false );
    m_sendThread.Start();
    m_receiveThread.Start();
}

ComCentre::~ComCentre()
{
    m_transportError = true; // Causes threads to exit (@todo use better method)
    m_rxReady.WakeAll();
    m_txReady.WakeAll();
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
            m_txReady.Wait( m_txLock );
        }

        // Send in priority order:
        /// @todo if one queue is always full we could get starvation here...how to fix?
        /// (But in that case the system is overloaded anyway so what would we like to do when overloaded?)
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
        if ( ReceivePacket( packet ) )
        {
            GLK::MutexLock lock( m_rxLock );
            m_rxQueues[ packet.GetType() ].push( std::move(packet) );
            m_rxReady.WakeOne();
        }
    }
}

/**
    Used externally to post a packet to the comms system.
    The packet will be queued and sent later.

    @param packet An rvalue reference to a packet that will be posted on the queue.
    If it is not an rvalue reference then after calling this the packet object will become
    invalid ( i.e. have type ComPacket::Type::invalid and no data).
*/
void ComCentre::PostPacket( ComPacket&& packet )
{
    GLK::MutexLock lock( m_txLock );
    // Each packet type goes onto a separate queue:
    m_txQueues[ packet.GetType() ].push( std::move(packet) );
    m_numPosted += 1;
    m_txReady.WakeOne();
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
    while ( m_transportError == false && packets.empty() == false )
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
    assert( packet.GetType() != ComPacket::Type::Invalid ); // Catch attempts to send invalid packets

    // Write the type as an unsigned 32-bit integer in network byte order:
    size_t writeCount = sizeof(uint32_t);
    uint32_t type = htonl( static_cast<uint32_t>( packet.GetType() ) );
    bool ok = WriteBytes( reinterpret_cast<const uint8_t*>(&type), writeCount );


    // Write the data size:
    uint32_t size = htonl( packet.GetDataSize() );
    writeCount = sizeof(uint32_t);
    ok &= WriteBytes( reinterpret_cast<const uint8_t*>(&size), writeCount );

    // Write the byte data:
    writeCount = packet.GetDataSize();
    ok &= WriteBytes( reinterpret_cast<const uint8_t*>( packet.GetDataPtr() ), writeCount );

    m_transportError = !ok;
}

/**
    @param packet If return value is true then packet will contain the new data, if false packet remains unchanged.
    @return false on comms error, true if successful.
*/
bool ComCentre::ReceivePacket( ComPacket& packet )
{
    uint32_t type = 0;
    uint32_t size = 0;

    size_t byteCount = sizeof(uint32_t);
    bool ok = ReadBytes( reinterpret_cast<uint8_t*>(&type), byteCount );
    if ( !ok ) return false;

    byteCount = sizeof(uint32_t);
    ok = ReadBytes( reinterpret_cast<uint8_t*>(&size), byteCount );
    if ( !ok ) return false;

    type = ntohl( type );
    size = ntohl( size );

    ComPacket p( static_cast<ComPacket::Type>(type), size );
    byteCount = p.GetDataSize();
    ok = ReadBytes( reinterpret_cast<uint8_t*>(p.GetDataPtr()), byteCount );
    if ( !ok ) return false;

    std::swap( p, packet );
    assert( packet.GetType() != ComPacket::Type::Invalid ); // Catch invalid packets at the lowest level.
    return true;
}

/**
    Loop to guarantee the number of bytes requested are actually read.

    On error (return of false) size will contain the number of bytes that were remaining to be read.

    @return true if all bytes were written, false if there was an error at any point.
*/
bool ComCentre::ReadBytes( uint8_t* buffer, size_t& size )
{
    while ( size > 0 )
    {
        int n = m_transport.Read( reinterpret_cast<char*>( buffer ), size );
        if ( n < 0 || m_transportError )
        {
            return false;
        }

        size -= n;
        buffer += n;
    }

    return true;
}

/**
    Loop to guarantee the number of bytes requested are actually written.

    On error (return of false) size will contain the number of bytes that were remaining to be written.

    @return true if all bytes were written, false if there was an error at any point.
*/
bool ComCentre::WriteBytes( const uint8_t* buffer, size_t& size )
{
    while ( size > 0 )
    {
        int n = m_transport.Write( reinterpret_cast<const char*>( buffer ), size );
        if ( n < 0 || m_transportError )
        {
            return false;
        }

        size -= n;
        buffer += n;
    }

    return true;
}

