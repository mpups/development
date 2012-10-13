#include "FFmpegSocketIO.h"

#include <assert.h>

#include "../io/TcpSocket.h"
#include "../io/Ipv4Address.h"

/**
    Send the buffered packet over the socket.
    @return the number of bytes sent over the socket.
*/
int socket_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    int numBytes = io->m_socket.Write( reinterpret_cast<char*>(buffer), size );

    if (numBytes > 0 )
    {
        io->m_bytesTx += numBytes;
    }

    return numBytes;
}

/**
    Receive from the socket and store in the buffer.
    @return the number of bytes received over the socket.
*/
int socket_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegSocketIO* io = reinterpret_cast<FFMpegSocketIO*>( opaque );
    int numRead = io->m_socket.Read( reinterpret_cast<char*>(buffer), size );

    if (numRead > 0 )
    {
        io->m_bytesRx += numRead;
    }
    return numRead;
}

/**
    Create an FFmpeg IO object that will read/write av data to the specified socket object.
    @param socket Socket to use for IO.
    @param sender True if this IO object will be used to send video, false if it will be used to receive video.
*/
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
    m_io->seekable = 0;

    m_socket.SetNagleBufferingOff();
}

FFMpegSocketIO::~FFMpegSocketIO()
{
    av_free( m_io );
    // av_free( m_buffer );
    /** @todo - freeing causes crash (ffmpeg bug?) */
}

AVIOContext* FFMpegSocketIO::GetAVIOContext()
{
    return m_io;
}

/**
    Get the address of the peer connection, then retrieve
    the host name storing a local copy and returning its C string.
*/
const char* FFMpegSocketIO::GetStreamName() const
{
    // Logical constness - we aren't changing the peer name just retrieving it:
    const_cast<std::string&>( m_peerName ) = RetrievePeerName();
    return m_peerName.c_str();
}

/**
    @return true if there was a low level IO error.
*/
bool FFMpegSocketIO::IoError() const
{
    return m_io->error < 0;
}

uint64_t FFMpegSocketIO::BytesRead() const
{
    return m_bytesRx;
}

uint64_t FFMpegSocketIO::BytesWritten() const
{
    return m_bytesTx;
}

/**
    Get the name of the connected peer and store internally.
    It is only valid to call this after connection is established.
*/
std::string FFMpegSocketIO::RetrievePeerName() const
{
    std::string peerName;
    Ipv4Address address;
    m_socket.GetPeerAddress( address );
    address.GetHostName( peerName );
    return peerName;
}

