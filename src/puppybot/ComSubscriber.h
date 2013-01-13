/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef _COMSUBSCRIBER_H_
#define _COMSUBSCRIBER_H_

#include <queue>

#include "ComPacket.h"

class PacketDemuxer;

class ComSubscriber
{
friend class PacketDemuxer;

public:
    typedef std::function< void( const ComPacket::ConstSharedPacket& ) > CallBack;

    virtual ~ComSubscriber();
    ComSubscriber( ComSubscriber& ) = delete;
    ComSubscriber( ComSubscriber&& ) = delete;

    ComPacket::Type GetType() const { return m_type; };

protected:
    // Constructor is protected: only a ComCentre object can create SubScribers.
    ComSubscriber( ComPacket::Type type, PacketDemuxer&, CallBack& );

private:
    ComPacket::Type m_type;
    PacketDemuxer&      m_comms;
    CallBack        m_callback;
};

#endif // _COMSUBSCRIBER_H_
