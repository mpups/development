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

/*
    Quick test of various components fo robolib in one program.
*/
int main( int argc, char** argv )
{   
    UnicapCamera camera;
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
    }

}

