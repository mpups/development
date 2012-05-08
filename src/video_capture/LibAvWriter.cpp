#include "LibAvWriter.h"

#include "LibAvCapture.h"

#include <iostream>

#include <ctype.h>
#include <assert.h>

/**
    Static method returns an 32-bit integer representing the four character code (fourcc).

    Lower case characters are converted to upper case before computing the code.
*/
int32_t LibAvWriter::FourCc( char c1, char c2, char c3, char c4 )
{
    int fourcc = toupper(c4);
    fourcc <<= 8;
    fourcc |= toupper(c3);
    fourcc <<= 8;
    fourcc |= toupper(c2);
    fourcc <<= 8;
    fourcc |= toupper(c1);
    return fourcc;
}

/**
    Static method for choosing optimal encoding pixel format.

    @note PixelFormat is a enum defiend by ffmpeg (in pixfmt.h).

    @param is the CodecID to be used
    @param inputFormat the pixel format the data will be supplied in.
    @return a good choice of encoder format
*/
PixelFormat LibAvWriter::ChooseCodecFormat( CodecID id, PixelFormat inputFormat )
{
    PixelFormat pixelFormat = PIX_FMT_YUV420P;

    switch ( id )
    {
        case CODEC_ID_FFV1:
        case CODEC_ID_HUFFYUV:
        pixelFormat = PIX_FMT_YUV422P;
        break;

        case CODEC_ID_MJPEG:
        pixelFormat = PIX_FMT_YUVJ420P;
        break;

        case CODEC_ID_RAWVIDEO:
        pixelFormat = inputFormat;
        break;

        default:
        break;
    }

    return pixelFormat;
}

/**
    Opens video file for writing and auto-detect format from filename.

    @param videoFile File name to write - if the file exists it will be over-written.
*/
LibAvWriter::LibAvWriter( const char* videoFile )
:
    m_formatContext  (0),
    m_codec          (0),
    m_encodingBuffer (0),
    m_open ( false )
{
    LibAvCapture::InitLibAvCodec();

    // This guesses the container format (e.g. .avi, .ogg):
    m_outputFormat = av_guess_format( 0, videoFile, 0 );

    if ( m_outputFormat == 0 )
    {
        return;
    }

    m_formatContext = avformat_alloc_context();

    if ( m_formatContext == 0 )
    {
        // @todo free m_outputFormat
        return;
    }

    m_formatContext->oformat = m_outputFormat;

    if ( av_set_parameters( m_formatContext, 0 ) < 0 )
    {
        return;
    }

    snprintf( m_formatContext->filename, sizeof(m_formatContext->filename), "%s", videoFile );

    m_encodingBuffer = reinterpret_cast<uint8_t*>( av_malloc( FF_MIN_BUFFER_SIZE ) );

    m_open = true;
}

LibAvWriter::~LibAvWriter()
{
    if ( m_videoStream != 0 )
    {
        avcodec_close( m_videoStream->codec );
        avpicture_free( reinterpret_cast<AVPicture*>( &m_codecFrame ) );
    }

    if ( m_formatContext != 0 )
    {
        av_write_trailer( m_formatContext );
        url_fclose( m_formatContext->pb );
        avformat_free_context( m_formatContext );
    }

    av_free( m_encodingBuffer );
}

bool LibAvWriter::IsOpen() const
{
    return m_open;
}

/**
    @param width width of images to be encoded in this stream
    @param height height of images to be encoded in this stream
    @param fps Frame rate in frames-per-second
    @param fourcc four character code as returned from LibAvWriter::FourCc().
*/
bool LibAvWriter::AddVideoStream( uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc )
{
    bool success = false;

    if ( IsOpen() )
    {
        // Get the codec ID:
        CodecID codecId = av_codec_get_id( m_outputFormat->codec_tag, fourcc );

        if ( codecId == CODEC_ID_NONE )
        {
            std::cerr << "Could not get codec Id" << std::endl;
        }
        else
        {
            m_videoStream = av_new_stream( m_formatContext, 0 );
            if ( m_videoStream != 0 )
            {
                AVCodecContext* codecContext = m_videoStream->codec;
                codecContext->codec_id = codecId;
                codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
                codecContext->codec_tag = fourcc;
                codecContext->bit_rate_tolerance = codecContext->bit_rate;
                codecContext->pix_fmt = ChooseCodecFormat( codecId, PIX_FMT_GRAY8 ); //@todo hard coded to grey-scale input at moment

                assert( width%2 == 0 );
                assert( height%2 == 0 );
                codecContext->width = width;
                codecContext->height = height;
                codecContext->time_base.num = 1;
                if ( fps == 0) { fps = 1; }
                codecContext->time_base.den = fps;

                avcodec_get_frame_defaults( &m_codecFrame );
                int err = avpicture_alloc( reinterpret_cast<AVPicture*>( &m_codecFrame ), codecContext->pix_fmt, codecContext->width, codecContext->height );
                assert( err == 0 );

                av_dump_format( m_formatContext, 0, m_formatContext->filename, 1 );

                m_codec = avcodec_find_encoder( codecId );
                // We don't check result of above because the following fails gracefully if m_codec==null
                err = avcodec_open2( codecContext, m_codec, 0 );
                if ( err == 0 )
                {
                    success = true;
                }
            }
        }
    }

    if ( success )
    {
        int err = url_fopen( &m_formatContext->pb, m_formatContext->filename, URL_WRONLY );
        if ( err < 0 )
        {
            success = false;
        }
        else
        {
            av_write_header( m_formatContext );
        }
    }
    else
    {
        //@todo free m_videoStream ?
    }

    return success;
}

/**
    Write a grey frame to the video-file.

    @note If the width and height of the image data do not match those of the stream, the image will be scaled.
*/
bool LibAvWriter::PutGreyFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride )
{
    bool success = false;
    AVCodecContext* codecContext = m_videoStream->codec;

    if ( m_converter.Configure( width, height, PIX_FMT_GRAY8,
                                codecContext->width, codecContext->height, codecContext->pix_fmt )
       )
    {
        uint8_t* srcPlanes[4] = { buffer, 0, 0, 0 };
        int srcStrides[4] = { stride, 0, 0, 0 };
        m_converter.Convert( srcPlanes, srcStrides, 0, height, m_codecFrame.data, m_codecFrame.linesize );
        WriteCodecFrame();
        success = true;
    }

    return success;
}

/**
    Write the current codec picture to the current stream.
*/
void LibAvWriter::WriteCodecFrame()
{
    AVCodecContext* codecContext = m_videoStream->codec;
    int bytes = avcodec_encode_video( codecContext, m_encodingBuffer, FF_MIN_BUFFER_SIZE, &m_codecFrame );
    if ( bytes > 0 )
    {
        AVPacket pkt;
        av_init_packet( &pkt );
        pkt.stream_index = m_videoStream->index;
        pkt.data = m_encodingBuffer;
        pkt.size = FF_MIN_BUFFER_SIZE;
        if ( codecContext->coded_frame->key_frame )
        {
            pkt.flags |= AV_PKT_FLAG_KEY;
        }
        av_write_frame( m_formatContext, &pkt );
    }
}

