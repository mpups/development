/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __VIDEO_CLIENT_H__
#define __VIDEO_CLIENT_H__

/// @todo use more limited include set:
#include "../../include/RoboLib.h"

#include <memory>
#include <cinttypes>

class PacketDemuxer;

class VideoClient
{
public:
    VideoClient( PacketDemuxer& demuxer );
    virtual ~VideoClient();

    bool InitialiseVideoStream();
    int GetFrameWidth() const { return m_streamer->GetFrameWidth(); };
    int GetFrameHeight() const { return m_streamer->GetFrameHeight(); };

    bool ReceiveVideoFrame( std::function< void(LibAvCapture&) > );

    double ComputeVideoBandwidthConsumed( double seconds );

protected:
    bool StreamerOk() const;
    bool StreamerIoError() const;
    int  ReadPacket( uint8_t* buffer, int size );

private:
    SimpleQueue m_avPackets;
    int         m_packetOffset;
    uint64_t    m_lastTotalVideoBytes;
    uint64_t    m_totalVideoBytes;
    PacketSubscription m_subscription;

    std::unique_ptr<FFMpegStdFunctionIO> m_videoIO;
    std::unique_ptr<LibAvCapture> m_streamer;
};

#endif /* __VIDEO_CLIENT_H__ */

