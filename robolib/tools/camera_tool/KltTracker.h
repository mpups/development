// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef KLT_TRACKER_H
#define KLT_TRACKER_H

#include <opencv2/imgproc/imgproc.hpp>

/**
    Wrapper for OpenCV's KLT tracking code.
**/
class KltTracker
{
public:
    KltTracker( int maxFeatures, int w, int h );
    ~KltTracker();

    void DetectFeatures( uint8_t* img );
    void Track( uint8_t* img );

    int NumTracked() const { return m_numTracked; };
    const CvPoint2D32f& operator[] ( int i ) const { return m_pointsOut[i]; };

private:
    const int       m_maxFts;
    IplImage*       m_imgA;
    IplImage*       m_pyrA;
    IplImage*       m_imgB;
    IplImage*       m_pyrB;
    CvPoint2D32f*   m_pointsIn;
    CvPoint2D32f*   m_pointsOut;
    char*           m_status;
    float*          m_errors;
    int             m_numTracked;
};

#endif // KLT_TRACKER_H

