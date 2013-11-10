/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __PACKET_DEMUXER_H__
#define __PACKET_DEMUXER_H__

#include <unordered_map>
#include <string>
#include <initializer_list>
#include <mutex>

#include "IdManager.h"
#include "ComPacket.h"
#include "PacketSubscription.h"
#include "PacketSubscriber.h"
#include "ControlMessage.h"
#include "../network/Socket.h"
#include "../utility/SimpleAsyncFunction.h"

/**
    Class which manages communications to and from the robot.

    This is the demuxer half of the comms-system - it does not
    know anything about the messages except their size and type ID
    (ComPacket::Type).

    Throughout the low-level comms system the type ID is only used
    for muxing/demuxing packets into different send/receive queues.
    All serialisation of the actual packet data must be done externally.

    The demuxing reads packets from the transport layer and then sends
    these packets onto any subscribers registered for the packet type.

    The data itself is currently sent as byte stream over TCP.
*/
class PacketDemuxer
{
public:
    typedef std::shared_ptr<PacketSubscriber> SubscriberPtr;

    PacketDemuxer( AbstractSocket& socket, const std::vector<std::string>& packetIds );
    virtual ~PacketDemuxer();

    bool Ok() const;

    PacketSubscription Subscribe( const std::string& type, PacketSubscriber::CallBack callback );
    void Unsubscribe( const PacketSubscriber *subscriber );
    bool IsSubscribed( const PacketSubscriber* subscriber ) const;

    void ReceiveLoop();
    bool ReceivePacket( ComPacket& packet, const int timeoutInMilliseconds );

protected:
    typedef std::pair< IdManager::PacketType, std::vector<SubscriberPtr> > SubscriptionEntry;

    bool ReadBytes( uint8_t* buffer, size_t& size, bool transportErrorOnZeroBytes=false );
    void SignalTransportError();

private:
    IdManager m_packetIds;
    int m_nextSubscriberId;
    std::mutex m_subscriberLock;
    std::unordered_map< SubscriptionEntry::first_type, SubscriptionEntry::second_type > m_subscribers;
    AbstractSocket& m_transport;
    bool m_transportError;

    // This must be initialised last to ensure all other members are intialised before the thread starts:
    SimpleAsyncFunction m_receiver;

    void ReceiveHelloMessage( ComPacket& packet, int timeoutInMillisecs );
    void HandleControlMessage( const ComPacket::ConstSharedPacket& sptr );
    ControlMessage GetControlMessage( const ComPacket::ConstSharedPacket& sptr );
};

#endif /* __PACKET_DEMUXER_H__ */
