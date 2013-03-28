#ifndef MUXERCONTROLMESSAGES_H
#define MUXERCONTROLMESSAGES_H

#include <cstdint>

enum class ControlMessage : std::uint8_t
{
    Hello     = 0,
    HeartBeat = 1
};

#endif // MUXERCONTROLMESSAGES_H
