#ifndef GLK_SOCKET_H
#define GLK_SOCKET_H

#ifdef __linux

#include <sys/types.h>
#include <sys/socket.h>

#endif

/**
    Wrapper object for sockets API.

    @todo Socket error messages should say 'Warning' not 'error'
**/
class Socket
{
public:
    Socket();
    ~Socket();

    bool IsValid() const;

    bool Bind( int );
    bool Listen( int );
    Socket* Accept();
    void Shutdown();
    bool Connect( const char*, int );

    int Read( char* message, size_t maxBytes );
    int Write( const char* message, size_t size );

    void SetNagleBufferingOn();
    void SetNagleBufferingOff();
    void SetBlocking( bool );

protected:

private:
    int m_socket;
    
    explicit Socket( int socket );
};

#endif // GLK_SOCKET_H

