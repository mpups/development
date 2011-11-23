#ifndef __PUPPYBOT_CLIENT_H__
#define __PUPPYBOT_CLIENT_H__

class TcpSocket;
class Ipv4Address;

class PuppybotClient
{
public:
    PuppybotClient (  );
    virtual ~PuppybotClient ();

    bool Connect( const Ipv4Address& );
    void RunCommsLoop();

private:
    TcpSocket*   m_server;
    Ipv4Address* m_addr;
};

#endif /* __PUPPYBOT_CLIENT_H__ */

