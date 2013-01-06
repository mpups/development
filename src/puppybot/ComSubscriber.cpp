#include "ComSubscriber.h"

#include "ComCentre.h"

ComSubscriber::ComSubscriber(ComPacket::Type type, ComCentre& comms, CallBack& callback )
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
