/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __PACKET_DEMUXER_H__
#define __PACKET_DEMUXER_H__

/// @todo Need VideoLib and GLK for Locks and Sockets - should be independent of these ideally.
#include <glkcore.h>
#include <VideoLib.h>

#include <unordered_map>

#include "../utility/RunnableFunction.h"
#include "../packetcomms/ComPacket.h"
#include "../packetcomms/ComSubscriber.h"

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

    PacketDemuxer( Socket& socket );
    virtual ~PacketDemuxer();

    bool Ok() const;

    PacketSubscription Subscribe( ComPacket::Type type, PacketSubscriber::CallBack callback );
    void Unsubscribe( const PacketSubscriber *subscriber );
    bool IsSubscribed( const PacketSubscriber* subscriber ) const;

    void Receive();
    bool ReceivePacket( ComPacket& packet );

protected:
    typedef std::pair< ComPacket::Type, std::vector<SubscriberPtr> > SubscriptionEntry;

    bool ReadBytes( uint8_t* buffer, size_t& size, bool transportErrorOnZeroBytes=false );

private:
    RunnableFunction m_receiver;
    GLK::Thread m_receiveThread;

    int m_nextSubscriberId;
    std::unordered_map< SubscriptionEntry::first_type, SubscriptionEntry::second_type > m_subscribers;

    Socket& m_transport;
    bool m_transportError;

};

#endif /* __PACKET_DEMUXER_H__ */
