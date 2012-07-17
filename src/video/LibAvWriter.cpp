#include "LibAvWriter.h"

#include "LibAvCapture.h"
#include "LibAvVideoStream.h"
#include "FFmpegCustomIO.h"

extern "C" {
#include <libavutil/mathematics.h>
}

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

    @param id the CodecID to be used
    @param inputFormat the pixel format the data will be supplied in.
    @return a valid format for the specified CodecID
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
    Initialisation common to all constructors.
    
    Should only be called from within a constructor.
*/
void LibAvWriter::Init()
{
    LibAvCapture::InitLibAvCodec();

    m_formatContext = avformat_alloc_context();

    if ( m_formatContext == 0 )
    {
        return;
    }

    if ( m_customIO != 0 )
    {
        m_formatContext->pb = m_customIO->GetAVIOContext();
    }

    m_open = true;
}

/**
    Opens video file for writing. Auto-detects format from filename extension.

    @param videoFile File name to write - if the file exists it will be over-written.
*/
LibAvWriter::LibAvWriter( const char* videoFile )
:
    m_formatContext  (0),
    m_customIO       (0),
    m_stream         (0),
    m_open ( false )
{
    Init();
    if ( m_open )
    {
        // This guesses the container format (e.g. .avi, .ogg):
        m_outputFormat = av_guess_format( 0, videoFile, 0 );
        if ( m_outputFormat == 0 )
        {
            m_open = false;
            return;
        }

        m_formatContext->oformat = m_outputFormat;
        snprintf( m_formatContext->filename, sizeof(m_formatContext->filename), "%s", videoFile );
    }
}

/**
    Construct a writer that will use custom I/O.

    @param customIO the custom io object that must provide an AVIOContext
    that is valid for output.
*/
LibAvWriter::LibAvWriter( FFMpegCustomIO& customIO )
:
    m_formatContext  (0),
    m_customIO       (&customIO), // m_customIO ptr should not need to be deleted locally!
    m_stream         (0),
    m_open ( false )
{
    Init();
    if ( m_open )
    {
        // This guesses the container format (e.g. .avi, .ogg):
        m_outputFormat = av_guess_format( "avi", 0, 0 );
        m_formatContext->oformat = m_outputFormat;
        snprintf( m_formatContext->filename, sizeof(m_formatContext->filename), "%s", customIO.GetStreamName() );
    }
}

LibAvWriter::~LibAvWriter()
{
    if ( m_stream && m_stream->IsValid() )
    {
        avpicture_free( reinterpret_cast<AVPicture*>( &m_codecFrame ) );
        av_write_trailer( m_formatContext );

        if ( m_customIO == 0 )
        {
            avio_close( m_formatContext->pb );
        }
    }

    delete m_stream;

    if ( m_formatContext != 0 )
    {
        avformat_free_context( m_formatContext );
    }
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
        m_stream = new LibAvVideoStream( m_formatContext, width, height, fps, fourcc );
        if ( m_stream->IsValid() )
        {
            avcodec_get_frame_defaults( &m_codecFrame );
            int err = avpicture_alloc( reinterpret_cast<AVPicture*>( &m_codecFrame ), m_stream->CodecContext()->pix_fmt, m_stream->CodecContext()->width, m_stream->CodecContext()->height );
            assert( err == 0 );

            m_codecFrame.pts = 0;
            av_dump_format( m_formatContext, 0, m_formatContext->filename, 1 );

            // We don't check result of above because the following fails gracefully if m_codec==null
            err = avcodec_open2( m_stream->CodecContext(), m_stream->Codec(), 0 );
            if ( err == 0 )
            {
                success = true;
            }
        }
    }

// @todo - can this be done elsewhere - what if we had more than 1 stream?
    if ( success )
    {
        // We don't need to call avio_open if we are using custom I/O:
        if ( m_customIO == 0 )
        {
            int err = avio_open( &m_formatContext->pb, m_formatContext->filename, AVIO_FLAG_WRITE );
            if ( err != 0 )
            {
                success = false;
            }
        }

        if ( success )
        {
            avformat_write_header( m_formatContext, 0 );
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
    return PutFrame( buffer, width, height, stride, PIX_FMT_GRAY8 );
}

/**
    Write an RGB colour frame to the video file.

    @note If the width and height of the image data do not match those of the stream, the image will be scaled.
    @note If a colour video stream has not been setup then the video will be converted to greyscale.
*/
bool LibAvWriter::PutRgbFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride )
{
    return PutFrame( buffer, width, height, stride, PIX_FMT_RGB24 );
}

/**
    Write an RGB colour frame to the video file.

    @note If the width and height of the image data do not match those of the stream, the image will be scaled.
    @note If a colour video stream has not been setup then the video will be converted to greyscale.
*/
bool LibAvWriter::PutBgrFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride )
{
    return PutFrame( buffer, width, height, stride, PIX_FMT_BGR24 );
}

bool LibAvWriter::PutYUYV422Frame( uint8_t* buffer, uint32_t width, uint32_t height )
{
    return PutFrame( buffer, width, height, width*2, PIX_FMT_YUYV422 );
}

/**
    Write a frame with the specified pixel format to the video file.

    The sws_scale library is used to convert the frame from the specified format to the format
    required by the stream's codec.
*/
bool LibAvWriter::PutFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride, PixelFormat format )
{
    bool success = false;
    AVCodecContext* codecContext = m_stream->CodecContext();

    if ( m_converter.Configure( width, height, format,
                                codecContext->width, codecContext->height, codecContext->pix_fmt )
       )
    {
        uint8_t* srcPlanes[4] = { buffer, 0, 0, 0 };
        int srcStrides[4] = { stride, 0, 0, 0 };
        m_converter.Convert( srcPlanes, srcStrides, 0, height, m_codecFrame.data, m_codecFrame.linesize );
        m_codecFrame.pts += 1;
        success = WriteCodecFrame();
    }

    return success;
}

/**
    Write the current codec picture to the current stream.
*/
bool LibAvWriter::WriteCodecFrame()
{
    bool ok = false;

    AVCodecContext* codecContext = m_stream->CodecContext();
    AVPacket pkt;
    // Note: used to set pkt.stream_index but encode_video2 now seems to set it using the codec context.
    pkt.data = m_stream->Buffer();
    pkt.size = m_stream->BufferSize();
    int packetOk;
    int err = avcodec_encode_video2( codecContext, &pkt, &m_codecFrame, &packetOk );

    if ( err == 0 && packetOk == 1 )
    {
        // Note: not sure if we need to do this anymore as it gets set in encode_video2:
        pkt.pts = av_rescale_q( codecContext->coded_frame->pts, codecContext->time_base, m_stream->TimeBase() );

        if ( codecContext->coded_frame->key_frame )
        {
            pkt.flags |= AV_PKT_FLAG_KEY;
        }
        err = av_write_frame( m_formatContext, &pkt );
        ok = err == 0;
    }

    return ok;
}

