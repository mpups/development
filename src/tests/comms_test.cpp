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

int main( int argc, char** argv )
{   
    if ( argc == 2 )
    {
        fprintf( stderr, "server process here...\n" );
    
        // Only a port specified so this is server process
        MotionMind motors( "/dev/ttyUSB0" );
        DiffDrive drive( motors );
        int32_t position;

        Socket s; // Setup a server socket:
        s.Bind( atoi( argv[1] ) ); // Get port from command line
        s.Listen( 1 ); // Wait for connection
        Socket* con = s.Accept(); // Create connection

        if ( con )
        {
            fprintf( stderr, "Client connected to robot.\n" );
            char msg[256];
            int n = 0;
        
            int leftSpeed  = 0;
            int rightSpeed = 0;
            int maxVal     = 1;
        
            // Read messages from client in a loop:
            int failedReads = 0; // Keep count of sequential failed reads.
            while ( failedReads < 20 )
            {
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
                
                // Read amps
                float lA = drive.GetLeftAmps();
                float rA = drive.GetRightAmps();
                
                // Read arduino sensors values
                DiffDrive::MotorData data = drive.JoyControl( leftSpeed, rightSpeed, maxVal );
                
                // Log all the data to stdout:
                fprintf( stdout, "%d %d %f %f\n", data.leftPos, data.rightPos, lA, rA );
            }
        
        }
        delete con;
    }
    else if ( argc == 3 )
    {
        // IP and port specified so process should behave as client:
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
    
}

