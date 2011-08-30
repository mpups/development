#include "Socket.h"

#include <assert.h>

#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>

#include <netinet/tcp.h>
#include <fcntl.h>

#include "Ipv4Address.h"

/**
    Initialises the internal socket to an invalid value.
**/
Socket::Socket()
:
    m_socket (-1)
{
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
    Ipv4Address addr( hostname, portNumber );

    if ( addr.IsValid() == false )
    {
        return false;
    }

    int err = connect( m_socket, (struct sockaddr*)addr.Get_sockaddr_in_Ptr(), sizeof(struct sockaddr_in) );
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

    
