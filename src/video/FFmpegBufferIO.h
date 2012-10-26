/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FFMPEG_BUFFER_IO_H__
#define __FFMPEG_BUFFER_IO_H__

#include "FFmpegCustomIO.h"

#include <deque>

/**
    This class is just for testing :- video packets are written into a vector of buffers.
*/
class FFMpegBufferIO : public FFMpegCustomIO
{
friend int buffer_write_packet( void* opaque, uint8_t* buffer, int size );
friend int buffer_read_packet( void* opaque, uint8_t* buffer, int size );

public:

    struct Buffer {
        int size; // actual bytes of data in buffer
        uint8_t* data; // the data buffer
    };

    enum BufferType {
        ReadBuffer  = 0,
        WriteBuffer = 1
    };

    FFMpegBufferIO( BufferType direction );
    virtual ~FFMpegBufferIO();

    void ChangeDirection( BufferType direction );

    virtual AVIOContext* GetAVIOContext();
    virtual const char* GetStreamName() const;
    virtual bool IoError() const;

    static const int BUFFER_SIZE = 32*1024;

    void DumpToFile( const char* fileName ) const;

protected:
    void AllocateBuffer();

private:
    AVIOContext* m_io;
    uint8_t* m_buffer;
    std::deque< Buffer > m_buffers;
    int m_readIndex;
};

#endif /* __FFMPEG_BUFFER_IO_H__ */

