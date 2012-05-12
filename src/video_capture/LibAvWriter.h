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

/**
    Class which uses libavcodec to write video streams to a video file.
*/
class LibAvWriter
{
public:
    LibAvWriter( const char* videoFile );
    virtual ~LibAvWriter();

    bool IsOpen() const;

    bool AddVideoStream( uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc );

    bool PutGreyFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride );

    static int32_t FourCc( char, char, char, char );
    static PixelFormat ChooseCodecFormat( CodecID id, PixelFormat inputFormat );

protected:
    void WriteCodecFrame();

private:
    AVOutputFormat*  m_outputFormat;
    AVFormatContext* m_formatContext;
    AVFrame          m_codecFrame;

    LibAvVideoStream* m_stream;
    FrameConverter   m_converter;
    uint8_t*         m_encodingBuffer;

    bool    m_open;
};

#endif /* __LIB_AV_WRITER_H__ */

