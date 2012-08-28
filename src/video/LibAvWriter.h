/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_WRITER_H__
#define __LIB_AV_WRITER_H__

#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "FrameConverter.h"

class LibAvVideoStream;
class FFMpegCustomIO;

/**
    Class which uses libavcodec to write video streams to a video file.
*/
class LibAvWriter
{
private:
    void Init();

public:
    LibAvWriter( const char* videoFile );
    LibAvWriter( FFMpegCustomIO& customIO );
    virtual ~LibAvWriter();

    bool IsOpen() const;

    bool AddVideoStream( uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc );

    bool PutGreyFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride );
    bool PutRgbFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride );
    bool PutBgrFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride );

    bool PutYUYV422Frame( uint8_t* buffer, uint32_t width, uint32_t height );
    bool PutYUV420PFrame( uint8_t* buffer, uint32_t width, uint32_t height );

    static int32_t FourCc( char, char, char, char );

protected:
    bool PutFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride, PixelFormat format );
    bool WriteCodecFrame();

private:
    static const int BUFFER_SIZE = FF_MIN_BUFFER_SIZE+(640*480*4);

    AVOutputFormat*  m_outputFormat;
    AVFormatContext* m_formatContext;
    FFMpegCustomIO*    m_customIO;
    AVFrame          m_codecFrame;

    LibAvVideoStream* m_stream;
    FrameConverter   m_converter;

    bool    m_open;

public:
    // For benchmarking only:
    double lastConvertTime_ms;
    double lastEncodeTime_ms;
    double lastPacketWriteTime_ms;
};

#endif /* __LIB_AV_WRITER_H__ */

