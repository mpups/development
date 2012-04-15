#include "LoadBalancingCornerDetector.h"

#include "ImageGeometry.h"

#include <iostream>

namespace robo
{

LoadBalancingCornerDetector::LoadBalancingCornerDetector( uint32_t width, uint32_t height, uint32_t stride, uint8_t* buffer )
:
    m_height( height ),
    m_splitHeight( height / 2 ),
    m_cornerCount1( 0 ),
    m_cornerCount2( 0 )
{
    // Setup the jobs:
    assert( height > 32 );

    m_cornerJob1.w = width;
    m_cornerJob1.stride = width;
    m_cornerJob1.buffer = buffer;

    m_cornerJob2.w = width;
    m_cornerJob2.stride = width;

    ReBalance();
}

LoadBalancingCornerDetector::~LoadBalancingCornerDetector()
{
}

/**
    Return the height at which the image is currently being split for load balancing purposes.
*/
uint32_t LoadBalancingCornerDetector::GetSplitHeight() const
{
    return m_splitHeight;
}

/**
    @param threshold fast corner detection threshold
    @param results vector of pixel-coords to store the corner locations - this vector will be cleared (while the jobs are completing asynchronously).
*/
void LoadBalancingCornerDetector::Detect( int threshold, std::vector< PixelCoord >& results )
{
    m_cornerJob1.threshold = threshold;
    m_cornerJob2.threshold = threshold;

    // Send the jobs:
    m_cornerDetectThread1.PostJob( m_cornerJob1 );
    m_cornerDetectThread2.PostJob( m_cornerJob2 );

    int oldOffset = m_yOffsetJob2; // need to record this because ReBalance changes it.
    ReBalance(); // Lags one frame behind but not noticeable, and we may aswell do it while we are waiting for jobs to finish.

    // Get the results:
    results.clear();
    m_cornerCount1 = m_cornerDetectThread1.RetrieveResults( results );
    m_cornerCount2 = m_cornerDetectThread2.RetrieveResults( results, 0, oldOffset );
}

/**
    Uses the corner counts on previous iteration to adjust the jobs for next time.
*/
void LoadBalancingCornerDetector::ReBalance()
{
    // Use a threshold (20% of total number of corners) so
    // we don't jump about too much due to noise:
    const uint32_t thresh = (m_cornerCount1 + m_cornerCount2) / 5;

    // Don't split too close to image top/bottom:
    uint32_t border = m_height / 20;
    if ( border < 16 )
    {
        border = 16;
    }

    // Compute delta for the work split:
    if ( m_cornerCount1 > m_cornerCount2+thresh && m_splitHeight > border )
    {
        m_splitHeight -= 16;
    }

    if ( m_cornerCount1+thresh < m_cornerCount2 && m_splitHeight < (m_height - border - 1))
    {
        m_splitHeight += 16;
    }

    // Quick reset:
    if ( m_cornerCount1 == m_cornerCount2 )
    {
        m_splitHeight = m_height / 2;
    }

    m_yOffsetJob2 = (m_splitHeight) - 3;
    m_cornerJob1.h = (m_splitHeight) + 3;
    m_cornerJob2.h = (m_height - m_splitHeight) + 3;
    m_cornerJob2.buffer = m_cornerJob1.buffer + ( m_cornerJob2.stride * m_yOffsetJob2 );
}

} // end of namespace robo

