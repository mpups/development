#ifndef ROBO_SERIAL_PORT_H
#define ROBO_SERIAL_PORT_H

#include <cstddef>

#ifdef __linux

#include <termios.h>

#define HANDLE int
#define LPCWSTR const char*
#define INVALID_HANDLE_VALUE -1

#elif defined WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

/**
    Very basic serial port interface.
**/
class SerialPort
{
public:

    enum BaudRate
    {
        BAUD_9600 = B9600,
        BAUD_19200 = B19200
    };

    static void Test();

    SerialPort( LPCWSTR portName );
    ~SerialPort();

    void SetBaudRate( BaudRate );
    void SetRawOn();
    void SetRawOff();

    bool WaitForBytes( int timeout_ms ) const;

    size_t Write( const char* buffer, size_t numBytes );
    int Printf( const char* formatString, ... );
        
    size_t Read( char* buffer, size_t numBytes );
    int Scanf( const char* formatString, ... );

    size_t ReadLine( char* buffer, size_t maxBytes );

    void Flush();

    //! Returns true if the port was opened successfully, false otherwise.
    bool IsOpen() { return m_port != INVALID_HANDLE_VALUE; };

private:
    HANDLE m_port;
};

#endif // ROBO_SERIAL_PORT_H

