#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include "../../include/RoboLib.h"

#include <memory>
#include <string>
#include <vector>

class TcpSocket;

class RobotServer
{
public:
    RobotServer( int tcpPort, const char* motorSerialPort );
    virtual ~RobotServer();

    bool Listen(const std::vector<std::string>& packetTypes);

    void RunCommsLoop();

    void SetupMotors();
    void SetupCamera();
private:
    void PostConnectionSetup(const std::vector<std::string>& packetTypes);
    void PostCommsCleanup();
    void StreamVideo( TeleJoystick& joy );

    std::string m_serialPort;

    std::unique_ptr<MotionMind>     m_motors;
    std::unique_ptr<DiffDrive>      m_drive;
    std::unique_ptr<PacketMuxer>    m_muxer;
    std::unique_ptr<PacketDemuxer>  m_demuxer;
    std::unique_ptr<TcpSocket>      m_server;
    std::unique_ptr<TcpSocket>      m_con;
    std::unique_ptr<UnicapCamera>   m_camera;
};

#endif // ROBOT_SERVER_H
