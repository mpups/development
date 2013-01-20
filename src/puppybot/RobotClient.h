#ifndef ROBOTCLIENT_H
#define ROBOTCLIENT_H

#include <memory>
#include <queue>

#include "../../include/RoboLib.h"

#include <glkcore.h>

#ifndef ARM_BUILD
#include <glk.h>
#endif

class PacketDemuxer;
class PacketMuxer;
class VideoClient;

class RobotClient
{
public:
    RobotClient();
    virtual ~RobotClient();

    bool Connect( const char* host, int port );

    bool RunCommsLoop();

protected:
    void SendJoystickData();
    void SetupImagePostData( int w, int h );

    bool InitialiseVideoStream();
    bool ReceiveVideoFrame();
    int FfmpegReadPacket( uint8_t* buffer, int size );

private:
    TcpSocket     m_client;
    std::unique_ptr<PacketDemuxer> m_demuxer;
    std::unique_ptr<PacketMuxer>   m_muxer;

    std::unique_ptr<VideoClient> m_videoClient;

    Joystick m_joystick;
    uint8_t* m_imageBuffer;

#ifndef ARM_BUILD
    GLK::ImageWindow            m_display;  // Used to display images received from robot.
    GLK::ImageWindow::ImageData m_postData; // This struct describes the image data we post for display.
#endif // ARM_BUILD
};

#endif // ROBOTCLIENT_H
