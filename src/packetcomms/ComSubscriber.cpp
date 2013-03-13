#include "ComSubscriber.h"

#include "PacketDemuxer.h"

/**
    This is the only constructor and is protected: the intent being
    that only a PacketDemuxer object can construct a PacketSubscription.
*/
PacketSubscriber::PacketSubscriber( ComPacket::Type type, PacketDemuxer& comms, CallBack& callback )
:
    m_type      ( type ),
    m_comms     ( comms ),
    m_callback  ( callback)
{
}

PacketSubscriber::~PacketSubscriber()
{
}

/**
    This is protected and only should be accessed by the friend class PacketSubscription.

    Currently if a client was allowed to call unsubscribe on a subscriber then an assert
    would trigger in PacketDemuxer::Unsubscribe() when the PacketSubscription attempted
    to unsubscribe automatically on destruction.
*/
void PacketSubscriber::Unsubscribe()
{
    m_comms.Unsubscribe( this );
}

bool PacketSubscriber::IsSubscribed() const
{
    return m_comms.IsSubscribed( this );
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
    return *this;
}

/**
    When a subscription goes out of scope it automatically removes
    its subscription from the PacketMuxer that created it.
*/
PacketSubscription::~PacketSubscription()
{
    m_subscriber->Unsubscribe();
}

/**
    @return true if the underlying subscriber is still subscribed, false otherwise.

    The underlying subscriber can be unsubscribed before the PacketSubscription
    goes out of scope if the PacketDemuxer's receive thread terminates.
*/
bool PacketSubscription::IsSubscribed() const
{
    return m_subscriber->IsSubscribed();
}
