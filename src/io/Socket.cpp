#include "Socket.h"

#include <assert.h>

#include <unistd.h>
#include <errno.h>

/**
    Create and open a socket of the specified type.
**/
Socket::Socket()
{
    m_socket = socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 );
    assert( m_socket != -1 );
}

/**
    @return True if the connection was successful.
**/
bool Socket::Connect()
{
    sockaddr addr;
    
    int err = connect( m_socket, &addr, sizeof(sockaddr) );
    assert( err != -1 );
    
    return err != -1;
}

/**
    Asserts.

    @return False if message could not be sent.
**/
bool Socket::Send( char* message, int size )
{
    int err = send( m_socket, message, size, 0 );
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
