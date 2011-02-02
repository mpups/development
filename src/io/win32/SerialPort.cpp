#include "SerialPort.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

namespace GLK
{

SerialPort::SerialPort( LPCSTR portName )
{
    m_port = CreateFile( portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0 );
    
    // setup a timeouts strcuture to be optimal for comms with motion-mind
    COMMTIMEOUTS to;
    GetCommTimeouts( m_port, &to );
    to.ReadIntervalTimeout = 0;
    to.ReadTotalTimeoutMultiplier = 2;
    to.ReadTotalTimeoutConstant = 0;
    SetCommTimeouts( m_port, &to );

    DCB state;
    SecureZeroMemory( &state, sizeof(DCB) );
    state.DCBlength = sizeof(DCB);

    int err = GetCommState( m_port, &state );
    assert( err != 0 );
   
    state.BaudRate = CBR_19200;
    state.ByteSize = 8;
    state.Parity = NOPARITY;
    state.StopBits = ONESTOPBIT;
    
    err = SetCommState( m_port, &state );
    assert( err != 0 );
}

SerialPort::~SerialPort()
{
    if ( m_port != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_port );
    }
}

size_t SerialPort::Write( const char* buffer, size_t numBytes )
{
    DWORD numWritten;
    WriteFile( m_port, static_cast<LPCVOID>( buffer ), numBytes, &numWritten, 0 );
    return numWritten;
}

int SerialPort::Printf( const char* formatString, ... )
{
    DWORD numWritten;
    va_list args;
    va_start( args, formatString );

    int numChars = _vscprintf( formatString, args );
    char* buf = new char[numChars+1];
    
    int numPrinted = vsnprintf( buf, numChars, formatString, args );
    assert( numPrinted == numChars );

    WriteFile( m_port, static_cast<LPCVOID>( buf ), numChars, &numWritten, 0 );

    delete [] buf;
    va_end( args );

    return numWritten;
}
        
/**
    Read a number of bytes from the serial port.

    Whether the call blocks if not enough bytes are
    available or returns with the bytes that are
    available depends on the serial port settings.
**/
size_t SerialPort::Read( char* buffer, size_t numBytes )
{
    DWORD numRead;
    ReadFile( m_port, static_cast<LPVOID>( buffer ), numBytes, &numRead, 0 );
    return numRead;
}

int SerialPort::Scanf( const char* formatString, ... )
{
    assert(0); // Scanf() doesn't work yet, sorry!
    return 0;
}

/**
    Keep reading characters into buffer until CR or LF is
    encountered or until there is no more data available.

    Whether the call blocks if not enough bytes are
    available or returns with the bytes that are available
    depends on the serial port settings.

    CR+LF will appear as 2 lines - The second empty.

    The new-line character is not written to buffer.

    A terminating character is written to the buffer.

    An empty line will return a count of 1 and a terminating character will be written at buffer[0].

    If there is no data or an end of file then ReadLine will return 0;

    @param buffer Buffer to receive the line. Must be at least maxBytes in size.
    @return The number of characters put in the buffer including the terminating character - will be 0 if there was no data.
**/
size_t SerialPort::ReadLine( char* buffer, size_t maxBytes )
{
    char c = '\0';
    size_t count = 0;
    DWORD numRead = 1;

    while (
            count != maxBytes
            && numRead
            && !( c == '\r' || c == '\n' )
        )
    {
        ReadFile( m_port, static_cast<LPVOID>( buffer ), 1, &numRead, 0 );
        c = *buffer;
        count += numRead;
        buffer += numRead;
    }

    if (count)
    {
        *(buffer - 1) = '\0';
    }

    return count;
}

/**
    Flushes the read buffer - reads all available bytes from read buffer and discards them.
**/
void SerialPort::Flush()
{
    char c;
    size_t n = 0;
    do
    {
        n = Read( &c, 1 );
    }
    while ( n != 0 );
}

} // end namepsace GLK

