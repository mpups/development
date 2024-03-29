/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FAST_CORNER_THREAD_H__
#define __FAST_CORNER_THREAD_H__

#include <vector>
#include <thread>
#include <cstdint>

#include <glkcore.h>

#include "../third_party/fast-C-src-2.1/fast.hpp"

namespace robo
{

class PixelCoord;

/**
    Class which runs a thread to which
    requests for fast corner detection can
    be made. The corner detection then runs
    asynchronously.
*/
class FastCornerThread
{
public:
    /**
        Struct containing data need to perform a fast
        corner detection. buffer should be a pointer
        to grey-level image data.
    */
    struct Job
    {
        uint16_t w;
        uint16_t h;
        uint16_t stride;
        uint16_t threshold;
        uint8_t* buffer;
    };

    FastCornerThread();
    virtual ~FastCornerThread();

    void PostJob( const Job& j );
    uint32_t RetrieveResults( std::vector<PixelCoord>&, int offsetX=0, int offsetY=0 );

    virtual void Run();

protected:
    void StartThread();
    void StopThread();

private:
    typedef void (*fast_func_type)(const unsigned char*, int, int, int, int, std::vector<Corner>&);

    void SignalResultsReady();
    void WaitForResults();
    void SignalResultsRetrieved();
    void WaitForRetrieval();

    Job m_job;
    std::vector<Corner> m_corners;
    fast_func_type m_fastFunction;

    std::thread m_thread;
    volatile bool m_terminate;
    GLK::MessageQueue<Job> m_jobQueue;
    GLK::Semaphore m_done;
};

} // end of namespace robo

#endif /* __FAST_CORNER_THREAD_H__ */

