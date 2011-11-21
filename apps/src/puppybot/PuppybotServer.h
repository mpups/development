#ifndef __PUPPYBOT_SERVER_H__
#define __PUPPYBOT_SERVER_H__

class PuppybotServer
{
public:
    PuppybotServer ();
    virtual ~PuppybotServer ();

    void Listen();
    void RunCommsLoop();

private:
    
};

#endif /* __PUPPYBOT_SERVER_H__ */

