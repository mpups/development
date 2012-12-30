/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __COM_CENTRE_H__
#define __COM_CENTRE_H__

#include <glkcore.h>
#include <VideoLib.h>

#include <cstdint>
#include <queue>
#include <unordered_map>
#include <functional>

#include "ComPacket.h"
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

    Holds various message queues of different priorities.
*/
class ComCentre
{
public:
    ComCentre( Socket& socket );
    virtual ~ComCentre();

    bool Ok() const;

    void Send();
    void Receive();
    void PostPacket( ComPacket&& packet );

    std::queue<ComPacket>& GetAvDataQueue() { return m_rxQueues[ ComPacket::Type::AvData ]; };

    bool LockPackets() {
        m_rxLock.Lock();
        m_rxReady.Wait( m_rxLock );
        if ( m_rxQueues[ ComPacket::Type::AvData ].size() > 0 )
        {
            return true;
        }
        else
        {
            m_rxLock.Unlock();
            return false;
        }
    };
    void DonePackets() { m_rxLock.Unlock(); };

protected:
    typedef std::queue<ComPacket> PacketContainer;
    typedef std::pair< ComPacket::Type, PacketContainer > MapEntry;

    void SendAll( PacketContainer& packets );
    void SendPacket( const ComPacket& packet );
    bool ReceivePacket( ComPacket& packet );

    bool ReadBytes( uint8_t* buffer, size_t& size );
    bool WriteBytes( const uint8_t* buffer, size_t& size );

private:
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

    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_txQueues;
    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_rxQueues;
    Socket& m_transport;
    bool m_transportError;
};

#endif /* __COMMS_CENTRE_H__ */

