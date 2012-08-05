#include "FFmpegSocketIO.h"

#include <assert.h>

#include "../io/TcpSocket.h"
#include "../io/Ipv4Address.h"

//#include <iostream>

int socket_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    int numBytes = io->m_socket.Write( reinterpret_cast<char*>(buffer), size );

    if (numBytes > 0 )
    {
        io->m_bytesTx += numBytes;
    }

    //std::cerr << "putting " << size << " bytes (return value := " << numBytes << ")" << std::endl;
    return numBytes;
}

int socket_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    int numRead = io->m_socket.Read( reinterpret_cast<char*>(buffer), size );

    if (numRead > 0 )
    {
        io->m_bytesRx += numRead;
    }
    //std::cerr << "requesting " << size << " bytes (read " << numRead <<")" << std::endl;
    return numRead;
}

FFMpegSocketIO::FFMpegSocketIO( TcpSocket& socket, bool sender )
:
    m_socket  ( socket ),
    m_bytesTx ( 0 ),
    m_bytesRx ( 0 )
{
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
    int writeable = sender ? 1 : 0;
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, writeable, this, socket_read_packet, socket_write_packet, 0 );

    m_socket.SetNagleBufferingOff();
}

FFMpegSocketIO::~FFMpegSocketIO()
{
    av_free( m_io );
    // av_free( m_buffer ); // @todo - freeing causes crash (ffmpeg bug?)
}

AVIOContext* FFMpegSocketIO::GetAVIOContext()
{
    return m_io;
}

/**
    Get the address of the peer connection, then retrieve
    the host name storing a local copy and returning its C string.
*/
const char* FFMpegSocketIO::GetStreamName()
{
    Ipv4Address address;
    m_socket.GetPeerAddress( address );
    address.GetHostName( m_peerName );
    return m_peerName.c_str();
}

uint64_t FFMpegSocketIO::BytesRead() const
{
    return m_bytesRx;
}

uint64_t FFMpegSocketIO::BytesWritten() const
{
    return m_bytesTx;
}

