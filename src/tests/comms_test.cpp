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
        
            // Read messages from client in a loop:
            while ( n >= 0 )
            {
                n = con->Read( msg, 256 );
                if ( n > 0 )
                {
                    int leftSpeed;
                    int rightSpeed;
                    int nf = sscanf( msg, "%d %d\n", &leftSpeed, &rightSpeed );
                    if ( nf == 2 )
                    {
                        fprintf( stderr, "Setting speed: %d %d\n", leftSpeed, rightSpeed );
                            
                        motors.SetSpeed( 1, -leftSpeed, position );
                        motors.SetSpeed( 2, rightSpeed, position );
                    }
                    else
                    {
                        motors.SetSpeed( 1, 0, position );
                        motors.SetSpeed( 2, 0, position );    
                    }
                }
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
            if ( client.Connect( argv[1], atoi( argv[2] ) ) )
            {
                int n = 0;
                while ( n >= 0 )
                {
                    int l = js.GetAxis(1) / -33;
                    int r = js.GetAxis(3) / -33;
                    
                    char msg[256];
                    sprintf( msg, "%d %d\n", l, r );
                    n = client.Write( msg, strlen(msg) );
                    GLK::Thread::Sleep( 100 );
                }
            }
        }
    }
    
}

