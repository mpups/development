#ifndef GLK_UDP_SOCKET_H
#define GLK_UDP_SOCKET_H

#ifdef __linux

#include <sys/types.h>
#include <sys/socket.h>

#endif

/**
    Class for creating UDP (datagram) sockets.

    This allows datagrams to be sent via a connection with Read() and Write() or
    to a specific host and port using SendTo().
**/
class UdpSocket
{
public:
    UdpSocket ();
    virtual ~UdpSocket ();

    bool IsValid    () const;
    bool Bind       ( int );
    bool Connect    ( const char*, int );
    void Shutdown   ();
    int  Read       ( char* message, size_t maxBytes );
    int  Write      ( const char* message, size_t size );

    int SendTo( const char* hostname, int portNumber, const char* message, size_t size );

private:
    int m_socket;
};

#endif /* GLK_UDP_SOCKET_H */

