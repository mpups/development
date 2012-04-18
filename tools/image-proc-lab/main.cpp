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
    Capture from a live feed or video file and do some image processing:
*/
int main( int argc, char** argv )
{
    boost::scoped_ptr<Capture> capture;
    CameraCapture* camera = 0;

    if ( argc > 1 )
    {
        // Try to capture from video file:
        capture.reset( new LibAvCapture( argv[1] ) );
        if ( false == capture->IsOpen() )
        {
            std::cerr << "Coult not open video-file: " << argv[1] << std::endl;
        }
    }
    else
    {
        // Try to capture from camera:
        capture.reset( new UnicapCamera() );
        camera = reinterpret_cast<CameraCapture*>( capture.get() );
    }

    if ( capture->IsOpen() == false )
    {
        std::cerr << "Error: no video source." << std::endl; 
        return EXIT_FAILURE;
    }

    if ( camera )
    {
        camera->StartCapture();
    }

    robo::AnnotatedImage display( capture->GetFrameWidth(), capture->GetFrameHeight() );
    display.SetWindowName( "Image Processing Lab" );

    uint8_t* lum;
    int err = posix_memalign( (void**)&lum, 16, capture->GetFrameWidth() * capture->GetFrameHeight() * sizeof(uint8_t) );
    assert( err == 0 );

    robo::LoadBalancingCornerDetector m_detector(
                capture->GetFrameWidth(), capture->GetFrameHeight(), // w,h
                capture->GetFrameWidth(), // stride
                lum // image data
            );

    GLK::Timer timer;
    std::vector< robo::PixelCoord > detectedCorners;

    double totalImageConversionTime_us = 0.0;
    double totalDetectTime_us = 0.0;
    double frames = 0.0;

    while ( display.IsRunning() && capture->GetFrame() )
    {
        timer.Reset();
        capture->ExtractLuminanceImage( lum, capture->GetFrameWidth() );
        totalImageConversionTime_us += timer.GetMicroSeconds();

        timer.Reset();
        m_detector.Detect( 33, detectedCorners );
        totalDetectTime_us += timer.GetMicroSeconds();

        // Update visualisation:
        display.Clear();
        uint32_t split = m_detector.GetSplitHeight();
        display.Add( robo::AnnotatedImage::Line( robo::AnnotatedImage::Point(0,split), robo::AnnotatedImage::Point(capture->GetFrameWidth(),split) ) );
        for ( size_t c=0;c<detectedCorners.size();++c)
        {
            display.Add( robo::AnnotatedImage::Point( detectedCorners[c].x, detectedCorners[c].y ) );
        }

        display.PostImage( GLK::ImageWindow::FixedAspectRatio, capture->GetFrameWidth(), capture->GetFrameHeight(), lum );

        capture->DoneFrame();

        frames += 1.0;
    }

    std::cout << "Total Feature detection time := " << totalDetectTime_us << " us" << std::endl;
    std::cout << "Image Conversion time per frame := " << totalImageConversionTime_us/frames << " us" << std::endl;

    if ( camera )
    {
        camera->StopCapture();
    }

    display.WaitForClose();

    return EXIT_SUCCESS;
}

