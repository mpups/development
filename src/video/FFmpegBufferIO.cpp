#include "FFmpegBufferIO.h"

#include <assert.h>

#include <iostream>

/**
    Copy from the user buffer to the av buffer.
    @return the number of bytes copied
*/
int buffer_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegBufferIO& io = *reinterpret_cast<FFMpegBufferIO*>( opaque );

    if( io.m_userBuffer.size < size )
    {
        return -1; // the user buffer is not big enough so return IO error.
    }

    memcpy( io.m_userBuffer.data, buffer, size );
    io.m_userBuffer.used = size;

    std::cout << "write_packet: " << size << " bytes" << std::endl;
    return size;
}

/**
    Copy from the av buffer to the user buffer.
    @return the number of bytes copied
*/
int buffer_read_packet( void* opaque, uint8_t* buffer, int size )
{
    std::cout << "read_packet: " << size << " bytes" << std::endl;
    return -1;
}

FFMpegBufferIO::FFMpegBufferIO( BufferType direction )
{
    // allocate io context and its buffer:
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, buffer_read_packet, buffer_write_packet, 0 );
    m_io->seekable = 0;

    // allocate the internal use buffer:
    m_userBuffer.size = BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;
    m_userBuffer.used == 0;
    m_userBuffer.data = (uint8_t*)av_malloc( m_userBuffer.size );
}

FFMpegBufferIO::~FFMpegBufferIO()
{
    av_free( m_io );
    // av_free( m_buffer );
    av_free( m_userBuffer.data );
}

AVIOContext* FFMpegBufferIO::GetAVIOContext()
{
    return m_io;
}

const char* FFMpegBufferIO::GetStreamName() const
{
    return "User-Buffer-IO";
}

/**
    @return true if there was a low level IO error.
*/
bool FFMpegBufferIO::IoError() const
{
    return m_io->error < 0;
}

