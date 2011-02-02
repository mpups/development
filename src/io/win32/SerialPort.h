#ifndef GLK_SERIAL_PORT_H
#define GLK_SERIAL_PORT_H

#include "../StdIO.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace GLK
{

/**
    Very basic serial port interface.
**/
class FW_API SerialPort : public StdIO
{
public:
    static void Test();

    SerialPort( LPCSTR portName );
    ~SerialPort();

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

} // end namepsace GLK

#endif // GLK_SERIAL_PORT_H
