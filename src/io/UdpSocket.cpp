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

#include "Ipv4Address.h"


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
    Send a datagram to an unconnected socket at the specified IPv4 address.

    @param [in] addr A valid IPv4 address you want to send to.
    @param [in] Pointer to data you want to send.
    @param [in] size Length of message to send in bytes.

    It is an error to supply an invalid address object and -1 is returned in this case.

    @return Number of bytes sent or -1 on error. If the call would block and the socket is non-blocking 0 can be returned.
**/
int UdpSocket::SendTo( const Ipv4Address& addr, const char* message, size_t size )
{
    if ( addr.IsValid() == false )
    {
        return -1;
    }

    int n = sendto( m_socket, message, size, MSG_NOSIGNAL, (struct sockaddr*)addr.Get_sockaddr_in_Ptr(), sizeof(struct sockaddr_in) );
    if ( n == -1 && errno == EAGAIN )
    {
        n = 0;
    }
    
    return n;
}

