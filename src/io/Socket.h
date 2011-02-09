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

    bool IsValid() const { return m_socket >= 0; };

    bool Bind( int );
    bool Listen( int );
    Socket* Accept();
    void Shutdown();
    bool Connect( char*, int );
            
    int Read( char* message, size_t maxBytes );
    int Write( char* message, size_t size );

protected:
    

private:
    int m_socket;
    
    explicit Socket( int socket );
};

#endif // GLK_SOCKET_H

