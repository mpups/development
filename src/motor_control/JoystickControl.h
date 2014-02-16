#ifndef JOYSTICKCONTROL_H
#define JOYSTICKCONTROL_H

#include "../packetcomms/PacketMuxer.h"
#include <arpa/inet.h>

namespace robolib
{

static inline void MuxJoystickData(PacketMuxer& muxer, const int16_t jx, const int16_t jy, const int16_t jmax)
{
    constexpr int dataSize = 3;
    int16_t joyData[dataSize] = {
            static_cast<int16_t>(htons(jx)),
            static_cast<int16_t>(htons(jy)),
            static_cast<int16_t>(htons(jmax)) };

    muxer.EmplacePacket( "Joystick", reinterpret_cast<VectorStream::CharType*>(joyData), dataSize*sizeof(int16_t) );
}

} // end namespace robolib

#endif // JOYSTICKCONTROL_H
