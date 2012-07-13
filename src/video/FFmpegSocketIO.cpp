#include "FFmpegSocketIO.h"

#include <assert.h>

#include "../io/TcpSocket.h"
#include "../io/Ipv4Address.h"

int socket_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    return io->m_socket.Write( reinterpret_cast<char*>(buffer), size );
}

int socket_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    return io->m_socket.Read( reinterpret_cast<char*>(buffer), size );
}

FFMpegSocketIO::FFMpegSocketIO( TcpSocket& socket )
:
    m_socket ( socket )
{
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, 1, this, socket_read_packet, socket_write_packet, 0 );

    m_socket.SetNagleBufferingOff();
    m_socket.SetBlocking( false );
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

