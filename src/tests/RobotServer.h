#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include "../../include/RoboLib.h"
#include "../opencv/opencv_utils.h"

#include <arpa/inet.h>

class RobotServer
{
public:
    RobotServer( const char* tcpPort, const char* motorSerialPort );
    virtual ~RobotServer();

    void Listen();

    void RunCommsLoop();

private:
    void PostConnectionSetup();
    void PostCommsCleanup();

    DiffDrive*  m_drive;
    MotionMind* m_motors;
    Socket*     m_server;
    Socket*     m_con;
    UnicapCamera* m_camera;
    uint8_t* m_lum;
};

#endif // ROBOT_SERVER_H

