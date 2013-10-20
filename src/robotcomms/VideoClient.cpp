#include "VideoClient.h"
#include "../packetcomms/PacketSerialisation.h"

VideoClient::VideoClient( PacketDemuxer &demuxer )
:
    m_packetOffset (0),
    m_avInfoSubscription (
        demuxer.Subscribe( "AvInfo", [this]( const ComPacket::ConstSharedPacket& packet )
        {
            //AvInfo info;
            timespec stamp;
            int frameNumber;
            double conversionTime;
            Deserialise( packet, stamp, frameNumber, conversionTime );

            const double sendSecs = stamp.tv_sec + (stamp.tv_nsec*0.000000001);
            std::clog.precision(10);
            std::clog << "AvInfo: " << frameNumber << " " << sendSecs << " " << conversionTime << std::endl;
        })
    ),
    m_avDataSubscription (
      demuxer.Subscribe( "AvData", [this]( const ComPacket::ConstSharedPacket& packet )
      {
          m_avDataPackets.Emplace( packet );
          m_totalVideoBytes += packet->GetDataSize();
      })
    )
{
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

/**
    @param seconds Time elapsed since last call to this function (assumes video has benn constantly streaming for this whole time).
    @return Bandwidth used by video stream in bits per second.
*/
double VideoClient::ComputeVideoBandwidthConsumed( double seconds )
{
    double bits_per_sec = ( m_totalVideoBytes - m_lastTotalVideoBytes )*(8.0/seconds);
    m_lastTotalVideoBytes = m_totalVideoBytes;
    return bits_per_sec;
}

bool VideoClient::StreamerOk() const
{
    return m_streamer != nullptr && m_streamer->IoError() == false;
}

bool VideoClient::StreamerIoError() const
{
    if ( m_streamer.get() == nullptr )
    {
        return false; // Streamer not allocated yet (obviosuly this does not count as IO error)
    }

    return m_streamer->IoError();
}

int VideoClient::ReadPacket( uint8_t* buffer, int size )
{
    const int32_t packetTimeout_ms = 1000;
    SimpleQueue::LockedQueue lock = m_avDataPackets.Lock();
    while ( m_avDataPackets.Empty() && m_avDataSubscription.GetDemuxer().Ok() )
    {
        m_avDataPackets.WaitNotEmpty( lock, packetTimeout_ms );
    }

    if ( m_avDataPackets.Empty() )
    {
        std::clog << "VideoClient timed out waiting for an AV packet." << std::endl;
    }

    if ( StreamerIoError() )
    {
        return -1;
    }

    // We were asked for more than packet contains so loop through packets until
    // we have returned what we needed or there are no more packets:
    int required = size;
    while ( required > 0 && m_avDataPackets.Empty() == false )
    {
        const ComPacket::ConstSharedPacket packet = m_avDataPackets.Front();
        const int availableSize = packet->GetData().size() - m_packetOffset;

        if ( availableSize <= required )
        {
            // Current packet contains less than required so copy the whole packet
            // and continue:
            std::copy( packet->GetData().begin() + m_packetOffset, packet->GetData().end(), buffer );
            m_packetOffset = 0; // Reset the packet offset so the next packet will be read from beginning.
            m_avDataPackets.Pop();
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
