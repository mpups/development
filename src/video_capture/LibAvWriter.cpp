#include "LibAvWriter.h"

#include "LibAvCapture.h"

#include <ctype.h>

#include <iostream>

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

    @param is the CodecID to be used
    @param inputFormat the pixel format the data will be supplied in
    @return a good choice of encoder format
*/
int32_t LibAvWriter::ChooseCodecFormat( CodecID id, int32_t inputFormat )
{
    int32_t pixelFormat = PIX_FMT_YUV420P;

    switch ( id )
    {
        case CODEC_ID_FFV1:
        pixelFormat = PIX_FMT_YUV420P;
        break;

        case CODEC_ID_RAWVIDEO:
        pixelFormat = inputFormat;
        break;

        default:
        break;
    }
}

/**
    Opens video file for writing and auto-detect format from filename.

    @param videoFile File name to write - if the file exists it will be over-written.
*/
LibAvWriter::LibAvWriter( const char* videoFile )
:
    m_formatContext (0),
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

    m_open = true;
}

LibAvWriter::~LibAvWriter()
{
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
            success = false;
        }
        else
        {
            AVStream* videoStream = av_new_stream( m_formatContext, 0 );
            if ( videoStream != 0 )
            {
                AVCodecContext* codecContext = videoStream->codec;
                codecContext->codec_id = codecId;
                codecContext->codec_type = AVMEDIA_TYPE_VIDEO;

                av_dump_format( m_formatContext, 0, m_formatContext->filename, 1 );

                success = true;
            }
        }
    }

    return success;
}

bool LibAvWriter::PutGreyFrame( uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride )
{
    return false;
}

