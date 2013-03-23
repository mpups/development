#include "PacketDemuxer.h"

#include <iostream>
#include <algorithm>

/**
    Create a new demuxer that will receive packets from the specified socket.

    This object is guaranteed to only ever read from the socket.
*/
PacketDemuxer::PacketDemuxer( Socket& socket )
:
    m_receiver      ( std::bind(&PacketDemuxer::Receive, std::ref(*this)) ),
    m_receiveThread ( m_receiver ),
    m_nextSubscriberId (0),
    m_transport     ( socket ),
    m_transportError( false )
{
    m_transport.SetBlocking( false );
    m_receiveThread.Start();
}

PacketDemuxer::~PacketDemuxer()
{
    {
        m_transportError = true; /// Causes receive-thread to exit (@todo use better method)
    }

    m_receiveThread.Join();
}

/**
    Return false if ther ehave been any communication errors, true otherwise.
*/
bool PacketDemuxer::Ok() const
{
    return m_transportError == false;
}


/**
    Returns a subscriber object.
*/
PacketSubscription PacketDemuxer::Subscribe( ComPacket::Type type, PacketSubscriber::CallBack callback )
{
    SubscriptionEntry::second_type& queue = m_subscribers[ type ];
    queue.emplace_back( new PacketSubscriber( type, *this , callback ) );
    m_nextSubscriberId += 1;
    return PacketSubscription( queue.back() );
}

void PacketDemuxer::Unsubscribe( const PacketSubscriber* pSubscriber )
{
    ComPacket::Type type = pSubscriber->GetType();
    SubscriptionEntry::second_type& queue = m_subscribers[ type ];

    // Search through all subscribers of this type for the specific subscriber:
    auto itr = std::remove_if( queue.begin(), queue.end(), [pSubscriber]( const PacketDemuxer::SubscriberPtr& subscriber ) {
        return subscriber.get() == pSubscriber;
    });

    assert( itr != queue.end() );
    queue.erase( itr );
}

/**
    @return true if the specified subscriber is subscribed to this demuxer.
*/
bool PacketDemuxer::IsSubscribed( const PacketSubscriber* pSubscriber ) const
{
    ComPacket::Type type = pSubscriber->GetType();
    auto queueItr = m_subscribers.find( type );

    if ( queueItr == m_subscribers.end() )
    {
        return false;
    }

    const SubscriptionEntry::second_type& queue = queueItr->second;

    // Search through all subscribers of this type for the specific subscriber:
    auto itr = std::find_if( queue.begin(), queue.end(), [pSubscriber]( const PacketDemuxer::SubscriberPtr& subscriber ) {
        return subscriber.get() == pSubscriber;
    });

    return itr != queue.end();
}

/**
    This function loops receiving data from the transport layer, splitting
    it into packets. The loop exits if there is a transport error
    (e.g. if the other end hangs up).

    Runs asnchronously in its own thread (it is passed as a RunnableFunction
    to m_receiveThread in the PacketDemuxer constructor).
*/
void PacketDemuxer::Receive()
{
    std::cerr << "PacketDemuxer::Receive() entered." << std::endl;
    ComPacket packet;
    while ( m_transportError == false )
    {
        if ( ReceivePacket( packet ) )
        {
            ComPacket::Type packetType = packet.GetType(); // Need to cache this before we use std::move
            auto sptr = std::make_shared<ComPacket>( std::move(packet) );

            // Post the new packet to the message queues of all the subscribers for this packet type:
            SubscriptionEntry::second_type& queue = m_subscribers[ packetType ];
            for ( auto& subscriber : queue )
            {
                subscriber->m_callback( sptr );
            }
        }
    }

    m_subscribers.clear();
    std::cerr << "PacketDemuxer::Receive() exited." << std::endl;
}

/**
    @param packet If return value is true then packet will contain the new data, if false packet remains unchanged.
    @return false on comms error, true if successful.
*/
bool PacketDemuxer::ReceivePacket( ComPacket& packet )
{
    const int timeoutInMilliseconds = 1000;
    if ( m_transport.ReadyForReading( timeoutInMilliseconds ) == false )
    {
        return false;
    }

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
    if ( !ok )
    {
        return false;
    }

    std::swap( p, packet );
    assert( packet.GetType() != ComPacket::Type::Invalid ); // Catch invalid packets at the lowest level.

    return true;
}

/**
    Loop to guarantee the number of bytes requested are actually read.

    On error (return of false) size will contain the number of bytes that were remaining to be read.

    @return true if all bytes were written, false if there was an error at any point.
*/
bool PacketDemuxer::ReadBytes( uint8_t* buffer, size_t& size )
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
