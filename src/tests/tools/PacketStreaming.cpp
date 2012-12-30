// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <VideoLib.h>
#include <RoboLib.h>
#include <time.h>

#include <queue>
#include <iostream>

#include "../../../src/puppybot/ComCentre.h"

double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

int reportError( const char* error, bool fatal=true )
{
    std::cerr << "Error: " << error << std::endl;

    if ( fatal )
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}

/**
    Open the camera device and enter a loop that wills tream video over the specified socket.
*/
int streamVideo( TcpSocket& client )
{
    UnicapCamera camera;

    if ( camera.IsOpen() )
    {
        ComCentre comms( client );

        // Create a video writer object that passes a lamba function that posts video packets to
        // the communication sub-system:
        FFMpegStdFunctionIO videoIO( FFMpegCustomIO::WriteBuffer, [&]( uint8_t* buffer, int size ) {
            comms.PostPacket( ComPacket( ComPacket::Type::AvData, buffer, size ) );
            return comms.Ok() ? size : -1;
        });

        LibAvWriter streamer( videoIO );

        // Setup an MPEG4 video stream:
        // @note Unicap capture is currently hard-coded to half scale the images so the stream must be half-sized to work:
        streamer.AddVideoStream( camera.GetFrameWidth()/2, camera.GetFrameHeight()/2, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );

        struct timespec t1;
        struct timespec t2;

        VideoFrame frame( camera.UnsafeBufferAccess(), PIX_FMT_YUV420P, camera.GetFrameWidth()/2, camera.GetFrameHeight()/2, camera.GetFrameWidth()/2 );

        // Start capturing and transmitting images:
        camera.StartCapture();
        bool sentOk = true;
        clock_gettime( CLOCK_MONOTONIC, &t1 );
        while ( sentOk && camera.GetFrame() )
        {
            clock_gettime( CLOCK_MONOTONIC, &t2 );

            sentOk = streamer.PutVideoFrame( frame );
            camera.DoneFrame();
            sentOk &= !streamer.IoError();

            double grabTime = milliseconds(t2) - milliseconds(t1);
            fprintf( stderr, "%f %f %f %f\n", grabTime, streamer.lastConvertTime_ms, streamer.lastEncodeTime_ms, streamer.lastPacketWriteTime_ms );
            clock_gettime( CLOCK_MONOTONIC, &t1 );
        }

    }
    else
    {
        return reportError( "No video source found." );
    }

    return EXIT_SUCCESS;
}

/**
    Capture video from camera and stream it to a single connected client.
*/
int runServer( int argc, char** argv )
{
    int port = atoi( argv[1] );
    TcpSocket server;
    if ( server.Bind( port ) == false )
    {
        return reportError( "Could not bind socket" );
    }

    std::cerr << "Starting video-streaming server on port " << port << std::endl;

    server.Listen( 0 );
    TcpSocket* clientConnection =  server.Accept();

    if ( clientConnection )
    {
        Ipv4Address clientAddress;
        clientConnection->GetPeerAddress( clientAddress );
        std::string clientName;
        clientAddress.GetHostName( clientName );
        std::cerr << "Connection from '" << clientName << "'" << std::endl;

        // Network connection successful so try to capture video:
        int streamSuccess = streamVideo( *clientConnection );
        delete clientConnection;
        return streamSuccess;
    }
    else
    {
        return reportError( "Could not accept connection." );
    }
}

/**
    The client receives (and displays) the video stream.
*/
int runClient( int argc, char** argv )
{
    std::cerr << "Starting video-streaming client" << std::endl;
    TcpSocket client;

    if ( client.Connect( argv[1], atoi( argv[2] ) ) )
    {
        ComCentre comms( client );

        // Create a video writer object that passes a lamba function that reads from socket:
        FFMpegStdFunctionIO videoIO( FFMpegCustomIO::ReadBuffer, [&comms]( uint8_t* buffer, int size ) {
            /// @todo - All this needs a mutex!:
            std::queue<ComPacket>& avPackets = comms.GetAvDataQueue();

            ///@todo - use condition variable:
            while ( comms.LockPackets() == false ) {}

            std::cerr << "Queued packet count := " << avPackets.size() << std::endl;
            std::cerr << "Requested Packet size := " << size << std::endl;

            // We were asked for more than packet contains so loop through packets until
            // we have returned what we needed or there are no more packets:
            int required = size;
            while ( required > 0 && !avPackets.empty() )
            {
                ComPacket& packet = avPackets.front();
                const int availableSize = packet.GetData().size();

                std::cerr << "\tCurrent Packet size := " << availableSize << std::endl;
                std::cerr << "\tRemaining required bytes := " << required << std::endl;

                if ( availableSize <= required )
                {
                    // Current packet contains less than required so copy the whole packet
                    // and continue:
                    std::copy( packet.GetData().begin(), packet.GetData().end(), buffer );
                    avPackets.pop();
                    buffer += availableSize;
                    std::cerr << "\tSent " << availableSize << " bytes." << std::endl;
                    required -= availableSize;
                }
                else
                {
                    assert( availableSize > required );
                    // Current packet contains at least enough to fulfill the request
                    // so copy what is required and save the rest for later:
                    std::copy( packet.GetData().begin(), packet.GetData().begin()+required, buffer );
                    const size_t remainder = availableSize - required;
                    std::copy( packet.GetData().begin()+required, packet.GetData().end(), packet.GetData().begin() );
                    packet.GetData().resize( remainder );
                    std::cerr << "\tSent " << required << " bytes. Remainder:=" << remainder << ". Finished" << std::endl;
                    required = 0;
                }
            }

            std::cerr << "\tTotal Sent " << size - required << std::endl;
            comms.DonePackets();/// @todo - horrible
            return size - required;
        });

        LibAvCapture streamer( videoIO );
        if ( streamer.IsOpen() == false )
        {
            return reportError( "Could not create stream capture." );
        }

        // Get some frames so we can extract correct image dimensions:
        for ( int i=0;i<3;++i )
        {
            streamer.GetFrame();
            streamer.DoneFrame();
        }

        int w = streamer.GetFrameWidth();
        int h = streamer.GetFrameHeight();
        std::cerr << "Recevied frame dimensions: " << w << "x" << h << std::endl;

        // Create a buffer for image data:
        uint8_t* imageBuffer;
        int err = posix_memalign( (void**)&imageBuffer, 16, w * h * 3 * sizeof(uint8_t) );
        assert( err == 0 );

#ifndef ARM_BUILD
        // Setup a display window:
        robo::AnnotatedImage display( w, h );
        GLK::ImageWindow::ImageData postData; // this struct describes the image data we post for display
        postData.mode = GLK::ImageWindow::FixedAspectRatio;
        postData.w = w;
        postData.h = h;
        postData.stride = w*3;
        postData.ptr = imageBuffer;
        postData.isColourBgr = true;

        GLK::Timer timer;
#endif

        int numFrames = 0;
        bool gotFrame = true;

#ifndef ARM_BUILD
        while ( display.IsRunning() && gotFrame )
#else
        while ( gotFrame )
#endif
        {
            gotFrame = streamer.GetFrame();
            if ( gotFrame )
            {
                streamer.ExtractBgrImage( imageBuffer, w*3 );
                streamer.DoneFrame();
#ifndef ARM_BUILD
                display.PostImage( postData );
#else
                std::cerr << "got frame" << std::endl;
#endif
                numFrames += 1;
            }
            else
            {
                reportError( "Could not get frame." );
            }
            if ( numFrames == 50 )
            {
#ifndef ARM_BUILD
                std::cerr << "Average frame rate: " << numFrames/timer.GetSeconds() << " fps" << std::endl;
                timer.Reset();
#endif
                numFrames = 0;
            }
        }

        client.Shutdown();
        free( imageBuffer );
        return EXIT_SUCCESS;
    }
    else
    {
        return reportError( "Could not connect." );
    }
}

/*
    Server process captures live video and streams it over a socket.

    The client process displays the video.
*/
int main( int argc, char** argv )
{
    if ( argc == 2 )
    {
        // Only a port specified so this will be a server process:
        return runServer( argc, argv );
    }
    else if ( argc == 3 )
    {
        // IP and port specified so process should behave as client:
        return runClient( argc, argv );
    }

    std::cerr << "Usage: " << argv[0] << " [ADDRESS] PORT" << std::endl;
    return EXIT_FAILURE;
}

