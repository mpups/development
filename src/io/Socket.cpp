#include "Socket.h"

#include <assert.h>

#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netdb.h> 

/**
    Create and open a socket of the specified type.
**/
Socket::Socket()
{
    m_socket = socket( AF_INET, SOCK_STREAM, 0 );
    assert( m_socket != -1 );
}


/**
    Internal provate utility for creating socket with specific filedescriptor id.
**/
Socket::Socket( int socket )
:
    m_socket (socket)
{
}

/**
    Bind this socket to the specified port-number on any INET address.
**/
bool Socket::Bind( int portNumber )
{
    struct sockaddr_in addr;
    memset( (void*)&addr, 0, sizeof(sockaddr_in) );
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
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
    @param queueSize Max number of established connections.
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
    
//    setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, &socket, sizeof(int) );
    
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
    assert( err == 0 );
}

/**
    Connect this socket to a server at specified address.

    @return True if the connection was successful.
**/
bool Socket::Connect( char* hostname, int portNumber )
{
    struct hostent* server;
    server = gethostbyname( hostname );

    struct sockaddr_in addr;
    memset( (void*)&addr, 0, sizeof(sockaddr_in) );
    
    memcpy( (void*)server->h_addr, (void*)&addr.sin_addr.s_addr, server->h_length );
    addr.sin_port = htons( portNumber );
            
    int err = connect( m_socket, (sockaddr*)&addr, sizeof(sockaddr) );
    assert( err == 0 );
    
    return err != -1;
}


/**
    Blocking read from this socket.
    
    @param message Storage for received bytes.
    @return Number of bytes read.
**/
int Socket::Read( char* message, size_t maxBytes )
{
    int n = read( m_socket, message, maxBytes );
    if ( n < 0 )
    {
        fprintf( stderr, "%s\n", strerror(errno) );        
    }
    
    return n;
}

/**
    Asserts.

    @return False if message could not be sent.
**/
bool Socket::Write( char* message, size_t size )
{
    int err = write( m_socket, message, size );
    if ( err == -1 && errno != EAGAIN )
    {
        assert( 0 );
    }
    
    return err != -1;
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
