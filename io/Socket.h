#ifndef GLK_SOCKET_H
#define GLK_SOCKET_H

#ifdef __linux

#include <sys/types.h>
#include <sys/socket.h>

#endif

/**
    Wrapper object for sockets API.
    
    
**/
class Socket
{
public:
    Socket();
    ~Socket();

    bool Connect();
    
    bool Send( char* message, int size );

private:
    int m_socket;
};

#endif // GLK_SOCKET_H

