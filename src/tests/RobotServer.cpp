#include "RobotServer.h"

/**
    Setup a robot server with specified TCP and serial ports.
    
    TCP is used for a remote tele-link and the serial port is used for differential drive control (if available).
**/
RobotServer::RobotServer( const char* tcpPort, const char* motorSerialPort )
:
    m_drive ( 0 ),
    m_motors( new MotionMind( motorSerialPort ) )    
{
    if ( m_motors->Available() )
    {
        m_drive = new DiffDrive( *m_motors );
        float amps = 1.5f;
        int32_t currentLimit = roundf( amps/0.02f );
        int32_t pwmLimit = (72*1024)/120; // motor voltage / battery voltage
        
        m_motors->WriteRegister( 1, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 2, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 1, MotionMind::PWMLIMIT, pwmLimit );
        m_motors->WriteRegister( 2, MotionMind::PWMLIMIT, pwmLimit );
    }
    else
    {
        delete m_motors;
        m_motors = 0;
    }
}

RobotServer::~RobotServer()
{
    
}

void RobotServer::Listen()
{

}

