#ifndef __PUPPYBOT_SERVER_H__
#define __PUPPYBOT_SERVER_H__

#include <RoboLib.h>

class PuppybotServer
{
public:
    PuppybotServer ( uint16_t port );
    virtual ~PuppybotServer ();

    void Listen();
    const Ipv4Address& GetClientAddress();
    void RunCommsLoop();

private:
    Ipv4Address m_clientAddress;
    TcpSocket*  m_server;
    TcpSocket*  m_con;
    UdpSocket*  m_udpChannel;
    uint16_t    m_port;

    void PostConnectionSetup();
    void SendHandshakePacket();
};

#endif /* __PUPPYBOT_SERVER_H__ */

