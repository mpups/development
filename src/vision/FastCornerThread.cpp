#include "FastCornerThread.h"

#include "ImageGeometry.h"

namespace robo
{

FastCornerThread::FastCornerThread()
:
    m_fastFunction ( fast9_detect ),
    m_thread( *this ),
    m_jobQueue ( 2 ),
    m_done ( 0 )
{
    StartThread();
}

FastCornerThread::~FastCornerThread()
{
    StopThread();
}

/**
    Start running asynchronously - after this call the object
    can receive and process jobs.
*/
void FastCornerThread::StartThread()
{
    m_terminate = false;
    m_thread.Start();
}

void FastCornerThread::StopThread()
{
    m_terminate = true;
    Job emptyJob; // need to send this to wake the thread
    emptyJob.w = 0; // setting width to 0 stops the thread from processing the job
    PostJob( emptyJob );
    m_thread.Join();
}

/**
    If the job queue is full this call will blcok until there is space for a new message.
*/
void FastCornerThread::PostJob( const Job& j )
{
    m_jobQueue.Write( &j, 1 );
}

/**
    If you do not call this after calling PostJob() then the corner detection
    thread will block indefinitely.

    @param results Results are appended to this vector.
    @param offsetX added to x-coordinate of detected corners.
    @param offsetY added to y-coordinate of detected corners.

    @return the number of corners detected.
*/
uint32_t FastCornerThread::RetrieveResults( std::vector<PixelCoord>& results, int offsetX, int offsetY )
{
    WaitForResults();

    for ( int c=0;c<m_count;++c)
    {
        PixelCoord p = { m_corners[c].x + offsetX, m_corners[c].y + offsetY };
        results.push_back( p );
    }
    free( m_corners );

    int cornerCount = m_count;
    SignalResultsRetrieved();

    return cornerCount;
}

/**
    Performs the actual detection.
*/
void FastCornerThread::Run()
{
    while ( !m_terminate )
    {
        m_jobQueue.Read( &m_job, 1 );
        if ( m_job.w > 0 )
        {
            m_corners = m_fastFunction
                        (
                            m_job.buffer, m_job.w, m_job.h, m_job.stride,
                            m_job.threshold, &m_count
                        );

            SignalResultsReady();
            WaitForRetrieval();
        }
    }
}

void FastCornerThread::SignalResultsReady()
{
    m_done.Release( 1 );
}

void FastCornerThread::WaitForResults()
{
    m_done.Acquire( 1 );
}

void FastCornerThread::SignalResultsRetrieved()
{
    m_done.Release( 2 );
}

/**
    Block until results have been retrieved, then continue.
*/
void FastCornerThread::WaitForRetrieval()
{
    m_done.Acquire( 2 );
}

} // end of namespace robo

