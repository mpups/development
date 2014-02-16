#ifndef TELE_JOYSTICK_H
#define TELE_JOYSTICK_H

#include <queue>
#include <thread>

#include "DiffDrive.h"
#include "../packetcomms/ComPacket.h"
#include "../packetcomms/SimpleQueue.h"

class PacketMuxer;
class PacketDemuxer;

/**
    Class which receives tele-operation joystick commands and passes
    them to the differential drive unit. This takes place in a separate
    thread so that the robot is always responsive. The thread runs with
    a high real-time priority if allowed.

    The differential dive unit must be passed into the constructor and
    must have been previously setup correctly.

    You must also pass in a valid and connected socket from which to
    receive the joystick commands. The socket will be 'read only' from
    this object's point of view. The socket must receive only joystick
    command packets and nothing else.

    The case of a non-blocking socket is assumed - it should also
    work with a blocking socket but this is not tested fully. (The socket's
    blocking state is not modified by this class).
**/
class TeleJoystick
{
public:
    TeleJoystick( std::pair<PacketMuxer&,PacketDemuxer&> muxers );
    TeleJoystick( std::pair<PacketMuxer&,PacketDemuxer&> muxers, DiffDrive* drive );
    virtual ~TeleJoystick();

    virtual void Run();
    void ProcessPacket( SimpleQueue & joyPackets, int16_t& jx, int16_t& jy, int16_t& jmax );
    void Go();
    bool IsRunning() const;

private:
    PacketMuxer&   m_muxer;
    PacketDemuxer& m_demuxer;
    DiffDrive*     m_drive;
    volatile bool  m_terminate;
    std::thread    m_thread;
};

#endif // TELE_JOYSTICK_H

