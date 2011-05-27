#include "ImageWindow.h"

#ifndef ARM_BUILD

const int DEFAULT_WIDTH = 320;
const int DEFAULT_HEIGHT = 240;

ImageWindow::ImageWindow()
:
    GlThreadWindow  ( DEFAULT_WIDTH, DEFAULT_HEIGHT ),
    m_msgs          ( 8 ),
    m_image         ( {0,16,16,FixedSize} )
{
    StartEventLoopAsync();
    Show();
    SetWindowName( "Image Display" );
}

/**
    Tell the window to quit and then wait for its event loop to finish.
**/
ImageWindow::~ImageWindow()
{
    Quit( 0 );
    WaitForClose();
}

/**
    Setup a texture object resource and try to initialise texture with
    the first image in the message queue.

    If the message queue is empty a blank texture is initialised instead.
**/
bool ImageWindow::InitGL()
{
    glGenTextures( 1, &m_lumTex );
    
    m_image.ptr = 0;
    while( m_msgs.TryRead( &m_image, 1 ) )
    {
        // loop to get latest image data
    }
    
    uint8_t blank[ m_image.w * m_image.h ];
    if ( m_image.ptr == 0 )
    {
        memset( blank, 1, m_image.w * m_image.h );
        m_image.ptr = blank;
    }
    
    glBindTexture( GL_TEXTURE_2D, m_lumTex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE8, m_image.w, m_image.h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image.ptr );
    
    return true;
}

/**
    Delete the texture object resource.
**/
void ImageWindow::DestroyGL()
{
    glDeleteTextures( 1, &m_lumTex );
}

/**
    Called automatically in the window's message loop.

    Checks to see if new data has been posted. If so then updates the
    texture for display and causes a re-paint by returning true. Otherwise
    simply returns false and no re-paint occurs.
**/
bool ImageWindow::Update( unsigned int )
{
    ImageData newData;
    newData.ptr = 0;
    while( m_msgs.TryRead( &newData, 1 ) )
    {
    }
    uint8_t* lum = newData.ptr;
    
    if ( lum )
    {
        glBindTexture( GL_TEXTURE_2D, m_lumTex );

        // If image size has changed we will need to resize the texture:
        if ( newData.w != m_image.w || newData.h != m_image.h )
        {
            glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE8, newData.w, newData.h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, lum );
            m_image = newData;        
            Resize( GetWidth(), GetHeight() ); // call resize aswell
        }
        else
        {
            glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, newData.w, newData.h, GL_LUMINANCE, GL_UNSIGNED_BYTE, lum );
            m_image = newData;
        }

        return true;
    }
    
    return false;
}

void ImageWindow::Resize( const unsigned int w, const unsigned int h )
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, 1, 1, 0, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glColor3f( 1.f, 1.f, 1.f );

    // setup viewport based on display mode
    switch ( m_image.mode )
    {
    case FixedSize:    
        glViewport( 0, 0, m_image.w, m_image.h );
        break;
    case FixedAspectRatio:
        {
            float windowAspect = (float)w / (float)h;
            float imageAspect = (float)m_image.w / (float)m_image.h;
            float wf,hf;
            
            // Fit the image to the dimension that will contain the whole image.
            if ( w > h && windowAspect > imageAspect )
            {            
                hf = h;
                wf = (m_image.w * hf) / m_image.h;
            }
            else
            {
                wf = w;
                hf = (m_image.h * wf) / m_image.w;
            }
            
            glViewport( 0, 0, wf, hf );
        }
        break;
    case ScaleToFit:
        glViewport( 0, 0, w, h );
        break;
    }

    Render();
}

/**
    Display the image on a textured quad.
**/
void ImageWindow::Render()
{
    // Projection and viewports were setup in Resize()

    // Render the image
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

/**
    Send new image data to the window for display.
    @param imageBuffer Pointer to greyscale image data.
    @param width_px    Width of image in pixels.
    @param height_px   Height of image in pixels
    @param mode Select how the image should be fit into the window.
**/
void ImageWindow::PostImage( uint8_t* imageBuffer, uint16_t width_px, uint16_t height_px, DisplayMode mode )
{
    ImageData data = { imageBuffer, width_px, height_px, mode };
    m_msgs.Write( &data, 1 );
    Notify();
}

#endif

