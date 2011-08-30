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

/**
    Initialise the base class with an invlaid socket ID then create
    a datagram socket.
**/
UdpSocket::UdpSocket ()
:
    Socket (-1)
{
    m_socket = socket( AF_INET, SOCK_DGRAM, 0 );
    assert( m_socket != -1 );
}

/**
    Nothing to do, the socket is closed in the base class' destructor.
**/
UdpSocket::~UdpSocket ()
{
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

