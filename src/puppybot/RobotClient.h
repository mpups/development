#ifndef ROBOTCLIENT_H
#define ROBOTCLIENT_H

#include <memory>

#include "../../include/RoboLib.h"

#ifndef ARM_BUILD
#include <glk.h>
#endif

class RobotClient
{
public:
    RobotClient();
    virtual ~RobotClient();

    bool Connect( const char* host, int port );

    bool RunCommsLoop();

protected:
    bool InitialiseVideoStream();
    bool ReceiveVideoFrame();
    void SendJoystickData();

private:
    TcpSocket m_client;
    Joystick  m_joystick;

    std::unique_ptr<FFMpegSocketIO> m_videoIO;
    std::unique_ptr<LibAvCapture> m_streamer;

    uint8_t* m_imageBuffer;

#ifndef ARM_BUILD
    GLK::ImageWindow            m_display;  // Used to display images received from robot.
    GLK::ImageWindow::ImageData m_postData; // This struct describes the image data we post for display.
#endif // ARM_BUILD
};

#endif // ROBOTCLIENT_H
