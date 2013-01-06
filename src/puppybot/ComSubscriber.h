/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef _COMSUBSCRIBER_H_
#define _COMSUBSCRIBER_H_

#include <queue>

#include "ComPacket.h"

class ComCentre;

class ComSubscriber
{
friend class ComCentre;

public:
    virtual ~ComSubscriber();
    ComSubscriber( ComSubscriber& ) = delete;
    ComSubscriber( ComSubscriber&& ) = delete;

protected:
    // Constructor is protected: only a ComCentre object can create SubScribers.
    ComSubscriber( ComCentre& );

private:
    ComCentre& m_comms;
    std::queue<ComPacket::SharedPacket> m_queue;
};

#endif // _COMSUBSCRIBER_H_
