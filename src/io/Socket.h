#ifndef GLK_SOCKET_H
#define GLK_SOCKET_H

#ifdef __linux

#include <sys/types.h>
#include <sys/socket.h>

#endif

/**
    Wrapper object for sockets API.

    @todo Socket error messages should say 'Warning' not 'error'

    @todo TcpSocket should be derived from this base class instead of embedded in it.
**/
class Socket
{
public:
    Socket();
    virtual ~Socket();

    bool IsValid() const;

    bool Bind( int );
    void Shutdown();
    bool Connect( const char*, int );

    int Read( char* message, size_t maxBytes );
    int Write( const char* message, size_t size );

    void SetBlocking( bool );

protected:
    int m_socket;

private:

};

#endif // GLK_SOCKET_H

