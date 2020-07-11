/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __TCP_VIDEO_STREAM_H__
#define __TCP_VIDEO_STREAM_H__

#include "ffmpegjni.h"

#include <memory>

#include <stdint.h>
#include <pthread.h>

class FrameReceiver;
class VideoClient;
class PacketDemuxer;

/**
    Class contains all objects required to create a tcp video input
    stream in FFmpegJNI - socket, thread, synchronisation variables,
    LibAvCapture object etc..

    @todo rename to TcpVideoInputStream
*/
class TcpVideoStream
{
public:
    TcpVideoStream( TcpSocket&, int timeoutInSeconds );
    virtual ~TcpVideoStream();

    bool IsInitialised() const;
    bool IsStreaming() const;

    int32_t WaitForFrame();
    void GetFrameSize( int& w, int& h ) const;
    int64_t GetFrameCount() const;
    double GetStamp() const;
    float GetFrameRate() const;

    bool StartStreaming( JavaVM* jvm );
    void StopStreaming();

    std::unique_ptr<FrameReceiver> SetFrameReceiver( std::unique_ptr<FrameReceiver>&& receiver );

private:
    static void* streaming_thread( void* data );
    void DestroyComponents();
    void StreamingLoop( JNIEnv* env );
    void SignalFrame();

    std::unique_ptr<PacketDemuxer>  m_demuxer;
    std::unique_ptr<VideoClient>    m_videoClient;
    std::unique_ptr<PacketSubscription> m_subscription;

    std::unique_ptr<FrameReceiver>  m_frameReceiver;

    static const int MEAN_FRAME_WINDOW = 60;
    float m_meanFrameRate;

    pthread_t       m_thread;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
    int32_t         m_frameCount;
    bool 		    m_initialised;
    volatile bool   m_terminate;
    volatile bool   m_streaming;

    JavaVM* m_jvm;

    struct AvInfo
    {
		timespec stamp;
		int frameNumber;
    };

    AvInfo m_avinfo = {{-1,-1},-1};
};

#endif /* __TCP_VIDEO_STREAM_H__ */
