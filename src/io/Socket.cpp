#include "Socket.h"

#include <assert.h>

#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <fcntl.h>

/**
    Create and open a TCP socket (SOCK_STREAM).
**/
Socket::Socket()
{
    m_socket = socket( AF_INET, SOCK_STREAM, 0 );
    assert( m_socket != -1 );
}

/**
    Internal private constructor for creating socket deirectly from a specified file descriptor.

    The specified socket will be closed in the destructor of this object.

    @param [in] socket A valid file descriptor that must refer to a socket.
**/
Socket::Socket( int socket )
:
    m_socket (socket)
{
    assert( m_socket != -1 );
}

/**
    Closes the socket connection.
**/
Socket::~Socket()
{
    if ( m_socket != -1 )
    {
        close( m_socket );
    }
}

bool Socket::IsValid() const
{
    return m_socket >= 0;
}

/**
    Bind this socket to the specified port-number on any INET address.
**/
bool Socket::Bind( int portNumber )
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

/**
    Listen for connections.
    
    @param queueSize Max number of pending connections.
**/
bool Socket::Listen( int queueSize )
{
    int err = listen( m_socket, queueSize );
    return err == 0;
}

/**
    Accept a connection from a bound socket.
    
    This socket must be bound or null will be returned.
    
    @return New client socket connection - or null on error or timeout.
**/
Socket* Socket::Accept()
{
    Socket* newClient;
    int socket;    
    setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, &socket, sizeof(int) );
    
    struct sockaddr_in addr;
    memset( (void*)&addr, 0, sizeof(sockaddr_in) );
    
    socklen_t clientSize = sizeof(addr);
    socket = accept( m_socket, (struct sockaddr*)&addr, &clientSize );
    if ( socket != -1 )
    {
        newClient = new Socket( socket );
    }
    else
    {
        newClient = 0;
    }
    
    return newClient;
}

/**
    Shutdown this socket's connections.
**/
void Socket::Shutdown()
{
    int err = shutdown( m_socket, SHUT_RDWR );
    if ( err == -1 )
    {
        fprintf( stderr, "Error on socket shutdown: %s\n", strerror(errno) );           
    }
}

/**
    Connect this socket to a server at specified address.

    @return True if the connection was successful.
**/
bool Socket::Connect( const char* hostname, int portNumber )
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

/**
    Blocking read from this socket.
    
    In the case of non-blocking IO Read returns 0 if no bytes were
    immediately available.
    
    @param message Storage for received bytes.
    @return Number of bytes read or -1 if there was an error.
**/
int Socket::Read( char* message, size_t maxBytes )
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

/**
    In the case of non-blocking IO Write returns 0 if the write would have
    caused the process to block.

    @return Number of bytes written or -1 if there was an error.
**/
int Socket::Write( const char* message, size_t size )
{
    int n = send( m_socket, message, size, MSG_NOSIGNAL );
    if ( n == -1 && errno == EAGAIN )
    {
        n = 0;
    }
    
    return n;
}

void Socket::SetNagleBufferingOn()
{
    int flag = 0;
    int result = setsockopt( m_socket,           // socket affected
                            IPPROTO_TCP,    // set option at TCP level
                            TCP_NODELAY,    // name of option
                            (char *) &flag, // the cast is historical cruft
                            sizeof(int)     // length of option value
                           );
    if ( result < 0 )
    {
        fprintf( stderr, "SetSocketOpt failed: %s\n", strerror(errno) );        
    }    
}

void Socket::SetNagleBufferingOff()
{
    int flag = 1;
    int result = setsockopt( m_socket,           // socket affected
                            IPPROTO_TCP,    // set option at TCP level
                            TCP_NODELAY,    // name of option
                            (char *) &flag, // the cast is historical cruft
                            sizeof(int)     // length of option value
                           );
    if ( result < 0 )
    {
        fprintf( stderr, "SetSocketOpt failed: %s\n", strerror(errno) );        
    }      
}

/**
    @param block True set socket to blocking mode, false sets socket to non-blocking.
**/
void Socket::SetBlocking( bool block )
{
    if ( block )
    {
        fcntl( m_socket, F_SETFL, O_ASYNC );
    }
    else
    {
        fcntl( m_socket, F_SETFL, O_NONBLOCK );
    }
}

    
