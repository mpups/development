#include "../../include/RoboLib.h"

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
    Quick test of various components of robolib in one program.
*/
int main( int argc, char** argv )
{   
    // Try to get camera and motors:
    UnicapCapture camera;
    MotionMind motors( "/dev/ttyUSB0" );

    if ( camera.IsOpen() && motors.Available() )
    {
        // Got them!
        std::string title( "" );
        title += ':';
        title += ' ';
        title += std::string( camera.GetVendor() );
        title += ' ';
        title += std::string( camera.GetModel() );
        fprintf( stderr, "Found camera: %s\n", title.c_str() );
    
        // Allocate memory for capturing images:
        uint8_t* m_lum;
        int err = posix_memalign( (void**)&m_lum, 16, camera.GetFrameWidth() * camera.GetFrameHeight() * sizeof(uint8_t) );
        assert( err == 0 );
        
        camera.StartCapture();
        
        // Move the robot forwards for a short time, capturing images as it goes.
        int32_t position;
        motors.SetSpeed( 1, 0, position );
        motors.SetSpeed( 2, 0, position );

        // repeat - it seems to miss the first command sometimes
        motors.SetSpeed( 1, -600, position );
        motors.SetSpeed( 2, 600, position );
    
        for ( int i = 0; i<40; ++i )
        {    
            camera.GetFrame();
            timespec stamp = camera.GetFrameTimestamp();
            camera.ExtractLuminanceImage( m_lum, camera.GetFrameWidth() );
            camera.DoneFrame();
            
            // Save the captured images to a file:
            fprintf( stderr, "Captured frame. Timestamp: secs %ld nsecs %ld\n", stamp.tv_sec, stamp.tv_nsec );
            char name [] = "capture_####.pgm";
            sprintf( name, "capture_%.4d.pgm", i+1 );
            WritePgm( name, m_lum, camera.GetFrameWidth(), camera.GetFrameHeight() );
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    
        motors.SetSpeed( 1, 0, position );
        motors.SetSpeed( 2, 0, position );
    
        // This ensures minimum current draw from motors.
        motors.Move( 1, 0, position );
        motors.Move( 2, 0, position );
        
        camera.StopCapture();
        
        free( m_lum );
    }

}

