#include "PacketMuxer.h"

#include <arpa/inet.h>

#include <iostream>
#include <algorithm>
#include <type_traits>

/**
    Create a new muxer that will send packets over the specified
    socket.

    This object is guaranteed to only ever write to the socket.
*/
PacketMuxer::PacketMuxer( AbstractSocket& socket )
:
    m_txLock        ( GLK::Mutex::Recursive ), // Had to make this recursive so we can emplace control packets to the queue internally while we already hold the tx lock.
    m_numPosted     (0),
    m_numSent       (0),
    m_transport     ( socket ),
    m_transportError( false ),
    m_sender        ( std::bind(&PacketMuxer::SendLoop, std::ref(*this)) )
{
    m_transport.SetBlocking( false );
}

PacketMuxer::~PacketMuxer()
{
    {
        GLK::MutexLock lock( m_txLock );
        m_transportError = true; /// Causes threads to exit (@todo use better method)
        m_txReady.WakeAll();
    }

    {
        int c = 0;
        SimpleAsyncFunction( [&](){
            // do stuff
            c += 10;
        });
    }
}

/**
    Return false if ther ehave been any communication errors, true otherwise.
*/
bool PacketMuxer::Ok() const
{
    return m_transportError == false;
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
    This function loops sending all the queued packets over the
    transport layer. The loop exits if there is a transport error
    (e.g. if the other end hangs up).

    Runs asnchronously in its own thread :- it is passed
    using std::bind to a SimpleAsyncFunction object.
*/
void PacketMuxer::SendLoop()
{
    std::cerr << "PacketMuxer::SendLoop() entered." << std::endl;

    SendControlMessage( ControlMessage::Hello );

    // Grab the lock for the transmit/send queues:
    GLK::MutexLock lock( m_txLock );

    while ( m_transportError == false )
    {
        if ( m_numPosted == m_numSent )
        {
            // Atomically relinquish lock for send queues and wait
            // until new data is posted (don't care to which queue it
            // is posted, hence one condition variable for all queues):
            bool ready = m_txReady.TimedWait( m_txLock, 1000 );
            if ( ready == false )
            {
                // If there are no packets to send after waiting for 1 second then
                // send a 'HeartBeat' message - this serves two purposes:
                // 1. Lets the other side know we are still connected.
                // 2. Lets this side detect if the other side has hung up or crashed (Sending the packet will fail at the socket level).
                // These are similar to TCP keep-alive messages, but there is no defined standard about how to use TCP keepalive to
                // achieve the same behaviour:
                SendControlMessage( ControlMessage::HeartBeat );
            }
        }

        // Send in priority order:
        /// @todo if one queue is always full we could get starvation here...how to fix? Counter for each queue
        /// indicating how many send loops it has been not empty?
        /// (But in that case the system is overloaded anyway so what would we like to do when overloaded?)
        for ( auto& pair : m_txQueues )
        {
            SendAll( pair.second );
        }
    }

    std::cerr << "PacketMuxer::Send() exited." << std::endl;
}

/**
    Send all the packets from the FIFO container. The container will
    be empty after calling this function, regardless of whether there
    were any errors from the transport layer.

    @note Assumes you have acquired the appropriate
    lock to access the specified PacketContainer.

     @bug Bug on shutdown - the queues are always empty so we never attempt to send any packets
     and therefore never get to state where m_transportError is true, hence the caller loops forever.
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

    Header is:
    type (4-bytes)
    data-size (4-bytes)

    followed by the data payload.
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

void PacketMuxer::SendControlMessage( ControlMessage msg )
{
    /// @todo should use this once g++ is updated: std::underlying_type(ControlMessage)
    EmplacePacket( ComPacket::Type::Control, reinterpret_cast<uint8_t*>(&msg), sizeof(uint8_t) );
}
