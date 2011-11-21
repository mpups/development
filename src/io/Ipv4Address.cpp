#include "Ipv4Address.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>

#include <netdb.h>

/**
    Construct an uninitialised address - IsValid() shall return false.
**/
Ipv4Address::Ipv4Address()
{
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>( &m_addr );
    addr->sin_family = AF_INET - 1;
}

/**
    Construct an address from the specified hostname and port number.

    If the hostname is resolved successfully then this object will hold
    the address of the specified host (and IsValid() will return true).
    If the hostname cannot be resolved then IsValid() will return false.
*/
Ipv4Address::Ipv4Address( const char* hostname, int portNumber )
{
    GetHostByName( hostname, portNumber );
}

/**
    Copy constructor - performs a memcpy on the underlying addresss structure.
*/
Ipv4Address::Ipv4Address( const Ipv4Address& other )
{
    memcpy( &m_addr, &other.m_addr, sizeof( m_addr ) );
}

/**
    Nothing to clean up.
*/
Ipv4Address::~Ipv4Address()
{
    
}

/**
    @return true if the current address is a valid IPv4 address, false otherwise.
*/
bool Ipv4Address::IsValid() const
{
    return m_addr.ss_family == AF_INET;
}

/**
    Resolve the host name from this Ipv4 address.

    The address must already be known to be valid or the string "(invalid address)" is returned.
*/
void Ipv4Address::GetHostName( char* host, size_t length )
{
    sockaddr* addr = reinterpret_cast<sockaddr*>(&m_addr);
    int err = getnameinfo( addr, sizeof(m_addr), host, length, 0, 0, 0 );

    if ( err != 0 )
    {
        snprintf( host, length, "(Invalid address)" );
    }
}

/**
    Resolve an address from a host name and port number.

    If successful then this object will hold the address of the specified host (and IsValid() will return true).

    @todo gethostbyname is obsolete - should use getaddrinfo
*/
void Ipv4Address::GetHostByName( const char* hostname, int portNumber )
{
    memset( (void*)&m_addr, 0, sizeof(sockaddr_storage) );
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&m_addr);

    struct hostent* host = gethostbyname( hostname );

    if ( host == 0 )
    {
        addr->sin_family = AF_INET - 1;
    }
    else
    {
        addr->sin_family = AF_INET;
        memcpy( &addr->sin_addr.s_addr, host->h_addr, host->h_length );
        addr->sin_port = htons( portNumber );
    }
}

/**
    Private member for friend classes that need direct access to the sockaddr_in structure
    (e.g. Socket objects).
**/
const sockaddr_in* const Ipv4Address::Get_sockaddr_in_Ptr() const
{
    return reinterpret_cast<const sockaddr_in*>(&m_addr);
}

/**
    Private member for friend classes that need direct access to the sockaddr_storage structure
    (e.g. Socket objects).
*/
const sockaddr_storage* const Ipv4Address::Get_sockaddr_storage_Ptr() const
{
    return &m_addr;
}
