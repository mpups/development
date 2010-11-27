#include "SerialPort.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

/**
    Opens the specified device as a serial port.

    No check is performed to see if the device is actually a serial port.

    @param portName Name of serial port.
**/
SerialPort::SerialPort( const char* portName )
{
    m_port = open( portName, O_RDWR | O_NONBLOCK );
    
    if ( m_port > 0 )
    {
        // currently force the baud rate for motion-mind compatibility
        struct termios t;
        int err = tcgetattr( m_port, &t );
        assert( err == 0 );

        err = cfsetispeed( &t, B19200 );
        assert( err == 0 );
        err = cfsetospeed( &t, B19200 );
        assert( err == 0 );
        cfmakeraw( &t );
        err = tcsetattr( m_port, 0,  &t );
        assert( err == 0 );
    }
}

SerialPort::~SerialPort()
{
    if ( m_port != -1 )
    {
        close( m_port );
    }
}

/**
    @return Number of bytes actually written.
**/
size_t SerialPort::Write( const char* buffer, size_t numBytes )
{
    int n = write( m_port, buffer, numBytes );
    assert( n >= 0 );
    return n;
}

int SerialPort::Printf( const char* formatString, ... )
{
    size_t numWritten;
    va_list args;
    va_start( args, formatString );

    int numChars = 1+vsnprintf( 0, 0, formatString, args );
    char* buf = new char[numChars];
    
    int numPrinted = vsnprintf( buf, numChars, formatString, args );
    assert( numPrinted == numChars-1 );

    numWritten = write( m_port, buf, numChars );

    delete [] buf;
    va_end( args );

    return numWritten;
}
        
/**
    Read a number of bytes from the serial port.

    Whether the call blocks if not enough bytes are
    available or returns with the bytes that are available
    depends on the serial port settings.

    @param buffer Pointer to storage for bytes read.
    @param NumBytes Number of bytes to read.
    @return Number of bytes actually read.
**/
size_t SerialPort::Read( char* buffer, size_t numBytes )
{
    int n = read( m_port, buffer, numBytes );
    
    if ( n < 0 )
    {
        assert( errno == EAGAIN );
        // Only acceptable error is EAGAIN so return 0 bytes read
        // and caller will have to try again.
        n = 0;
    }
    return n;
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
    size_t numRead = 1;

    while (
            count != maxBytes
            && numRead
            && !( c == '\r' || c == '\n' )
        )
    {
        int n = read( m_port, buffer, maxBytes );
        if ( n >= 0 )
        {
            c = *buffer;
            count += numRead;
            buffer += numRead;
        }
        else
        {
            assert( errno == EAGAIN ); // Only acceptable error is that there were no bytes available.
        }   
    }

    if ( count )
    {
        *(buffer - 1) = '\0';
    }

    return count;
}

/**
    
**/
void SerialPort::Flush()
{
    char c;
    size_t n;
    do
    {
        n = Read( &c, 1 );
    } while (n);
}

