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
    if ( argc < 1 )
    {
        return 0;
    }

    MotionMind motors( "/dev/ttyUSB0" );
    int32_t position;

    Socket s; // Setup a server socket:
    s.Bind( atoi(argv[1]) ); // Get port from command line
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
                int nf = sscanf( msg, "speed: %d %d\n", &leftSpeed, &rightSpeed );
                if ( nf == 2 )
                {
                    fprintf( stderr, "Setting speed: %d %d\n", leftSpeed, rightSpeed );
                            
                    motors.SetSpeed( 1, -leftSpeed, position );
                    motors.SetSpeed( 2, rightSpeed, position );
                }
            }
        }
        
    }
    delete con;

/*    UnicapCamera camera;
    MotionMind motors( "/dev/ttyUSB0" );

    if ( camera.IsAvailable() && motors.Available() )
    {
        GLK::String title( "" );
        title += ':';
        title += ' ';
        title += GLK::String( camera.GetVendor() );
        title += ' ';
        title += GLK::String( camera.GetModel() );
        fprintf( stderr, "Found camera: %s\n", title.cStr() );
    
        uint8_t* m_lum;
        int err = posix_memalign( (void**)&m_lum, 16, camera.GetFrameWidth() * camera.GetFrameHeight() * sizeof(uint8_t) );
        assert( err == 0 );
        
        camera.StartCapture();
        
        int32_t position;
        motors.SetSpeed( 1, -600, position );
        motors.SetSpeed( 2, 600, position );
    
        for ( int i = 0; i<40; ++i )
        {    
            camera.GetFrame();
            double timeStamp = camera.GetFrameTimestamp() / 1000000.0;
            camera.ExtractLuminanceImage( m_lum );
            camera.DoneFrame();
            
            fprintf( stderr, "Captured frame. Timestamp = %f secs\n", timeStamp );
            char name [] = "capture_####.pgm";
            sprintf( name, "capture_%.4d.pgm", i+1 );
            WritePgm( name, m_lum, camera.GetFrameWidth(), camera.GetFrameHeight() );
            GLK::Thread::Sleep( 50 );
        }
    
        motors.SetSpeed( 1, 0, position );
        motors.SetSpeed( 2, 0, position );
    
        motors.Move( 1, 0, position );
        motors.Move( 2, 0, position );
        
        camera.StopCapture();
        
        free( m_lum );
    }*/

}

