// Copyright (c) 2013 Mark Pupilli, All Rights Reserved.

#include <VideoLib.h>
#include <glkcore.h>
#include <glk.h>

#include <memory>
#include <iostream>

/*
    Capture from a live feed or video file and do some image processing:
*/
int main( int argc, char** argv )
{
    std::unique_ptr<Capture> capture;
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

    GLK::ImageWindow display( capture->GetFrameWidth(), capture->GetFrameHeight() );
    display.SetWindowName( "Capture" );

    uint8_t* lum;
    int err = posix_memalign( (void**)&lum, 16, capture->GetFrameWidth() * capture->GetFrameHeight() * sizeof(uint8_t) );
    assert( err == 0 );

    GLK::Timer timer;
    double totalImageConversionTime_us = 0.0;
    double frames = 0.0;

    while ( display.IsRunning() && capture->GetFrame() )
    {
        timer.Reset();
        capture->ExtractLuminanceImage( lum, capture->GetFrameWidth() );
        totalImageConversionTime_us += timer.GetMicroSeconds();

        display.PostImage( GLK::ImageWindow::FixedAspectRatio, capture->GetFrameWidth(), capture->GetFrameHeight(), lum );

        capture->DoneFrame();

        frames += 1.0;
    }

    if ( camera )
    {
        camera->StopCapture();
    }

    display.WaitForClose();

    free( lum );

    return EXIT_SUCCESS;
}
