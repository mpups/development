#include "../RoboLib.h"

#include <glkcore.h>

/*#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include <stdio.h>
 
int main(int argc, char* argv[])
{    
  CvCapture* camera = cvCreateCameraCapture(-1); // Use the default camera
 
  IplImage*     frame = 0;
  IplImage      img;
 
  frame = cvQueryFrame(camera); //need to capture at least one extra frame
  frame = cvQueryFrame(camera);
  if (frame != NULL) {
    printf("got frame\n\r");
        cvSaveImage("webcam.jpg", frame);
  } else {
      printf("Null frame\n\r");
  }
  cvReleaseCapture(&camera);
  return 0;
}*/

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
    Joystick joystick( "/dev/input/js0" );
    if ( joystick.IsAvailable() )
    {
        joystick.Start();
        
        Joystick::ButtonEvent b;
        while ( b.id != 16 )
        {
            fprintf( stderr, "%d %d\n", joystick.GetAxis(2), joystick.GetAxis(1)  );
        }
    }
    
    UnicapCamera camera;
    if ( camera.IsAvailable() )
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
        for ( int i = 0; i<10; ++i )
        {    
            camera.GetFrame();
            fprintf( stderr, "Captured frame. Timestamp = %f secs\n", camera.GetFrameTimestamp()/1000000.0 );
            camera.ExtractLuminanceImage( m_lum );
            camera.DoneFrame();
        }
        camera.StopCapture();
        
        WritePgm( "test_capture.pgm", m_lum, camera.GetFrameWidth(), camera.GetFrameHeight() );
    
        free( m_lum );
    }
}

