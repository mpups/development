#include "ComSubscriber.h"

#include "PacketDemuxer.h"

ComSubscriber::ComSubscriber(ComPacket::Type type, PacketDemuxer& comms, CallBack& callback )
:
    m_type      (type),
    m_comms     ( comms ),
    m_callback  ( callback)
{
}

ComSubscriber::~ComSubscriber()
{
   /// @todo m_comms.Unsubscribe( this );
}
