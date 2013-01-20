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
    PacketDemuxer* m_demuxer;
    PacketMuxer*   m_muxer;

    SimpleQueue m_avPackets;
    int m_packetOffset;

    std::unique_ptr<FFMpegStdFunctionIO> m_videoIO;
    std::unique_ptr<LibAvCapture> m_streamer;

    Joystick m_joystick;
    uint8_t* m_imageBuffer;

#ifndef ARM_BUILD
    GLK::ImageWindow            m_display;  // Used to display images received from robot.
    GLK::ImageWindow::ImageData m_postData; // This struct describes the image data we post for display.
#endif // ARM_BUILD
};

#endif // ROBOTCLIENT_H
