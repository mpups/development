/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_CAPTURE_H__
#define __LIB_AV_CAPTURE_H__

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "Capture.h"

/**
    Class which uses libavcodec to read video streams from a video file.
*/
class LibAvCapture : public Capture
{
public:
    LibAvCapture( const char* videoFile );
    virtual ~LibAvCapture();

    bool IsOpen() const;
    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    uint64_t GetFrameTimestamp() const;

    void ExtractLuminanceImage( uint8_t* data, int stride );
    void ExtractRgbImage( uint8_t* data, int stride );
    void ExtractBgrImage( uint8_t* data, int stride );

    static void InitLibAvCodec();

protected:
    void FrameConversion( PixelFormat format, uint8_t* data, int stride );

private:
    AVFormatContext* m_formatContext;
    AVCodecContext*  m_codecContext;
    AVCodec*         m_codec;
    AVFrame*         m_avFrame;
    int              m_videoStream;
    AVPacket         m_packet;
    SwsContext*      m_imageConversionContext;
    bool m_open;
};

#endif /* __LIB_AV_CAPTURE_H__ */

