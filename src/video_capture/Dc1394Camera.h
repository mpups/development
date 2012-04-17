// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef DC1394_CAMERA_H
#define DC1394_CAMERA_H

#include <stdint.h>

extern "C" {
#include <dc1394/dc1394.h>
}

#include "CameraCapture.h"

/**
    Class for accessing a fire-wire camera through libdc1394 (version 2).

    Dc1394Cameras are statically counted. This is so that only one video bus is
    opened and shared - it is opened when the first Dc1394Camera is constructed
    and closed when the last is deleted.
**/
class Dc1394Camera : public CameraCapture
{
public:
    Dc1394Camera();
    ~Dc1394Camera();

    bool IsOpen() const;
    void StartCapture();
    void StopCapture();

    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    uint64_t GetFrameTimestamp() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t*, int stride );
    void ExtractRgbImage( uint8_t* img, int stride );
    void ExtractBgrImage( uint8_t*, int stride );

private:
    bool OpenDevice();

    const static int NUM_BUFFERED_FRAMES = 4;    

    static int32_t       ms_objectCount;
    static dc1394_t*     ms_video;

    dc1394camera_t*      m_camera;
    dc1394video_frame_t* m_frame;
    
    uint64_t    m_guid;

    bool m_transmitting;
    bool m_captured;
};

#endif // DC1394_CAMERA_H

