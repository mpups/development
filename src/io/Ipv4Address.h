#ifndef __IPV4_ADDRESS_H__
#define __IPV4_ADDRESS_H__

#include <netinet/in.h>

/**
    Class for storing and manipulating IPv4 addresses.
**/
class Ipv4Address
{
    friend class Socket;
    friend class UdpSocket;

public:
    Ipv4Address();
    Ipv4Address( const char* hostname, int portNumber );
    Ipv4Address( const Ipv4Address& );
    virtual ~Ipv4Address();

    bool IsValid() const;

private:
    sockaddr_in m_addr;

    void GetHostByName( const char* hostname, int portNumber );

    const sockaddr_in* const Get_sockaddr_in_Ptr() const;
};

#endif /* __IPV4_ADDRESS_H__ */

