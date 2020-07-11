#ifndef MOTION_MIND_H
#define MOTION_MIND_H

#include "../io/SerialPort.h"
#include "stdint.h"

#ifdef __linux
typedef const char* tPortName;
#define DEFAULT_SERIAL_PORT "/dev/ttyUSB0"
#elif defined WIN32
typedef const wchar_t* tPortName;
#define DEFAULT_SERIAL_PORT (L"COM1")
#endif

/**
    Class for communicating with a motion mind motor controllers over a single serial bus.

    All read/write functions wait to receive the ack before returning. This is somewhat
    inefficient but the alternative becomes very complicated.

    Motion-mind(tm) is a motor controler designed by Solutions cubed engineering.
    Should work with versions 2 and 3 of the controller.

    Motion min setup:
    This code assumes the controllers are set to the mode for BINARY PID CONTROL
    and have the appropriate function bits set so that they return POSITION as an ack.
**/
class MotionMind
{
public:
    enum Register
    {
        POSITION        = 0,
        VELOCITYLIMIT,
        VELOCITYFF,
        FUNCTION,
        PTERM,
        ITERM,   
        DTERM,    
        ADDRESS,  
        PIDSCALAR,
        TIMER,   
        RCMAX,  
        RCMIN,   
        RCBAND,  
        RCCOUNT, 
        VELOCITY,
        TIME,
        STATUS,         
        REVISION,       
        MODE,           
        ANALOGCON,      
        ANALOGFBCK,     
        PWMOUT,         
        INDEXPOS,
        VNLIMIT,      
        VPLIMIT,                  
        PWMLIMIT,       
        DEADBAND,      
        DESIREDPOSITION,
        AMPSLIMIT,
        AMPS,
        FUNCTION2,
        NOT_A_REGISTER
    };

    MotionMind( tPortName port );
    ~MotionMind();

    bool Available();
    bool SetSpeed( int32_t addr, int32_t speed_cps, int32_t& position);
    bool Move( int32_t addr, int32_t move_c, int32_t& position );

    Register StringToRegister( const char* );
    bool WriteRegister( int32_t addr, Register reg, int32_t value, bool store=false );
    bool ReadRegister( int32_t addr, Register reg, int32_t& value );

protected:
    static size_t m_regSize[32];
    static char m_regName[32][32];

    bool GetSingleByteAck();
    bool GetAck( int32_t& position );
    bool GetReadAck( int32_t addr, Register reg, int32_t& value );
    char ComputeCheckSum( char* b, size_t n );

private:
    SerialPort m_com;
};

#endif // MOTION_MIND_H

