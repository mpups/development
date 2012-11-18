#include "LibAvCapture.h"

#include "FFmpegCustomIO.h"

#include <assert.h>

#include <iostream>
#include <stdint.h>

extern "C" {
#include <libavutil/mathematics.h>
}

/**
    Static member to register all codecs with libav.
    This is not thread safe.
*/
void LibAvCapture::InitLibAvCodec()
{
    static int called = 0;
    if ( called == 0 )
    {
        called = 1;
        av_register_all();
    }
}

/**
    Initialisation common to all constructors.
    
    Should only be called from within a constructor.

    @param streamName usually a filename or URL.
*/
void LibAvCapture::Init( const char* streamName )
{
    InitLibAvCodec();

    m_formatContext = avformat_alloc_context();

    if ( m_customIO != 0 )
    {
        m_formatContext->pb = m_customIO->GetAVIOContext();
    }

    AVInputFormat* inputFormat = av_find_input_format( "m4v" );

    m_open = ( avformat_open_input( &m_formatContext, streamName, inputFormat, 0 ) >= 0 );

    if ( m_open == false )
    {
        return;
    }

    int foundStreamInfo = avformat_find_stream_info( m_formatContext, 0 );

    if ( foundStreamInfo == -1)
    {
        m_open = false;
        return;
    }

    // Find the first video stream
    m_videoStream = -1;
    for(unsigned int i=0; i<m_formatContext->nb_streams; i++)
    {
        if(m_formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            m_videoStream = i;
            break;
        }
    }

    if ( m_videoStream == -1)
    {
        m_open = false;
        return;
    }

    av_dump_format( m_formatContext, m_videoStream, m_formatContext->filename, 0 );

    // Get a pointer to the codec context for the video stream
    m_codecContext = m_formatContext->streams[m_videoStream]->codec;

    // Find the decoder for the video stream
    m_codec = avcodec_find_decoder( m_codecContext->codec_id );
    if( m_codec == 0 )
    {
        m_open = false;
        return;
    }

    // Open codec
    if( avcodec_open2( m_codecContext, m_codec, 0 ) < 0 )
    {
        m_open = false;
        return;
    }

    // Hack to correct wrong frame rates that seem to be generated by some codecs
    if( m_codecContext->time_base.num>1000 && m_codecContext->time_base.den==1)
    {
        m_codecContext->time_base.den=1000;
    }

    // Allocate video frame
    m_avFrame = avcodec_alloc_frame();
}

/**
    Construct a LibAvCapture object that is ready to read from the specified file.

    @param videoFile video file in a valid format (please see libavcodec docs on your platform for supported formats).

    @note This calls the static member InitLibAvCodec() so is not thread safe. If you are using this
    class in a multi-threaded system then call InitLibAvCodec() manually in your start up code before launching the multi-threaded components.
*/
LibAvCapture::LibAvCapture( const char* videoFile )
:
    m_formatContext ( 0 ),
    m_customIO      ( 0 ),
    m_codecContext  ( 0 ),
    m_open ( false )
{
    Init( videoFile );
}

/*
    Construct a writer that will use custom I/O.

    @param customIO the custom io object that must provide an AVIOContext
    that is valid for output.
*/
LibAvCapture::LibAvCapture( FFMpegCustomIO& customIO )
:
    m_formatContext ( 0 ),
    m_customIO      ( &customIO ),
    m_codecContext  ( 0 ),
    m_open ( false )
{
    Init( m_customIO->GetStreamName() );
}

LibAvCapture::~LibAvCapture()
{
    if ( m_open )
    {
        av_free( m_avFrame );
        avcodec_close( m_codecContext );

        if ( m_customIO == 0 )
        {
            avformat_close_input( &m_formatContext );
        }
        else
        {
            avformat_free_context( m_formatContext );
            m_formatContext = 0;
        }
    }
}

bool LibAvCapture::IsOpen() const
{
    return m_open;
}

/**
    @return false if there was any kinf of IO error, true otherwise.

    @note This will return false even if there are more frames in the
    internal decode buffer that could be returned by GetFrame(), so if
    all frames are required you will need to continue to call GetFrame()
    until GetFrame() itself returns false.
*/
bool LibAvCapture::IoError() const
{
    assert( m_formatContext != 0 );

    assert( m_formatContext->pb != 0 );
    if ( m_formatContext->pb == 0 )
    {
        return true; // no IO object has been set - consider this error
    }

    return m_formatContext->pb->error < 0;
}

/**
    Read frame and buffer it internally.

    @return false if there are no more frames to read, true otherwise.
*/
bool LibAvCapture::GetFrame()
{
    if ( m_open == false )
    {
        return false;
    }

    bool success = false;
    int frameFinished;
    AVPacket packet;
    int eof;
    while( (eof = av_read_frame( m_formatContext, &packet)) == 0 )
    {
        // Is this a packet from the video stream?
        if( packet.stream_index == m_videoStream )
        {
            // Decode video frame
            int bytes = avcodec_decode_video2( m_codecContext, m_avFrame, &frameFinished, &packet );

            // Did we get a video frame?
            if( bytes >=0 && frameFinished )
            {
                success = true;
                av_free_packet( &packet );
                break;
            }
        }

        av_free_packet( &packet );
    }

    if ( success == false )
    {
        if ( m_codecContext->codec->capabilities & CODEC_CAP_DELAY )
        {
            // av_read_frame reached the end of input
            // but there might be more buffered frames:
            packet.data = 0;
            packet.size = 0;
            int bytes = avcodec_decode_video2( m_codecContext, m_avFrame, &frameFinished, &packet );
            if( bytes >=0 && frameFinished )
            {
                success = true;
            }
        }
    }

    return success;
}

/**
    Frees any resources allocated during GetFrame(), hence not calling this will cause a memory or resource leak.
*/
void LibAvCapture::DoneFrame()
{
}

int32_t LibAvCapture::GetFrameWidth() const
{
    return m_codecContext->width;
}

int32_t LibAvCapture::GetFrameHeight() const
{
    return m_codecContext->height;
}

/**
    @return the frame's timestamp in micro seconds. For video formats where no time stamp is available return -1.
*/
int64_t LibAvCapture::GetFrameTimestamp_us() const
{
    if ( m_avFrame->pts == AV_NOPTS_VALUE )
    {
        return -1; // no presentation timestamp provided by decoder
    }

    // Convert from the codec's timebase to microseconds:
    int64_t pts = av_rescale_q( m_avFrame->pts, m_codecContext->time_base, (AVRational){1,1000000} );
    return pts;
}

void LibAvCapture::ExtractLuminanceImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_GRAY8, data, stride );
}

void LibAvCapture::ExtractRgbImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_RGB24, data, stride );
}

void LibAvCapture::ExtractBgrImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_BGR24, data, stride );
}

void LibAvCapture::ExtractRgb565Image( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_RGB565, data, stride );
}

/**
    Uses swscale library to convert the most recently read frame to
    the specified format.

    @note It is currently assumed the output image will be the same size as the
    video frame read from the file.

    @param format PixelFormat of output buffer - currently only supports single plane formats.
    @param data pointer to buffer that must be large enough to hold the data
    @param stride number of bytes to jump between rows in data.
*/
void LibAvCapture::FrameConversion( PixelFormat format, uint8_t* data, int stride )
{
    const int w = m_codecContext->width;
    const int h = m_codecContext->height;
    uint8_t* dstPlanes[4] = { data, 0, 0, 0 };
    int dstStrides[4] = { stride, 0, 0, 0 };

    m_converter.Configure( w, h, m_codecContext->pix_fmt, w, h, format );
    m_converter.Convert( m_avFrame->data, m_avFrame->linesize, 0, h, dstPlanes, dstStrides );
}

