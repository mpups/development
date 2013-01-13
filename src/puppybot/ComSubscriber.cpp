#include "ComSubscriber.h"

#include "PacketDemuxer.h"

/**
    This is the only constructor and is protected: the intent being
    that only a PacketDemuxer object can construct a PacketSubscription.
*/
PacketSubscriber::PacketSubscriber( ComPacket::Type type, PacketDemuxer& comms, CallBack& callback )
:
    m_type      (type),
    m_comms     ( comms ),
    m_callback  ( callback)
{
}

PacketSubscriber::~PacketSubscriber()
{
}

void PacketSubscriber::Unsubscribe()
{
    m_comms.Unsubscribe( this );
}

PacketSubscription::PacketSubscription( std::shared_ptr<PacketSubscriber>& subscriber )
:
    m_subscriber( subscriber )
{
}

PacketSubscription::PacketSubscription( PacketSubscription&& toMove )
{
    std::swap( m_subscriber, toMove.m_subscriber );
}

PacketSubscription& PacketSubscription::operator=( PacketSubscription&& toMove )
{
    m_subscriber->Unsubscribe();
    m_subscriber.reset();
    std::swap( m_subscriber, toMove.m_subscriber );
}

/**
    When a subscription goes out of scope it automatically removes
    its subscription from the PacketMuxer that created it.
*/
PacketSubscription::~PacketSubscription()
{
    m_subscriber->Unsubscribe();
}
