/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef _COMSUBSCRIBER_H_
#define _COMSUBSCRIBER_H_

#include <queue>

#include "ComPacket.h"

class PacketDemuxer;

/**
    A packet subscription is a component which can subscribe to
    a particular packet type to be received from a PacketDemuxer.

    @todo This file has wrong name and contains 2 classes.
*/
class PacketSubscriber
{
friend class PacketDemuxer;

public:
    typedef std::function< void( const ComPacket::ConstSharedPacket& ) > CallBack;

    virtual ~PacketSubscriber();
    PacketSubscriber( PacketSubscriber& ) = delete;
    PacketSubscriber( PacketSubscriber&& ) = delete;

    ComPacket::Type GetType() const { return m_type; };
    void Unsubscribe();

protected:
    PacketSubscriber( ComPacket::Type type, PacketDemuxer&, CallBack& );

private:
    ComPacket::Type m_type;
    PacketDemuxer&  m_comms;
    CallBack        m_callback;
};

class PacketSubscription
{
public:
    PacketSubscription( const PacketSubscription& )    = delete;
    PacketSubscription& operator=(PacketSubscription&) = delete;

    PacketSubscription( std::shared_ptr<PacketSubscriber>& subscriber );
    PacketSubscription( PacketSubscription&& );
    PacketSubscription& operator=(PacketSubscription&&);

    virtual ~PacketSubscription();

protected:

private:
    std::shared_ptr<PacketSubscriber> m_subscriber;
};

#endif // _COMSUBSCRIBER_H_
