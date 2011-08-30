#include "Ipv4Address.h"

#include <assert.h>
#include <memory.h>

#include <netdb.h>

/**
    Construct an uninitialised address - IsValid() shall return false.
**/
Ipv4Address::Ipv4Address()
{
    m_addr.sin_family = AF_INET - 1;
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
    return m_addr.sin_family == AF_INET;
}

/**
    Resolve an address from a host name and port number.

    If successful then this object will hold the address of the specified host (and IsValid() will return true).
*/
void Ipv4Address::GetHostByName( const char* hostname, int portNumber )
{
    memset( (void*)&m_addr, 0, sizeof(sockaddr_in) );

    struct hostent* host = gethostbyname( hostname );

    if ( host == 0 )
    {
        m_addr.sin_family = AF_INET - 1;
    }
    else
    {
        m_addr.sin_family = AF_INET;
        memcpy( &m_addr.sin_addr.s_addr, host->h_addr, host->h_length );
        m_addr.sin_port = htons( portNumber );
    }
}

/**
    Private member for friend classes that need direct access to the sockaddr_in structure
    (e.g. Socket objects).
**/
const sockaddr_in* const Ipv4Address::Get_sockaddr_in_Ptr() const
{
    return &m_addr;
}
