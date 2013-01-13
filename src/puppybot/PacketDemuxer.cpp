#include "PacketDemuxer.h"

PacketDemuxer::PacketDemuxer( Socket& socket )
:
    m_receiver      ( std::bind(&PacketDemuxer::Receive, std::ref(*this)) ),
    m_receiveThread ( m_receiver ),
    m_transport     ( socket ),
    m_transportError( false )
{
}

PacketDemuxer::~PacketDemuxer()
{
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
            auto sptr = std::make_shared<ComPacket>(std::move(packet));
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

