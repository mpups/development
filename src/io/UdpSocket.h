#ifndef GLK_UDP_SOCKET_H
#define GLK_UDP_SOCKET_H

#include "Socket.h"


/**
    Class for creating UDP (datagram) sockets.

    inherits from the base class Socket which provides the generic socket functionality.

    This allows datagrams to be sent via a connection with Read() and Write() or
    to a specific host and port using SendTo().
**/
class UdpSocket : public Socket
{
public:
    UdpSocket ();
    virtual ~UdpSocket ();

    int SendTo( const char* hostname, int portNumber, const char* message, size_t size );
};

#endif /* GLK_UDP_SOCKET_H */

