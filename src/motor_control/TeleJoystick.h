#ifndef TELE_JOYSTICK_H
#define TELE_JOYSTICK_H

#include <glkcore.h>
#include "../io/Socket.h"
#include "DiffDrive.h"

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
**/
class TeleJoystick : public GLK::Thread
{
public:
    TeleJoystick( Socket& socket, DiffDrive& drive );
    virtual ~TeleJoystick();
    
    virtual void Run();

protected:
    
    
private:
    Socket&     m_socket;
    DiffDrive&  m_drive;
    int32_t     m_failedReads;
};

#endif // TELE_JOYSTICK_H

