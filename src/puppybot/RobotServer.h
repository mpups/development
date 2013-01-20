#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include "../../include/RoboLib.h"
#include "../opencv/opencv_utils.h"
#include "PacketMuxer.h"
#include "PacketDemuxer.h"

#include <arpa/inet.h>

class RobotServer
{
public:
    RobotServer( int tcpPort, const char* motorSerialPort );
    virtual ~RobotServer();

    bool Listen();

    void RunCommsLoop();

private:
    void PostConnectionSetup();
    void PostCommsCleanup();
    void StreamVideo( TeleJoystick& joy );

    GLK::String m_serialPort;

    /// @todo use std::unique_ptr for all of these:
    DiffDrive*  m_drive;
    MotionMind* m_motors;
    PacketMuxer*   m_muxer;
    PacketDemuxer* m_demuxer;
    TcpSocket*  m_server;
    TcpSocket*  m_con;
    UnicapCamera* m_camera;
    uint8_t* m_lum;
};

#endif // ROBOT_SERVER_H
