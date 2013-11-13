/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef _PACKET_MUXER_H_
#define _PACKET_MUXER_H_

#include <VideoLib.h>

#include <iostream>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>

#include "IdManager.h"
#include "ComPacket.h"
#include "PacketSubscription.h"
#include "ControlMessage.h"
#include "../network/AbstractSocket.h"

#include "../io/VectorStream.h"

#include <mutex>
#include <chrono>
#include <condition_variable>
#include <thread>

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
    friend void TestPacketMuxer();

public:
    typedef std::shared_ptr<PacketSubscriber> Subscription;

    PacketMuxer( AbstractSocket& socket, const std::vector<std::string>& packetIds );
    virtual ~PacketMuxer();

    bool Ok() const;

    //void PostPacket( ComPacket&& packet );
    void EmplacePacket(const std::string&, const VectorStream::CharType* buffer, int size );
    void EmplacePacket( const std::string& name, VectorStream::Buffer&& vector );

protected:
    typedef std::pair< IdManager::PacketType, ComPacket::PacketContainer > MapEntry;
    typedef std::pair< IdManager::PacketType, std::vector<Subscription> > SubscriptionEntry;

    void SendLoop();
    void SendAll( ComPacket::PacketContainer& packets );
    void SendPacket( const ComPacket& packet );

    bool WriteBytes( const uint8_t* buffer, size_t& size );

    uint32_t GetNumPosted() const { return m_numPosted; };
    uint32_t GetNumSent() const { return m_numSent; };

private:
    void SignalPacketPosted();

    IdManager m_packetIds;

    // Need a recursive mutex so that we can emplace control packets
    // to the queue internally while we already hold the tx lock:
    std::recursive_mutex m_txLock;
    std::condition_variable_any m_txReady;
    uint32_t m_numPosted;
    uint32_t m_numSent;

    std::unordered_map< MapEntry::first_type, MapEntry::second_type > m_txQueues;

    AbstractSocket& m_transport;
    bool m_transportError;

    // Async function should be initialised last - it requires everything else to
    // be setup before it can run:
    std::thread m_sendThread;

    void SendControlMessage( ControlMessage msg );
};

#endif /* _PACKET_MUXER_H_ */
