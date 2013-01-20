#include "VideoClient.h"

VideoClient::VideoClient( PacketDemuxer &demuxer )
{
    // Lambda which subscribes to the packets containing video-data:
    PacketSubscription sub = demuxer.Subscribe( ComPacket::Type::AvData, [this]( const ComPacket::ConstSharedPacket& packet )
    {
        m_avPackets.Emplace( packet ); // The callback simply queues up all the packets.
        m_totalVideoBytes += packet->GetDataSize();
    });
}

VideoClient::~VideoClient()
{
}

bool VideoClient::InitialiseVideoStream()
{
    // Create a video reader object that uses socket IO:
    m_videoIO.reset( new FFMpegStdFunctionIO( FFMpegCustomIO::ReadBuffer, std::bind( &VideoClient::ReadPacket, std::ref(*this), std::placeholders::_1, std::placeholders::_2 ) ) );
    m_streamer.reset( new LibAvCapture( *m_videoIO ) );
    if ( m_streamer->IsOpen() == false )
    {
        return false;
    }

    // Get some frames so we can extract correct image dimensions:
    bool gotFrame = false;
    for ( int i=0;i<2;++i )
    {
        gotFrame = m_streamer->GetFrame();
        m_streamer->DoneFrame();
    }

    if ( gotFrame == false )
    {
        return false;
    }

    const int w = m_streamer->GetFrameWidth();
    const int h = m_streamer->GetFrameHeight();
    std::clog << "Received frame dimensions: " << w << "x" << h << std::endl;

    return true;
}

bool VideoClient::ReceiveVideoFrame( std::function< void(LibAvCapture&) > callback )
{
    assert( m_streamer != nullptr );

    bool gotFrame = m_streamer->GetFrame();
    if ( gotFrame )
    {
        callback( *m_streamer );
        m_streamer->DoneFrame();
    }

    return gotFrame;
}

int VideoClient::ReadPacket( uint8_t* buffer, int size )
{
    SimpleQueue::LockedQueue lock = m_avPackets.Lock();
    while ( m_avPackets.Empty() ) /// @note this used to check the state of the muxer (incase of comms error)
    {
        m_avPackets.WaitNotEmpty( lock );
    }

    // We were asked for more than packet contains so loop through packets until
    // we have returned what we needed or there are no more packets:
    int required = size;
    while ( required > 0 && m_avPackets.Empty() == false )
    {
        const ComPacket::ConstSharedPacket packet = m_avPackets.Front();
        const int availableSize = packet->GetData().size() - m_packetOffset;

        if ( availableSize <= required )
        {
            // Current packet contains less than required so copy the whole packet
            // and continue:
            std::copy( packet->GetData().begin() + m_packetOffset, packet->GetData().end(), buffer );
            m_packetOffset = 0; // Reset the packet offset so the next packet will be read from beginning.
            m_avPackets.Pop();
            buffer += availableSize;
            required -= availableSize;
        }
        else
        {
            assert( availableSize > required );
            // Current packet contains more than enough to fulfill the request
            // so copy what is required and save the rest for later:
            auto startItr = packet->GetData().begin() + m_packetOffset;
            std::copy( startItr, startItr+required, buffer );
            m_packetOffset += required; // Increment the packet offset by the amount read from this packet.
            required = 0;
        }
    }

    return size - required;
}
