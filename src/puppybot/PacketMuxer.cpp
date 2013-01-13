#include "PacketMuxer.h"

#include <arpa/inet.h>

#include <iostream>
#include <algorithm>

/**
    Create a new muxer that will send packets over the specified
    socket.

    This object is guaranteed to only ever write to the socket.
*/
PacketMuxer::PacketMuxer( Socket& socket )
:
    m_sender        ( std::bind(&PacketMuxer::Send, std::ref(*this)) ),
    m_sendThread    ( m_sender ),
    m_numPosted     (0),
    m_numSent       (0),
    m_transport     ( socket ),
    m_transportError( false )
{
    m_transport.SetBlocking( false );
    m_sendThread.Start();
}

PacketMuxer::~PacketMuxer()
{
    {
        GLK::MutexLock lock( m_txLock );
        m_transportError = true; /// Causes threads to exit (@todo use better method)
        m_txReady.WakeAll();
    }

    m_sendThread.Join();
}

/**
    Return false if ther ehave been any communication errors, true otherwise.
*/
bool PacketMuxer::Ok() const
{
    return m_transportError == false;
}

void PacketMuxer::Send()
{
    //std::cerr << "Send thread started." << std::endl;
    GLK::MutexLock lock( m_txLock );

    while ( m_transportError == false )
    {
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

/**
    Used externally to post a packet to the comms system.
    The packet will be queued and sent later.

    @param packet An rvalue reference to a packet that will be posted on the queue.
    If it is not an rvalue reference then after calling this the packet object will become
    invalid ( i.e. have type ComPacket::Type::invalid and no data).
*/
void PacketMuxer::PostPacket( ComPacket&& packet )
{
    GLK::MutexLock lock( m_txLock );
    // Each packet type goes onto a separate queue:
    ComPacket::Type packetType = packet.GetType(); // Need to cache this before we use std::move
    ComPacket::SharedPacket sptr = std::make_shared<ComPacket>( std::move(packet) );
    m_txQueues[ packetType ].push( std::move(sptr) );
    SignalPacketPosted();
}

/**
    Optimised version of ComCentre::PostPacket() which uses forwarding to efficiently
    construct the packet in-place (no std::move required as in PostPacket).

    @param args @todo Variadic argument list to forward to the ComPacket constructor.

    @note There is a g++ bug which doesn't allow perfect forwarding in cases like this:
    when it is fixed variadic arguments can be forwarded directly to any ComPacket constructor.
*/
void PacketMuxer::EmplacePacket( ComPacket::Type type, uint8_t* buffer, int size )
{
    GLK::MutexLock lock( m_txLock );
    m_txQueues[ type ].emplace( std::make_shared<ComPacket>(type, buffer, size) );
    SignalPacketPosted();
}

/**
    Send all the packets from the FIFO container. The container will
    be empty after calling this function, regardless of whether there
    were any errors from the transport layer.

    @note Assumes you have acquired the appropriate
    lock to access the specified PacketContainer.
*/
void PacketMuxer::SendAll( ComPacket::PacketContainer& packets )
{
    while ( m_transportError == false && packets.empty() == false )
    {
        // Here we must send before popping to guarantee the shared_ptr is valid for the lifetime of SendPacket.
        SendPacket( *(packets.front().get()) );
        packets.pop();
        m_numSent += 1;
    }
}

/**
    Used internally to send a packet over the transport layer.
*/
void PacketMuxer::SendPacket( const ComPacket& packet )
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
    Loop to guarantee the number of bytes requested are actually written.

    On error (return of false) size will contain the number of bytes that were remaining to be written.

    @return true if all bytes were written, false if there was an error at any point.
*/
bool PacketMuxer::WriteBytes( const uint8_t* buffer, size_t& size )
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

void PacketMuxer::SignalPacketPosted()
{
    m_numPosted += 1;
    m_txReady.WakeOne();
}
