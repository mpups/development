#ifndef __PUPPYBOT_CLIENT_H__
#define __PUPPYBOT_CLIENT_H__

#include <RoboLib.h>

class TcpSocket;

class PuppybotClient
{
public:
    PuppybotClient (  );
    virtual ~PuppybotClient ();

    bool Connect( const Ipv4Address& );
    const Ipv4Address& GetServerAddress();
    void RunCommsLoop();

private:
    Ipv4Address m_addr;
    TcpSocket*  m_server;
    UdpSocket*  m_udpChannel;

    void PostConnectionSetup();
};

#endif /* __PUPPYBOT_CLIENT_H__ */

