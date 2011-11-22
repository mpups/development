#ifndef __PUPPYBOT_SERVER_H__
#define __PUPPYBOT_SERVER_H__

#include <RoboLib.h>

class PuppybotServer
{
public:
    PuppybotServer ( int port );
    virtual ~PuppybotServer ();

    void Listen();
    const Ipv4Address& GetClientAddress();
    void RunCommsLoop();

private:
    Ipv4Address m_clientAddress;
    TcpSocket*  m_server;
    TcpSocket*  m_con;
};

#endif /* __PUPPYBOT_SERVER_H__ */

