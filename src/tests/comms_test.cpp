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

    // Setup a serial port for Arduino comms:
    SerialPort* serial = new SerialPort( "/dev/ttyUSB1" );

    if ( !serial->IsOpen() )
    {
        fprintf( stderr, "Warning: Could not open serial port to Arduno\n" );
        delete serial;
        serial = 0;
    }
    else
    {
        serial->SetBaudRate( SerialPort::BAUD_9600 );
    }

    if ( con )
    {
        fprintf( stderr, "Client connected to robot.\n" );
        char msg[256];
        int n = 0;
        
        int leftSpeed  = 0;
        int rightSpeed = 0;
        int maxVal     = 1;

        GLK::Timer time;

        // Read messages from client in a loop:
        int failedReads = 0; // Keep count of sequential failed reads.
        while ( failedReads < 20 )
        {
            memset( msg, 0, 256 );
            n = con->Read( msg, 256 );
            if ( n > 0 )
            {
                int nf = sscanf( msg, "%d %d %d\n", &leftSpeed, &rightSpeed, &maxVal );
                if ( nf == 3 )
                {
                    failedReads = 0;
                }
            }
            else
            {
                failedReads += 1;
            }
            
            if ( failedReads > 2 )
            {
                // For safety set speeds to zero when commands stop arriving:
                leftSpeed  = 0;
                rightSpeed = 0;
                maxVal     = 1;  
            }
            
            uint32_t ms = time.GetMilliSeconds();
            time.Reset();
            fprintf( stderr, "Loop time: %dms\n", ms );
            
            if ( drive )
            {
                DiffDrive::MotorData data = drive->JoyControl( leftSpeed, rightSpeed, maxVal );
            }
            
            if ( camera )
            {
                camera->GetFrame();
                camera->ExtractLuminanceImage( m_lum );
                camera->DoneFrame();
                con->Write( reinterpret_cast<char*>( m_lum ), imageBufferSize );
            }
            
        } // end while
            
    }

    if ( camera )
    {
        camera->StopCapture();
        delete camera;
        free( m_lum );
    }

    delete motors;
    delete drive;
    delete serial;
    delete con; 
}

/**
    Program behaves as client: reads joystick commands and sends to sever.
**/
void runClient( int argc, char** argv )
{
        fprintf( stderr, "Client process here...\n" );
        ImageWindow display;
        
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
                    char msg[256];
                    sprintf( msg, "%d %d %d\n", jx, jy, max );
                    n = client.Write( msg, strlen(msg) );
                
                    static uint8_t image[640*480];
                    int n = 0;
                    while ( n != 640 * 480 )
                    {
                        n += client.Read( reinterpret_cast<char*>( image + n ), 640*480 - n );
                        //fprintf( stderr,"\tn = %d\n", n );
                    }
                    //fprintf( stderr,"Image received?: n = %d\n", n );
                    if (display.IsRunning() )
                    {
                        display.PostImage( image );
                    }
                    //WritePgm( "remote_capture.pgm", image, 640, 480 );
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

