#ifndef JOYSTICKCONTROL_H
#define JOYSTICKCONTROL_H

#include "../packetcomms/PacketMuxer.h"
#include "../packetcomms/PacketSerialisation.h"
#include <arpa/inet.h>

namespace robolib
{

static inline void MuxJoystickData(PacketMuxer& muxer, const int16_t jx, const int16_t jy, const int16_t jmax)
{
    Serialise( muxer, "Joystick", jx, jy, jmax );
}

} // end namespace robolib

#endif // JOYSTICKCONTROL_H
