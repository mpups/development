#include "LibAvVideoStream.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <assert.h>

#include "LibAvWriter.h"

/**
    
*/
LibAvVideoStream::LibAvVideoStream( AVFormatContext* context, uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc )
:
    m_stream    (0)
{
    const AVCodecTag *tags[] = { avformat_get_riff_video_tags(), 0 };
    CodecID codecId = av_codec_get_id( tags, fourcc );

    m_codec = avcodec_find_encoder( codecId );
    m_stream = avformat_new_stream( context, m_codec );
    if ( m_stream != 0 )
    {
        CodecContext()->codec_id  = codecId;
        CodecContext()->codec_tag = fourcc;
        CodecContext()->pix_fmt = LibAvWriter::ChooseCodecFormat( codecId, PIX_FMT_GRAY8 ); //@todo hard coded to grey-scale input at moment

        assert( width%2 == 0 );
        assert( height%2 == 0 );
        CodecContext()->width = width;
        CodecContext()->height = height;
        CodecContext()->time_base.num = 1;
        if ( fps == 0) { fps = 1; }
        CodecContext()->time_base.den = fps;
    }
}

LibAvVideoStream::~LibAvVideoStream()
{
    avcodec_close( m_stream->codec );
}

bool LibAvVideoStream::IsValid() const
{
    return m_stream != 0;
}

AVCodecContext* LibAvVideoStream::CodecContext()
{
    return m_stream->codec;
}

AVCodec* LibAvVideoStream::Codec()
{
    return m_codec;
}

int LibAvVideoStream::Index() const
{
    return m_stream->index;
}



