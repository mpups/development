/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef _PACKET_MUXER_H_
#define _PACKET_MUXER_H_

/// @todo Need VideoLib and GLK for Locks and Sockets - should be independent of these ideally.
#include <glkcore.h>
#include <VideoLib.h>

#include <iostream>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>

#include "ComPacket.h"
#include "PacketSubscription.h"
#include "ControlMessage.h"

#include "../utility/RunnableFunction.h"

/**
    Class which manages communications to and from the robot.

    This is the muxer half of the comms-system - it does not
    know anything about the messages except their size and type ID
    (ComPacket::Type).

    Throughout the low-level comms system the type ID is only used
    for muxing/demuxing packets into different send/receive queues.
    All serialisation of the actual packet data must be done externally.

    The muxer receives packets posted to it from any number of threads
    as messages and then sends the packets over the transport layer.

    The data itself is currently sent as byte stream over TCP.
*/
class PacketMuxer
{
public:
    typedef std::shared_ptr<PacketSubscriber> Subscription;

    PacketMuxer( Socket& socket );
    virtual ~PacketMuxer();

    bool Ok() const;

    void Send();
    void PostPacket( ComPacket&& packet );
    void EmplacePacket( ComPacket::Type type, uint8_t* buffer, int size );

protected:
    typedef std::pair< ComPacket::Type, ComPacket::PacketContainer > MapEntry;
    typedef std::pair< ComPacket::Type, std::vector<Subscription> > SubscriptionEntry;

    void SendAll( ComPacket::PacketContainer& packets );
    void SendPacket( const ComPacket& packet );

    bool WriteBytes( const uint8_t* buffer, size_t& size );

private:
    void SignalPacketPosted();

    RunnableFunction m_sender;
    GLK::Thread m_sendThread;

    GLK::ConditionVariable m_txReady;
    GLK::Mutex m_txLock;
    uint32_t m_numPosted;
    uint32_t m_numSent;

    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_txQueues;

    Socket& m_transport;
    bool m_transportError;

    void SendControlMessage( ControlMessage msg );
};

#endif /* _PACKET_MUXER_H_ */
