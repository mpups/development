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

private:
    DiffDrive*  m_drive;
    MotionMind* m_motors;
};

#endif // ROBOT_SERVER_H

