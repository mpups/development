#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include "../../include/RoboLib.h"
#include "../opencv/opencv_utils.h"
#include "PacketMuxer.h"
#include "PacketDemuxer.h"

#include <arpa/inet.h>

#include <memory>

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

    std::unique_ptr<MotionMind>     m_motors;
    std::unique_ptr<DiffDrive>      m_drive;
    std::unique_ptr<PacketMuxer>    m_muxer;
    std::unique_ptr<PacketDemuxer>  m_demuxer;
    std::unique_ptr<TcpSocket>      m_server;
    std::unique_ptr<TcpSocket>      m_con;
    std::unique_ptr<UnicapCamera>   m_camera;
    uint8_t* m_lum;
};

#endif // ROBOT_SERVER_H
