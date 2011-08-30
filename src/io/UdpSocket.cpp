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

/**
    Send a datagram to an unconnected socket at the specified address (hostaname and port-number).

    @param [in] hostname String containing the name or IP address you want to send to.
    @param [in] portNumber Port-number you want to send to.
    @param [in] Pointer to data you want to send.
    @param [in] size Length of message to send in bytes.

    @return Number of bytes sent or -1 on error. If the call would block and the socket is non-blocking 0 can be returned.
**/
int UdpSocket::SendTo( const char* hostname, int portNumber, const char* message, size_t size )
{
    struct hostent* server;
    server = gethostbyname( hostname );
    if ( server == 0 )
    {
        return -1;
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

