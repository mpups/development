// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <glkcore.h>
#include <glk.h>

#include <RoboLib.h>

#include <boost/scoped_ptr.hpp>

class AnnotatedImage : public GLK::ImageWindow
{
public:
    AnnotatedImage( uint16_t w, uint16_t h ) : ImageWindow(w,h) {};
    virtual ~AnnotatedImage() {};

    virtual void Annotate() const
    {
        glColor3f( 1, 0, 0 );
        glBegin( GL_LINES );
            glVertex2f( 0.1f,0.1f);
            glVertex2f( 0.7f,0.7f);
        glEnd();

        glColor3f( 0, 1, 0 );
        glPointSize( GetWidth()/640.f );
        glBegin( GL_POINTS );
            glVertex2f( 0.8f, 0.8f );
        glEnd();
    };
};

/*
*/
int main( int argc, char** argv )
{
    boost::scoped_ptr<CameraCapture> m_camera( new UnicapCamera() );

    if ( m_camera->IsAvailable() == false )
    {
        return EXIT_FAILURE;
    }

    m_camera->StartCapture();

    AnnotatedImage display( m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
    display.SetWindowName( "Image Processing Lab" );

    uint8_t* lum;
    int err = posix_memalign( (void**)&lum, 16, m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t) );
    assert( err == 0 );

    while ( display.IsRunning() )
    {
        m_camera->GetFrame();
        m_camera->ExtractLuminanceImage( lum );
        display.PostImage( GLK::ImageWindow::ScaleToFit, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), lum );
        m_camera->DoneFrame();
    }

    m_camera->StopCapture();

    display.WaitForClose();

    return EXIT_SUCCESS;
}

