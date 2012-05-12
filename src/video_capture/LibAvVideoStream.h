/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_VIDEO_STREAM_H__
#define __LIB_AV_VIDEO_STREAM_H__

#include <stdint.h>

struct AVStream;
struct AVFormatContext;
struct AVCodec;
struct AVCodecContext;

class LibAvVideoStream
{
public:
    LibAvVideoStream( AVFormatContext* context, uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc );
    virtual ~LibAvVideoStream();

    bool IsValid() const;

    AVCodecContext* CodecContext();
    AVCodec* Codec();

    int Index() const;

private:
    AVStream* m_stream;
    AVCodec*  m_codec;
};


#endif /* __LIB_AV_VIDEO_STREAM_H__ */

