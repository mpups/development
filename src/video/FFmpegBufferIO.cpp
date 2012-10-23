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

    FFMpegBufferIO::Buffer tmp;
    tmp.size = size;
    tmp.data = (uint8_t*)av_malloc( tmp.size );
    memcpy( tmp.data, buffer, tmp.size );
    io.m_buffers.push_back( tmp );

    std::cout << "write_packet: " << size << " bytes" << std::endl;
    return size;
}

/**
    Copy from the av buffer to the user buffer.
    @return the number of bytes copied
*/
int buffer_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegBufferIO& io = *reinterpret_cast<FFMpegBufferIO*>( opaque );

    std::cerr << "Starting read_packet: " << io.m_buffers.size() << " buffers in queue." << std::endl;

    if ( io.m_buffers.size() == 0 )
    {
        return -1;
    }

    FFMpegBufferIO::Buffer tmp = io.m_buffers.front();

    const int requested = size;
    while ( size )
    {
        int bytesAvailable = tmp.size - io.m_readIndex; // bytes available from current Buffer
        if ( bytesAvailable >= size )
        {
            memcpy( buffer, tmp.data + io.m_readIndex, size );
            io.m_readIndex += size;
            size = 0;
            std::cerr << "\tread_packet: Finished servicing " << requested-size << " bytes. Read Index := " << io.m_readIndex << std::endl;
        }
        else
        {
            memcpy( buffer, tmp.data + io.m_readIndex, bytesAvailable );
            size -= bytesAvailable;

            // move to next buffer in queue:
            io.m_readIndex = 0;
            if ( io.m_buffers.size() )
            {
                io.m_buffers.pop_front();
                av_free( tmp.data );
                tmp = io.m_buffers.front();
            }
            else
            {
                // No more buffers!
                return requested - size;
            }
            std::cerr << "\tread_packet: serviced " << requested-size << " bytes. Moving to next buffer (size := " << tmp.size << ")" << std::endl;
        }
    }

    std::cerr << "read_packet: " << requested << " bytes" << std::endl;
    return requested;
}

FFMpegBufferIO::FFMpegBufferIO( BufferType direction )
{
    // allocate io context and its buffer:
    AllocateBuffer();
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, buffer_read_packet, buffer_write_packet, 0 );
    m_io->seekable = 0;
}

FFMpegBufferIO::~FFMpegBufferIO()
{
    av_free( m_io );
//    av_free( m_buffer );
}

void FFMpegBufferIO::ChangeDirection( BufferType direction )
{
    av_free( m_buffer );
    av_free( m_io );

    AllocateBuffer();
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, buffer_read_packet, buffer_write_packet, 0 );
    m_io->seekable = 0;
    m_readIndex = 0;
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

void FFMpegBufferIO::AllocateBuffer()
{
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + 2*FF_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
}

