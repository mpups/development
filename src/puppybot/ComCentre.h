/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __COM_CENTRE_H__
#define __COM_CENTRE_H__

#include <glkcore.h>
#include <VideoLib.h>

#include <iostream>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>

#include "ComPacket.h"
#include "ComSubscriber.h"
#include "RunnableFunction.h"

// Need to define a hash function to use strongly typed enum as a map key:
namespace std
{
    template <>
    struct hash<ComPacket::Type>
    {
        size_t operator()(const ComPacket::Type& type) const
        {
            return hash<int>()( static_cast<int>(type) );
        }
    };
}

/**
    Class which manages communications to and from the robot.

    This system is only a simple muxer and demuxer - it does not
    know anything about the messages except their size and type ID
    (ComPacket::Type). The type ID is only used for the purpose
    of muxing/demuxing into different send/receive queues. All
    serialisation of the actual packet data must be done externally.

    The data itself is currently sent as byte stream over TCP.
*/
class ComCentre
{
public:
    typedef std::queue< ComPacket::SharedPacket > PacketContainer;
    typedef std::shared_ptr<ComSubscriber> Subscription;

    ComCentre( Socket& socket );
    virtual ~ComCentre();

    bool Ok() const;

    void Send();
    void Receive();
    void PostPacket( ComPacket&& packet );
    void EmplacePacket( ComPacket::Type type, uint8_t* buffer, int size );
    ComCentre::Subscription Subscribe( ComPacket::Type type );

    PacketContainer& GetAvDataQueue() {
        size_t odoSize = m_rxQueues[ ComPacket::Type::Odometry ].size();
        if ( odoSize )
        {
            std::cerr << "Odo queue asize := " << odoSize << "front data := " << m_rxQueues[ ComPacket::Type::Odometry ].front()->GetData().size() << std::endl;
        }
        return m_rxQueues[ ComPacket::Type::AvData ];
    }

    /**
        Class that wllows external clients to hold a queue resource with appropriate locks.
        The resources are released automatically when a QueueLock goes out of scope.
    */
    class QueueLock
    {
    friend class ComCentre;

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

    QueueLock WaitForPackets( ComPacket::Type type ) {
        m_rxLock.Lock();

        while ( m_transportError == false && m_rxQueues[ type ].empty() )
        {
            m_rxReady.Wait( m_rxLock ); // sleep until a packet is received
        }

        return QueueLock( type, m_rxLock ); // at this point m_rxLock is locked and will be unlocked when the returned object goes out of scope
    };

protected:
    typedef std::pair< ComPacket::Type, PacketContainer > MapEntry;
    typedef std::pair< ComPacket::Type, std::vector<Subscription> > SubscriptionEntry;

    void SendAll( PacketContainer& packets );
    void SendPacket( const ComPacket& packet );
    bool ReceivePacket( ComPacket& packet );

    bool ReadBytes( uint8_t* buffer, size_t& size );
    bool WriteBytes( const uint8_t* buffer, size_t& size );

private:
    void SignalPacketPosted();

    RunnableFunction m_sender;
    RunnableFunction m_receiver;
    GLK::Thread m_sendThread;
    GLK::Thread m_receiveThread;

    GLK::ConditionVariable m_txReady;
    GLK::Mutex m_txLock;
    uint32_t m_numPosted;
    uint32_t m_numSent;

    GLK::ConditionVariable m_rxReady;
    GLK::Mutex m_rxLock;

    std::unordered_map< SubscriptionEntry::first_type, SubscriptionEntry::second_type > m_subscribers;

    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_txQueues;
    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_rxQueues;
    Socket& m_transport;
    bool m_transportError;
};

#endif /* __COMMS_CENTRE_H__ */

