#include "MotionMind.h"

#include <assert.h>

#include <time.h>

double GetMicroSeconds()
{
    timespec t;
    clock_gettime( CLOCK_MONOTONIC, &t );
    return (t.tv_sec * 1000000.0) + (t.tv_nsec*0.001);
}

double GetThreadMicroSeconds()
{
    timespec t;
    clock_gettime( CLOCK_THREAD_CPUTIME_ID, &t );
    return (t.tv_sec * 1000000.0) + (t.tv_nsec*0.001);
}

size_t MotionMind::m_regSize[] = 
{
    4,2,1,2,2,2,2,1,1,1,2,2,2,2,2,4,2,1,1,2,2,2,4,4,4,2,2,4,2,2,2
};

char MotionMind::m_regName[][32] =
{
        "POSITION",
        "VELOCITYLIMIT",
        "VELOCITYFF",
        "FUNCTION",
        "PTERM",
        "ITERM",   
        "DTERM",    
        "ADDRESS",  
        "PIDSCALAR",
        "TIMER",   
        "RCMAX",  
        "RCMIN",   
        "RCBAND",  
        "RCCOUNT", 
        "VELOCITY",
        "TIME",
        "STATUS",         
        "REVISION",       
        "MODE",           
        "ANALOGCON",      
        "ANALOGFBCK",     
        "PWMOUT",         
        "INDEXPOS",
        "VNLIMIT",      
        "VPLIMIT",                  
        "PWMLIMIT",       
        "DEADBAND",      
        "DESIREDPOSITION",
        "AMPSLIMIT",
        "AMPS",
        "FUNCTION2",
        "NOT_A_REGISTER"
};

/**
    Create a link to MotionMinds on the specified serial port.
**/
MotionMind::MotionMind( tPortName port )
:
    m_com( port )
{
    if ( m_com.IsOpen() )
    {
        m_com.SetBaudRate( SerialPort::BAUD_19200 );
    }
}

/**
    Destroy link to motion minds.
**/
MotionMind::~MotionMind()
{
    // Nothing to do - the serial port is automatically cloesd in its own destructor.
}

/**
    Query whether comms to the motors are available (i.e. if the serial port was opened successfully).

    @return true if comms are available, false otherwise.
**/
bool MotionMind::Available()
{
    return m_com.IsOpen();
}

/**
    Send a speed command to the specified controller (the controllers must
    be programmed to unique addresses before they can share a bus).

    The speed is set in counts per second. To convert to metres per second you need to divide by counts-per-metre
    which will be specific to the motors/encoders/wheel-diameters you are using. For my current setup this is
    approx. 6575 counts-per-metre. E.g. 750 counts/sec = 750/6575 = 0.11 metres/sec.

    On success the call return true and stores the current encoder position in @p positon.
    On failure it returns false and does not write to @p position.

    Internally speed is converted from counts per second to counts per PID loop.
    The conversion assumes that quadrature encoding is enabled.

    @param addr Address/id of the motion mind controller .
    @speed_cps Speed to set in counts per second.
    @position Reference to integer where the current accumulated encoder counts are stored.
    @return true if ack was received intact, false otherwise.
**/
bool MotionMind::SetSpeed( int32_t addr, int32_t speed_cps, int32_t& position )
{
    int32_t speed_c_per_pid_loop = speed_cps / (50*4); // quadrature encoding and 50Hz PID (see motion-mind data sheet).

    char cmd[8];
    cmd[0] = 0x17;
    cmd[1] = addr;
    cmd[2] = speed_c_per_pid_loop & 0xFF;
    cmd[3] = (speed_c_per_pid_loop >> 8) & 0xFF;
    cmd[4] = cmd[0] + cmd[1] + cmd[2] + cmd[3];
    m_com.Write( cmd, 5 );

    return GetAck( position );
}

/**
    Command a controller to move the motor by a set number of encoders counts.

    On success the call return true and stores the current encoder position in @p positon.
    On failure it returns false and does not write to @p position.

    @param addr Address/id of the motion mind controller.
    @move_c Move in counts.
    @position Reference to integer where the current accumulated encoder counts are stored.
    @return true if ack was received intact, false otherwise.
**/
bool MotionMind::Move( int32_t addr, int32_t move_c, int32_t& position )
{
    char cmd[8];
    cmd[0] = 0x16;
    cmd[1] = addr;
    cmd[2] = move_c & 0xFF;
    cmd[3] = (move_c >> 8)  & 0xFF;
    cmd[4] = (move_c >> 16) & 0xFF;
    cmd[5] = (move_c >> 24) & 0xFF;
    cmd[6] = cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5];
    m_com.Write( cmd, 7 );

    return GetAck( position );
}

/**
    Return the register ID associated with the specified (null terminated) string.

    @string Textual name of register.
    @retun Register ID for use with (Write/Read)Register.
**/
MotionMind::Register MotionMind::StringToRegister( const char* string )
{
    /** @todo Should use a map or hash table */
    for ( unsigned int i=0; i<32; ++i )
    {
        if ( 0 == strcmp( string, m_regName[i] ) )
        {
            return static_cast<Register>(i);
        }
    }

    return NOT_A_REGISTER;
}

/**
    Writes the specifed value to the specified register on the motor controller.
    
    @return True if ack was recceived intact, false otherwise.
**/
bool MotionMind::WriteRegister( int32_t addr, MotionMind::Register reg, int32_t value, bool store )
{
    char cmd[8]; // This assumes max reg size is 4 bytes
    if ( store )
    {
        cmd[0] = 0x19; // Write to registers and store to eprom
    }
    else
    {
        cmd[0] = 0x18; // Write to registers
    }
    cmd[1] = addr;
    cmd[2] = reg;
    char sum = cmd[0] + cmd[1] + cmd[2];

    unsigned int l;
    for ( l = 0; l<m_regSize[reg]; ++l )
    {
        cmd[3+l] = value & 0xFF;
        value >>= 8;
        sum += cmd[3+l];                
    }
    cmd[3+l] = sum;

    m_com.Write( cmd, 4 + m_regSize[reg] );
    
    return GetSingleByteAck();
}

/**
    Read contents of specified motion-mind register.
**/
bool MotionMind::ReadRegister( int32_t addr, Register reg, int32_t& value )
{
    char cmd[8]; // This assumes max reg size is 4 bytes
    cmd[0] = 26;
    cmd[1] = addr;

    int32_t readData = 1;
    readData <<= reg;
    int32_t* pData = ((int32_t*)&cmd[2]);
    *pData = readData;

    char sum = 0;
    for ( unsigned int i=0; i<6; ++i )
    {
        sum += cmd[i];
    }
    cmd[6] = ComputeCheckSum( cmd, 6 );
    m_com.Write( cmd, 7 );

    return GetReadAck( addr, reg, value ); 
}

/**
    Write commands use single byte acks.

    @returns true if expected ack was received and checksum was ok, false otherwise.
**/
bool MotionMind::GetSingleByteAck()
{
    bool rval = false;
    char val;
    
    bool byteReady = m_com.WaitForBytes( 20 ); // Wait 20ms - write acks take longer
    
    if ( byteReady )
    {
        int n = m_com.Read( &val, 1 );
        if ( n == 1 && val == 6 )
        {
            rval = true;
        }
    }

    m_com.Flush();

    return rval;
}

/**
    Receive a (RETURN POS) ack from the motion minds.

    @returns true if expected ack was received and checksum was ok, false otherwise.
**/
bool MotionMind::GetAck( int32_t& position )
{
    bool rval = false;
    char cmd[6];
    bool bytesReady = m_com.WaitForBytes( 10 ); // Wait 10ms for response to begin

    if ( bytesReady )
    {
        int left = 6;
        int c = 0;
        const unsigned int MAX_TRIES = 3;
        unsigned int tries = 0;
        while ( tries < MAX_TRIES && c != 6 )
        {
            int n = m_com.Read( cmd+c, left );
            if ( n > 0 )
            {
                c += n;
                left = 6 - c;
            }
            if ( left )
            {
                GLK::Thread::Sleep(2);
            }
            tries++;
        }

        m_com.Flush();

        if ( c == 6  && ComputeCheckSum( cmd, 5 ) == cmd[5] )
        {
            position = *((int32_t*)(cmd+1));
            rval = true;
        }
        
        if (!rval) {fprintf(stderr,"FAIL(%d bytes received back)!\n",c);}
    }

    m_com.Flush();
    
    return rval;
}

/**
    Read acknowledgements contain the contents of the specified register into @p value.
    Upon failure the contents of value is undefined and the function retursn false.

    @param addr Addresss of motor
    @reg The register from which the read was requested.
    @value Reference that will store the result of the read (if successful)
    @returns true if expected ack was received and checksum was ok, false otherwise.
**/
bool MotionMind::GetReadAck( int32_t addr, Register reg, int32_t& value )
{
    char cmd[8];

    const unsigned int expected = 2 + m_regSize[reg];
    int left = expected;
    unsigned int c = 0;
    const unsigned int MAX_TRIES = 10;
    unsigned int tries = 0;
    while ( tries < MAX_TRIES && c != expected )
    {
        int n = m_com.Read( cmd+c, left );
        if ( n > 0 )
        {
            c += n;
            left = expected - c;
        }
        if ( left )
        {
            GLK::Thread::Sleep(2);
        }
        tries++;
    }

    m_com.Flush();

    if ( c == expected && ComputeCheckSum( cmd, expected-1 ) == cmd[expected-1] )
    {
        /** @todo  check address and compute checksum here: */
        value = 0;
        for ( unsigned int i = m_regSize[reg]; i>0; --i )
        {
            value <<= 8;
            value += cmd[i] & 0xFF;
        }
        return true;
    }
    
    return false;    
}

/**
    Computes checksum for byte array.
    @param b Pointer to at least @p n bytes
    @param n Number of bytes over whihc to compute checksum.    
**/
char MotionMind::ComputeCheckSum( char* b, size_t n )
{
    char sum = 0;
    while ( n-- )
    {
        sum += *b++;
    }

    return sum;
}

