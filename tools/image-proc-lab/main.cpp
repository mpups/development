// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <glkcore.h>
#include <glk.h>

#include <RoboLib.h>

#include <boost/scoped_ptr.hpp>

extern "C"
{
#include "../../src/third_party/fast-C-src-2.1/fast.h"
}

/*
*/
int main( int argc, char** argv )
{
    boost::scoped_ptr<CameraCapture> m_camera( new UnicapCamera() );

    if ( m_camera->IsAvailable() == false )
    {
        return EXIT_FAILURE;
    }

    m_camera->StartCapture();

    robo::AnnotatedImage display( m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
    display.SetWindowName( "Image Processing Lab" );

    uint8_t* lum;
    int err = posix_memalign( (void**)&lum, 16, m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t) );
    assert( err == 0 );

    // Create some corner detector threads:
    robo::FastCornerThread m_cornerDetectThread1;
    robo::FastCornerThread m_cornerDetectThread2;

    // Setup the jobs (they never change):
    assert( m_camera->GetFrameHeight() > 6 );
    robo::FastCornerThread::Job cornerJob1;
    cornerJob1.w = m_camera->GetFrameWidth();
    cornerJob1.h = (m_camera->GetFrameHeight()/2) + 3;
    cornerJob1.stride = m_camera->GetFrameWidth();
    cornerJob1.threshold = 33;
    cornerJob1.buffer = lum;

    int yOffsetJob2 = (m_camera->GetFrameHeight()/2) - 3;
    robo::FastCornerThread::Job cornerJob2;
    cornerJob2.w = m_camera->GetFrameWidth();
    cornerJob2.h = (m_camera->GetFrameHeight()/2) + 3;
    cornerJob2.stride = m_camera->GetFrameWidth();
    cornerJob2.threshold = 33;
    cornerJob2.buffer = lum + ( cornerJob2.stride * yOffsetJob2 );

    GLK::Timer timer;
    std::vector< robo::PixelCoord > detectedCorners;

    while ( display.IsRunning() )
    {
        m_camera->GetFrame();
        m_camera->ExtractLuminanceImage( lum );

        timer.Reset();
        m_cornerDetectThread1.PostJob( cornerJob1 );
        m_cornerDetectThread2.PostJob( cornerJob2 );
        detectedCorners.clear();
        m_cornerDetectThread1.RetrieveResults( detectedCorners );
        m_cornerDetectThread2.RetrieveResults( detectedCorners, 0, yOffsetJob2 );
        uint64_t detectTime = timer.GetMicroSeconds();
        std::cout << "Feature detection time := " << detectTime << "us" << std::endl;

        // Update visualisation:
        display.Clear();
        display.Add( robo::AnnotatedImage::Line( robo::AnnotatedImage::Point(0,240), robo::AnnotatedImage::Point(640,240) ) );
        for ( size_t c=0;c<detectedCorners.size();++c)
        {
            display.Add( robo::AnnotatedImage::Point( detectedCorners[c].x, detectedCorners[c].y ) );
        }

        display.PostImage( GLK::ImageWindow::ScaleToFit, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), lum );

        m_camera->DoneFrame();
    }

    m_camera->StopCapture();

    display.WaitForClose();

    return EXIT_SUCCESS;
}

