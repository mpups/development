/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __PACKET_DEMUXER_H__
#define __PACKET_DEMUXER_H__

/// @todo Need VideoLib and GLK for Locks and Sockets - should be independent of these ideally.
#include <glkcore.h>
#include <VideoLib.h>

#include <unordered_map>

#include "RunnableFunction.h"
#include "ComPacket.h"
#include "ComSubscriber.h"

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
    typedef std::shared_ptr<PacketSubscriber> Subscriber;

    PacketDemuxer( Socket& socket );
    virtual ~PacketDemuxer();

    PacketSubscription Subscribe( ComPacket::Type type, PacketSubscriber::CallBack callback );
    void Unsubscribe( PacketSubscriber* subscriber );

    void Receive();
    bool ReceivePacket( ComPacket& packet );

    /**
        Class that allows external clients to hold a queue resource with appropriate locks.
        The resources are released automatically when a QueueLock goes out of scope.
    */
    class QueueLock
    {
    friend class PacketDemuxer;

    public:
        virtual ~QueueLock() { if ( m_lock != nullptr ) { m_lock->Unlock(); } };
        QueueLock( const QueueLock& ) = delete;
        QueueLock( QueueLock&& ql ) : m_type(ql.m_type), m_lock(ql.m_lock) { ql.m_lock = nullptr; };

    protected:
        QueueLock( ComPacket::Type type, GLK::Mutex& mutex ) : m_type(type), m_lock(&mutex) {
            // m_lock should already be locked - it will be unlocked when the QueueLock goes out of scope.
        };

    private:
        ComPacket::Type m_type;
        GLK::Mutex* m_lock;
    };

    ComPacket::PacketContainer& GetAvDataQueue();
    QueueLock WaitForPackets( ComPacket::Type type );

protected:
    typedef std::pair< ComPacket::Type, ComPacket::PacketContainer > MapEntry;
    typedef std::pair< ComPacket::Type, std::vector<Subscriber> > SubscriptionEntry;

    bool ReadBytes( uint8_t* buffer, size_t& size );

private:
    RunnableFunction m_receiver;
    GLK::Thread m_receiveThread;

    GLK::ConditionVariable m_rxReady;
    GLK::Mutex m_rxLock;
    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_rxQueues;

    int m_nextSubscriberId;
    std::unordered_map< SubscriptionEntry::first_type, SubscriptionEntry::second_type > m_subscribers;

    Socket& m_transport;
    bool m_transportError;

};

#endif /* __PACKET_DEMUXER_H__ */
