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
    COntrols robot and receives commands from cient.
**/
void runServer( int argc, char** argv )
{
    fprintf( stderr, "server process here...\n" );

    // Setup comms to motors:
    MotionMind motors( "/dev/ttyUSB0" );
    DiffDrive drive( motors );
    int32_t position;

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
                    //fprintf( stderr, "Received joystick axes: %d %d (%d)\n", leftSpeed, rightSpeed, maxVal );
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
            
            // Read amps
            float lA = drive.GetLeftAmps();
            float rA = drive.GetRightAmps();
                
            // Read arduino sensors values
            int at,ax,ay,az;
            if ( serial )
            {
                n = 0;
                while (n <= 1)
                {
                    n = serial->ReadLine( msg, 255 );
                }
                n = sscanf( msg, "%d %d %d %d", &at, &ax, &ay, &az );
                if ( n != 4 )
                {
                    at = -1;
                }
            }
            
            DiffDrive::MotorData data = drive.JoyControl( leftSpeed, rightSpeed, maxVal );
           
            // Log all the data to stdout:
            fprintf( stdout, "%d %d %f %f %d %d %d %d\n", data.leftPos, data.rightPos, lA, rA, at, ax, ay, az );

        } // end while
            
    }

    delete serial;
    delete con;    
}

/**
    Program behaves as client: reads joystick commands and sends to sever.
**/
void runClient( int argc, char** argv )
{
        fprintf( stderr, "Client process here...\n" );
        
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
                    GLK::Thread::Sleep( 50 );
                }
            }
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

