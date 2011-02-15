#include "../../include/RoboLib.h"
#include <glkcore.h>

/**
    Saves an image as a simple raw pgm.
*/
void WritePgm( const char* fileName, const uint8_t* buffer, const uint32_t width, const uint32_t height )
{
    FILE* fp = fopen( fileName, "w" );
    assert( fp != 0 );
    if ( fp )
    {
        fprintf( fp, "P5 %d %d 255\n", width, height );
        fwrite( buffer, sizeof(uint8_t), width*height, fp );
        fclose( fp );
    }
}

/**
    Program behaves as server:
    Controls robot and receives commands from cient.
**/
void runServer( int argc, char** argv )
{
    fprintf( stderr, "server process here...\n" );

    // Setup comms to motors:
    DiffDrive*  drive;
    MotionMind* motors = new MotionMind( "/dev/ttyUSB0" );
    
    if ( motors->Available() )
    {
        drive = new DiffDrive( *motors );
    }
    else
    {
        delete motors;
        drive  = 0;
        motors = 0;
    }

    // Setup camera:
    UnicapCamera* camera = new UnicapCamera();
    uint8_t* m_lum;
    size_t imageBufferSize = camera->GetFrameWidth() * camera->GetFrameHeight() * sizeof(uint8_t);
    if ( camera->IsAvailable() )
    {
        camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        delete camera;
        camera = 0;
    }

    // Setup a server socket for receiving client commands:
    Socket s;
    s.Bind( atoi( argv[1] ) ); // Get port from command line
    s.Listen( 1 ); // Wait for connection
    Socket* con = s.Accept(); // Create connection
    con->SetBlocking( false );
        
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    if ( con /*&& drive*/ )
    {
        fprintf( stderr, "Client connected to robot.\n" );
        
        teljoy = new TeleJoystick( *con, *drive ); // Will start receiving and processing remote joystick cammands immediately.
        GLK::Thread::Sleep( 100 );
        fprintf( stderr, "running: %d\n", teljoy->IsRunning() );
            
        // Capture images continuously:
        while ( teljoy->IsRunning() )
        {
            if ( camera )
            {
                camera->GetFrame();
                camera->ExtractLuminanceImage( m_lum );
                camera->DoneFrame();
                con->SetBlocking( true );
                con->Write( reinterpret_cast<char*>( m_lum ), imageBufferSize );
                con->SetBlocking( false );
                GLK::Thread::Sleep( 10 );
            }
            
        } // end while
        
        fprintf( stderr, "Control terminated\n" );
        
    } // end if

    delete teljoy;

    if ( camera )
    {
        camera->StopCapture();
        delete camera;
        free( m_lum );
    }

    delete motors;
    delete drive;
    delete con; 
}

/**
    Program behaves as client: reads joystick commands and sends to sever.
**/
void runClient( int argc, char** argv )
{
        fprintf( stderr, "Client process here...\n" );
#ifndef ARM_BUILD
        ImageWindow display;
#endif        
        Joystick js( "/dev/input/js0" );

        if ( js.IsAvailable() )
        {
            js.Start();
    
            Socket client;
            client.SetNagleBufferingOff();
            if ( client.Connect( argv[1], atoi( argv[2] ) ) )
            {                            
                int n = 0;
                while ( n >= 0 )
                {
                    int jx = js.GetAxis(1);
                    int jy = js.GetAxis(2);
                    int max = 32767;
                    int data[3] = { jx, jy, max };
                    client.SetBlocking( true );
                    client.Write( reinterpret_cast<char*>( data ), 3*sizeof(int) );
                    client.SetBlocking( false );
                                    
                    static uint8_t image[640*480];
                    n += client.Read( reinterpret_cast<char*>( image + n ), 640*480 - n );

                    if ( n == 640*480 )
                    {
#ifndef ARM_BUILD
                        if ( display.IsRunning() )
                        {
                            display.PostImage( image );
                        }
#endif
                        n = 0;
                    }
                    GLK::Thread::Sleep( 46 );
                }
            }
        }
        else
        {
            fprintf( stderr, "Stopped: No joystick.\n" );
        }
}

int main( int argc, char** argv )
{   
    if ( argc == 2 )
    {
        // Only a port specified so this will be a server process:
        runServer( argc, argv );
    }
    else if ( argc == 3 )
    {
        // IP and port specified so process should behave as client:
        runClient( argc, argv );
    }
}

