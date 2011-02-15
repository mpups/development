#include "ImageWindow.h"

#ifndef ARM_BUILD

ImageWindow::ImageWindow()
:
    GlThreadWindow  ( 640, 480 ),
    m_msgs          ( 8 )
{
    StartEventLoopAsync();
    Show();
    SetWindowName( "Image Display" );
}

ImageWindow::~ImageWindow()
{
    Quit( 0 );
    WaitForClose();
}

bool ImageWindow::InitGL()
{
    glGenTextures( 1, &m_lumTex );
    
    uint8_t* m_lum = 0;
    while( m_msgs.TryRead( &m_lum, 1 ) )
    {
        // loop to get latest image data
    }
    
    glBindTexture( GL_TEXTURE_2D, m_lumTex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE8, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_lum );
    
    return true;
}

void ImageWindow::DestroyGL()
{
    glDeleteTextures( 1, &m_lumTex );
}

void ImageWindow::Resize( const unsigned int w, const unsigned int h )
{
    Render();
}

bool ImageWindow::Update( unsigned int )
{
    uint8_t* m_lum = 0;
    while( m_msgs.TryRead( &m_lum, 1 ) )
    {
    }
    
    if ( m_lum )
    {
        glBindTexture( GL_TEXTURE_2D, m_lumTex );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_lum );
        return true;
    }
    
    return false;
}

void ImageWindow::Render()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, 1, 1, 0, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glColor3f( 1.f, 1.f, 1.f );

    // Render the image from the camera:
    glViewport( 0, 0, 640, 480 );
    glBindTexture( GL_TEXTURE_2D, m_lumTex );
    glEnable( GL_TEXTURE_2D );

    glBegin( GL_QUADS );
    {
        glTexCoord2f( 0, 0 );
        glVertex2i( 0, 0 );

        glTexCoord2f( 0, 1 );
        glVertex2i( 0, 1 );

        glTexCoord2f( 1, 1 );
        glVertex2i( 1, 1 );

        glTexCoord2f( 1, 0 );
        glVertex2i( 1, 0 );
    }
    glEnd();

    SwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void ImageWindow::TimerExpired( int id )
{

}

void ImageWindow::PostImage( uint8_t* imageBuffer )
{
    m_msgs.Write( &imageBuffer, 1 );
    Notify();
}

#endif

