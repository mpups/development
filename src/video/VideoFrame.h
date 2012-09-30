/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __VIDEO_FRAME_H__
#define __VIDEO_FRAME_H__

extern "C" {
#include <libavformat/avformat.h>
}

/**
    Holds or wraps video frame data.
*/
class VideoFrame
{
public:
    VideoFrame( PixelFormat format, uint32_t width, uint32_t height );
    VideoFrame( uint8_t* buffer, PixelFormat format, uint32_t width, uint32_t height, uint32_t stride );
    virtual ~VideoFrame();

    AVPicture& GetAvPicture();
    const AVPicture& GetAvPicture() const;

    void FillAvFramePointers( AVFrame& frame ) const;

    int GetWidth() const;
    int GetHeight() const;
    PixelFormat GetAvPixelFormat() const;

private:
    AVPicture   m_picture;
    PixelFormat m_format;
    int         m_width;
    int         m_height;
    const bool m_freePicture;
};

#endif /* __VIDEO_FRAME_H__ */

