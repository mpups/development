#include "UdpSocket.h"

#include <assert.h>

#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <fcntl.h>

UdpSocket::UdpSocket ()
:
    m_socket (-1)
{
    m_socket = socket( AF_INET, SOCK_DGRAM, 0 );
    assert( m_socket != -1 );
}

UdpSocket::~UdpSocket ()
{
    if ( m_socket != -1 )
    {
        close( m_socket );
    }
}


bool UdpSocket::IsValid() const
{
    return m_socket >= 0;
}

bool UdpSocket::Bind( int portNumber )
{
    struct sockaddr_in addr;
    memset( (void*)&addr, 0, sizeof(sockaddr_in) );
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    addr.sin_port        = htons( portNumber );
    
    int err = bind( m_socket, (struct sockaddr*)&addr, sizeof(addr) );
    if ( err == -1 )
    {
        fprintf( stderr, "%s\n", strerror(errno) );
    }
    
    return err != -1;
}

bool UdpSocket::Connect( const char* hostname, int portNumber )
{
    struct hostent* server;
    server = gethostbyname( hostname );
    if ( server == 0 )
    {
        return false;
    }

    fprintf( stderr, "Connecting to: %s:%d\n", hostname, portNumber );

    struct sockaddr_in addr;
    memset( &addr, 0, sizeof(sockaddr_in) );
    addr.sin_family = AF_INET;
    memcpy( &addr.sin_addr.s_addr, server->h_addr, server->h_length );
    addr.sin_port = htons( portNumber );

    int err = connect( m_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in) );
    if ( err == -1 )
    {
        fprintf( stderr, "%s\n", strerror(errno) );
    }
    
    return err != -1;
}

void UdpSocket::Shutdown()
{
    int err = shutdown( m_socket, SHUT_RDWR );
    if ( err == -1 )
    {
        fprintf( stderr, "Error on socket shutdown: %s\n", strerror(errno) );
    }
}

int UdpSocket::Read( char* message, size_t maxBytes )
{
    int n = read( m_socket, message, maxBytes );
    if ( n == -1 && errno == EAGAIN )
    {
        // For non-blocking IO just return zero bytes were available.
        // Otherwise we return the error (-1)
        n = 0;
    }
    
    return n;
}

int  UdpSocket::Write( const char* message, size_t size )
{
    int n = send( m_socket, message, size, MSG_NOSIGNAL );
    if ( n == -1 && errno == EAGAIN )
    {
        n = 0;
    }
    
    return n;
}

int UdpSocket::SendTo( const char* hostname, int portNumber, const char* message, size_t size )
{
    struct hostent* server;
    server = gethostbyname( hostname );
    if ( server == 0 )
    {
        return 0;
    }
    
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof(sockaddr_in) );
    addr.sin_family = AF_INET;
    memcpy( &addr.sin_addr.s_addr, server->h_addr, server->h_length );
    addr.sin_port = htons( portNumber );

    int n = sendto( m_socket, message, size, MSG_NOSIGNAL, (struct sockaddr*)&addr, sizeof(struct sockaddr_in) );
    if ( n == -1 && errno == EAGAIN )
    {
        n = 0;
    }
    
    return n;
}

