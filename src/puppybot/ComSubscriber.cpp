#include "ComSubscriber.h"

#include "PacketMuxer.h"

ComSubscriber::ComSubscriber(ComPacket::Type type, PacketMuxer& comms, CallBack& callback )
:
    m_type      (type),
    m_comms     ( comms ),
    m_callback  ( callback)
{
}

ComSubscriber::~ComSubscriber()
{
    m_comms.Unsubscribe( this );
}
