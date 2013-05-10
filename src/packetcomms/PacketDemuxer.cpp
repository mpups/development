#include "PacketDemuxer.h"

#include <iostream>
#include <algorithm>

#include <arpa/inet.h>
#include <assert.h>

/**
    Create a new demuxer that will receive packets from the specified socket.

    This object is guaranteed to only ever read from the socket.
*/
PacketDemuxer::PacketDemuxer( AbstractSocket &socket )
:
    m_receiver      ( std::bind(&PacketDemuxer::ReceiveLoop, std::ref(*this)) ),
    m_nextSubscriberId (0),
    m_transport     ( socket ),
    m_transportError( false )
{
    m_transport.SetBlocking( false );
}

PacketDemuxer::~PacketDemuxer()
{
    SignalTransportError(); /// Causes receive-thread to exit (@todo use better method)
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

    Runs asnchronously in its own thread :- it is passed
    using std::bind to a SimpleAsyncFunction object.
*/
void PacketDemuxer::ReceiveLoop()
{
    std::cerr << "PacketDemuxer::Receive() entered." << std::endl;
    ComPacket packet;
    constexpr int helloTimeoutInMilliseconds = 2000;
    ReceiveHelloMessage( packet, helloTimeoutInMilliseconds );

    while ( m_transportError == false )
    {
        constexpr int timeoutInMilliseconds = 1000;
        if ( ReceivePacket( packet, timeoutInMilliseconds ) )
        {
            const ComPacket::Type packetType = packet.GetType(); // Need to cache this before we use std::move
            auto sptr = std::make_shared<ComPacket>( std::move(packet) );

            if ( packetType == ComPacket::Type::Control )
            {
                // Control messages are used by the muxer to communicate
                // with the demuxer (this is a one way protocol).
                HandleControlMessage( sptr );
            }
            else
            {
                // Post the new packet to the message queues of all the subscribers for this packet type:
                SubscriptionEntry::second_type& queue = m_subscribers[ packetType ];
                for ( auto& subscriber : queue )
                {
                    subscriber->m_callback( sptr );
                }
            }
        }
    }

    std::cerr << "PacketDemuxer::Receive() exited." << std::endl;
}

/**
    @param packet If return value is true then packet will contain the new data, if false packet remains unchanged.
    @return false on comms error, true if successful.
*/
bool PacketDemuxer::ReceivePacket( ComPacket& packet, const int timeoutInMilliseconds )
{
    if ( m_transport.ReadyForReading( timeoutInMilliseconds ) == false )
    {
        return false;
    }

    uint32_t type = 0;
    uint32_t size = 0;

    // For the first read we generate a transport error on zero bytes
    // (because ReadyForReading() said there were bytes available):
    size_t byteCount = sizeof(uint32_t);
    bool ok = ReadBytes( reinterpret_cast<uint8_t*>(&type), byteCount, true );
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

    @param transportErrorOnZeroBytes If this is true then in the case that Read() returns zero bytes
    a transport error will be signalled and ReadBytes() will return false.

    @return true if all bytes were written, false if there was an error at any point. Also see impact
    of the transportErrorOnZeroBytes parameter on the return value.
*/
bool PacketDemuxer::ReadBytes( uint8_t* buffer, size_t& size, bool transportErrorOnZeroBytes )
{
    while ( size > 0 && m_transportError == false )
    {
        int n = m_transport.Read( reinterpret_cast<char*>( buffer ), size );

        if ( n < 0 || (n == 0 && transportErrorOnZeroBytes) )
        {
            SignalTransportError();
            return false;
        }

        size -= n;
        buffer += n;
    }

    if ( m_transportError )
    {
        return false;
    }

    return true;
}

void PacketDemuxer::SignalTransportError()
{
    m_transportError = true;
}

/**
    Receive the hello message. The first packet sent from a PacketMuxer to
    a demuxer will always be an Hello control message. If the first message
    is not such a message it is considered a transport error and the demuxer
    will terminate for safety.

    This should make it extremely unlikely that an accidental connection
    can cause the demuxer to do anything dodgy. This is not intended to be
    a security measure, if security is important the application must perform
    its own secure handshaking procedure at a higher level (external to the
    Muxer/Demuxer system).
*/
void PacketDemuxer::ReceiveHelloMessage( ComPacket& packet, const int timeoutInMillisecs )
{
    if ( ReceivePacket( packet, timeoutInMillisecs ) )
    {
        bool failHard = true;

        // Very first packet should be a 'Hello' control packet:
        if ( packet.GetType() == ComPacket::Type::Control )
        {
            auto sptr = std::make_shared<ComPacket>( std::move(packet) );
            ControlMessage msg = GetControlMessage( sptr );
            if ( msg == ControlMessage::Hello )
            {
                failHard = false;
            }
        }

        if ( failHard )
        {
            std::cerr << "Error in PacketDemuxer::Receive() - first message was not 'Hello'." << std::endl;
            SignalTransportError();
        }
    }
}

void PacketDemuxer::HandleControlMessage( const ComPacket::ConstSharedPacket& sptr )
{
}

ControlMessage PacketDemuxer::GetControlMessage( const ComPacket::ConstSharedPacket& sptr )
{
    const uint8_t netMsg = *(sptr.get()->GetDataPtr());
    return static_cast<ControlMessage>( netMsg );
}
