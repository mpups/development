#include "KltTracker.h"

#include <RoboLibOpenCv.h>

#include <stdio.h>

/**
    @param maxFeatures The maximum number of features that can be simultaneously tracked.
    @param w width of image in which tracking will be performed
    @param h height of image in which tracking will be performed
**/
KltTracker::KltTracker( int maxFeatures, int w, int h )
:
    m_maxFts     ( maxFeatures ),
    m_numTracked ( 0 )
{
    // Setup opencv images for tracking
    m_imgA = cvCreateImage( cvSize( w, h ), IPL_DEPTH_8U, 1 );
    m_pyrA = cvCreateImage( cvGetSize( m_imgA ), IPL_DEPTH_8U, 1 );
    m_imgB = cvCloneImage( m_imgA );
    m_pyrB = cvCloneImage( m_pyrA );

    m_pointsIn  = new CvPoint2D32f[m_maxFts];
    m_pointsOut = new CvPoint2D32f[m_maxFts];
    m_status    = new char[m_maxFts];
    m_errors    = new float[m_maxFts];
}

/**
    Releases all resources, temporary image buffers, etc.
**/
KltTracker::~KltTracker()
{
    delete [] m_errors;
    delete [] m_status;
    delete [] m_pointsOut;
    delete [] m_pointsIn;

    cvReleaseImage( &m_pyrB );
    cvReleaseImage( &m_imgB );
    cvReleaseImage( &m_pyrA );
    cvReleaseImage( &m_imgA );
}

/**
    The tracker will detect features within the specified image creating a new feature set
    (destroying the old one).

    @param img The image in which to detect features.
**/
void KltTracker::DetectFeatures( uint8_t* img )
{
    FillIplImage( img, m_imgA );
            
    IplImage* eig = cvCreateImage( cvGetSize(m_imgA), IPL_DEPTH_32F, 1 );
    IplImage* tmp = cvCloneImage( eig );
    m_numTracked = m_maxFts;
    cvGoodFeaturesToTrack( m_imgA, eig, tmp, m_pointsIn, &m_numTracked, 0.02, 20, 0, 7, 0 );
    cvCopy( m_imgA, m_imgB );

    cvReleaseImage( &eig );
    cvReleaseImage( &tmp );
}

/**
    The tracker will locate the current feature set within the specified image.

    @param img The image in which to track.
**/
void KltTracker::Track( uint8_t* img )
{
    static int flags = 0;
    FillIplImage( img, m_imgA );
    
    cvCalcOpticalFlowPyrLK(
                    m_imgB, m_imgA, m_pyrB, m_pyrA,
                    m_pointsIn, m_pointsOut, m_numTracked,
                    cvSize(7,7), 2, m_status, m_errors,
                    cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03 ),
                    flags
                );

    flags |= CV_LKFLOW_PYR_A_READY;
    IplImage* swapImg;
    swapImg = m_imgA;
    m_imgA  = m_imgB;
    m_imgB  = swapImg;
    swapImg = m_pyrA;
    m_pyrA  = m_pyrB;
    m_pyrB  = swapImg;
            
    int k = 0;
    for ( int i=0; i<m_numTracked; ++i )
    {
        if ( m_status[i] && m_errors[i] < 500.f )
        {
            m_pointsIn[k++] = m_pointsOut[i];
        }
    }

    m_numTracked = k;
}

