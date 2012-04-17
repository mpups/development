// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS

#include <RoboLib.h>

#include <glkcore.h>
#include <glk.h>

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

    // TEST video:
    if ( argc >1 )
    {
        LibAvCapture video( argv[1] );

        if ( !video.IsOpen() )
        {
            std::cerr << "Couldn't open video-file: " << argv[1] << std::endl;
        }
        else
        {
            while( display.IsRunning() && video.GetFrame() )
            {
                video.ExtractLuminanceImage( lum, video.GetFrameWidth() );
                display.PostImage( GLK::ImageWindow::FixedAspectRatio, video.GetFrameWidth(), video.GetFrameHeight(), lum );

                video.DoneFrame();
            }
        }
    }

    robo::LoadBalancingCornerDetector m_detector(
                m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), // w,h
                m_camera->GetFrameWidth(), // stride
                lum // image data
            );

    GLK::Timer timer;
    std::vector< robo::PixelCoord > detectedCorners;

    while ( display.IsRunning() )
    {
        m_camera->GetFrame();
        m_camera->ExtractLuminanceImage( lum );

        timer.Reset();
        m_detector.Detect( 33, detectedCorners );
        uint64_t detectTime = timer.GetMicroSeconds();
        std::cout << "Feature detection time := " << detectTime << "us" << std::endl;

        // Update visualisation:
        display.Clear();
        uint32_t split = m_detector.GetSplitHeight();
        display.Add( robo::AnnotatedImage::Line( robo::AnnotatedImage::Point(0,split), robo::AnnotatedImage::Point(640,split) ) );
        for ( size_t c=0;c<detectedCorners.size();++c)
        {
            display.Add( robo::AnnotatedImage::Point( detectedCorners[c].x, detectedCorners[c].y ) );
        }

        display.PostImage( GLK::ImageWindow::FixedAspectRatio, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), lum );

        m_camera->DoneFrame();
    }

    m_camera->StopCapture();

    display.WaitForClose();

    return EXIT_SUCCESS;
}

