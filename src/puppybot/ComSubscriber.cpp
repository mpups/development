#include "ComSubscriber.h"

#include "ComCentre.h"

ComSubscriber::ComSubscriber( ComCentre& comms )
:
    m_comms(comms)
{
}

ComSubscriber::~ComSubscriber()
{
    //m_comms.Unsubscribe( *this );
}
