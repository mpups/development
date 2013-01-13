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
        GLK::MutexLock lock( m_rxLock );
        m_transportError = true; /// Causes receive-thread to exit (@todo use better method)
        m_rxReady.WakeAll();
    }

    m_receiveThread.Join();
}

/**
    Returns a subscriber object.

    @todo - This is all wrong. Problem here using Subscription(shared_ptr) here is that it won't get deleted
    automatically. Need to return an object that wraps a reference to the subscriber.
    I.e. return class Subscription { which wraps a Subscriber }; Then on unsubscribe, we pass
    the subscription back to COmCentre which can search for and remove the subscriber record.
*/
PacketSubscription PacketDemuxer::Subscribe( ComPacket::Type type, PacketSubscriber::CallBack callback )
{
    SubscriptionEntry::second_type& queue = m_subscribers[ type ];
    queue.emplace_back( new PacketSubscriber( type, *this , callback ) );
    m_nextSubscriberId += 1;
    return PacketSubscription( queue.back() );
}

void PacketDemuxer::Unsubscribe( PacketSubscriber* pSubscriber )
{
    /// @todo - how to locate the subscription? By raw pointer value?
    ComPacket::Type type = pSubscriber->GetType();
    SubscriptionEntry::second_type& queue = m_subscribers[ type ];

    // Search through all subscribers of this type for the specific subscriber:
    auto itr = std::remove_if( queue.begin(), queue.end(), [pSubscriber]( const PacketDemuxer::Subscriber& subscriber ) {
        return subscriber.get() == pSubscriber;
    });

    assert( itr != queue.end() );
    queue.erase( itr );
}

void PacketDemuxer::Receive()
{
    //std::cerr << "Receive thread started." << std::endl;
    ComPacket packet;
    while ( m_transportError == false )
    {
        if ( ReceivePacket( packet ) )
        {
            GLK::MutexLock lock( m_rxLock );
            ComPacket::Type packetType = packet.GetType(); // Need to cache this before we use std::move
            auto sptr = std::make_shared<ComPacket>( std::move(packet) );
            m_rxQueues[ packetType ].push( sptr );
            m_rxReady.WakeOne();

            // Post the new packet to the message queues of all the subscribers for this packet type:
            SubscriptionEntry::second_type& queue = m_subscribers[ packetType ];
            for ( auto& subscriber : queue )
            {
                subscriber->m_callback( sptr );
            }
        }
    }
}

/**
    @param packet If return value is true then packet will contain the new data, if false packet remains unchanged.
    @return false on comms error, true if successful.
*/
bool PacketDemuxer::ReceivePacket( ComPacket& packet )
{
    const int timeoutInMilliseconds = 1000;
    if ( m_transport.WaitForData( timeoutInMilliseconds ) == false )
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

ComPacket::PacketContainer& PacketDemuxer::GetAvDataQueue()
{
    size_t odoSize = m_rxQueues[ ComPacket::Type::Odometry ].size();
    if ( odoSize )
    {
        std::cerr << "Odo queue asize := " << odoSize << "front data := " << m_rxQueues[ ComPacket::Type::Odometry ].front()->GetData().size() << std::endl;
    }
    return m_rxQueues[ ComPacket::Type::AvData ];
}

PacketDemuxer::QueueLock PacketDemuxer::WaitForPackets( ComPacket::Type type )
{
    m_rxLock.Lock();

    while ( m_transportError == false && m_rxQueues[ type ].empty() )
    {
        m_rxReady.Wait( m_rxLock ); // sleep until a packet is received
    }

    return QueueLock( type, m_rxLock ); // at this point m_rxLock is locked and will be unlocked when the returned object goes out of scope
}
