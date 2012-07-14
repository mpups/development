// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <RoboLib.h>

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

int streamVideo( TcpSocket& client )
{
    UnicapCamera camera;

    if ( camera.IsOpen() )
    {
        // Create a video writer object that uses socket IO:
        FFMpegSocketIO videoIO( client, true );
        LibAvWriter streamer( videoIO );

        // Setup an MPEG4 video stream:
        streamer.AddVideoStream( camera.GetFrameWidth(), camera.GetFrameHeight(), 30, LibAvWriter::FourCc( 'F','M','P','4' ) );

        // Create a buffer for image data:
        uint8_t* imageBuffer;
        int err = posix_memalign( (void**)&imageBuffer, 16, camera.GetFrameWidth() * camera.GetFrameHeight() * 3 * sizeof(uint8_t) );
        assert( err == 0 );

        // Start capturing and transmitting images:
        camera.StartCapture();
        bool sentOk = true;
        while ( sentOk && camera.GetFrame() )
        {
            camera.ExtractLuminanceImage( imageBuffer, camera.GetFrameWidth() );
            sentOk = streamer.PutGreyFrame( imageBuffer, camera.GetFrameWidth(), camera.GetFrameHeight(), camera.GetFrameWidth() );
            camera.DoneFrame();
        }

        free( imageBuffer );
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
        return streamVideo( *clientConnection );
    }
    else
    {
        return reportError( "Could not accept connection." );
    }
}

int runClient( int argc, char** argv )
{
    std::cerr << "Starting video-streaming client" << std::endl;
    TcpSocket client;

    if ( client.Connect( argv[1], atoi( argv[2] ) ) )
    {
        // Create a video reader object that uses socket IO:
        FFMpegSocketIO videoIO( client, false );
        LibAvCapture streamer( videoIO );
        if ( streamer.IsOpen() == false )
        {
            return reportError( "Could not create stream capture." );
        }

        // Create a buffer for image data:
        uint8_t* imageBuffer;
        int err = posix_memalign( (void**)&imageBuffer, 16, 640 * 480 * sizeof(uint8_t) );
        assert( err == 0 );

        // Setup a display window:
        robo::AnnotatedImage display( 640, 480 );

        bool gotFrame = true;
        while ( display.IsRunning() && gotFrame )
        {
            gotFrame = streamer.GetFrame();
            if ( gotFrame )
            {
                streamer.ExtractLuminanceImage( imageBuffer, 640 );
                streamer.DoneFrame();
                display.PostImage( GLK::ImageWindow::FixedAspectRatio, 640, 480, imageBuffer );
            }
            else
            {
                reportError( "Could not get frame." );
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

