// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#include "CameraWindow.h"
#include "CameraCalibrationSystem.h"
#include "KltTracker.h"

#include <assert.h>

#include <opencv/highgui.h>

#include <RoboLibVision.h>

using namespace GLK;

CameraWindow::CameraWindow( String title )
:
    GlWindow        ( 1, 1 ),
    m_lastTimestamp ( 0 ),
    m_interFrameTime_ms ( 0 ),
    m_waitTime_ms   ( 0 ),
    m_lum           ( 0 ),
    m_rgb           ( 0 ),
    m_calibration   (0),
    m_showUndistorted( false ),
    m_klt           (0),
    m_tracking      (false),
    m_videoWriter   (0)
{
    m_camera = new UnicapCamera();
  
    if ( m_camera->IsOpen() )
    {
        // set window title:
        title += ':';
        title += ' ';
        title += String( m_camera->GetVendor() );
        title += ' ';
        title += String( m_camera->GetModel() );

        int err = posix_memalign( (void**)&m_lum, 16, m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t) );
        assert( err == 0 );
        err = posix_memalign( (void**)&m_rgb, 16, m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * 3 * sizeof(uint8_t) );
        assert( err == 0 );    
    }
    else
    {  
        Quit( -1 );
    }

    SetWindowName( title.cStr() );

    PushKeyboardHandler( this );
}

CameraWindow::~CameraWindow()
{
    delete m_calibration;
    delete m_klt;
    delete m_camera;
    free( m_lum );
    free( m_rgb );

    delete m_videoWriter;
}

bool CameraWindow::InitGL()
{
    bool ok = true;

    if ( m_camera->IsOpen() )
    {
        m_camera->StartCapture();
        m_camera->GetFrame();
        m_camera->ExtractLuminanceImage( m_lum, m_camera->GetFrameWidth() );
        m_camera->ExtractBgrImage( m_rgb, m_camera->GetFrameWidth()*3 );
        m_camera->DoneFrame();

        // Set window size to match camera: 
        Move( 50, 50, m_camera->GetFrameWidth(), m_camera->GetFrameHeight() + 100 );

        // setup texture for displaying captured images:
        glGenTextures( 1, &m_lumTex );
        glGenTextures( 1, &m_rgbTex );

        glBindTexture( GL_TEXTURE_2D, m_lumTex );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE8, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_lum );
        assert( glGetError() == GL_NO_ERROR );

        glBindTexture( GL_TEXTURE_2D, m_rgbTex );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, m_rgb );
        assert( glGetError() == GL_NO_ERROR );

        // Setup calibration system:
        m_calibration = new CameraCalibrationSystem( 10, 7, 23 );

        // Setup a tracker:
        m_klt = new KltTracker( 256, m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );

        // Test - setup video writer - use lossless ffv1 codec:
        m_videoWriter = new LibAvWriter( "test.avi" );
        assert ( m_videoWriter->IsOpen() );
        if ( m_videoWriter->IsOpen() )
        {
            bool streamOk = m_videoWriter->AddVideoStream( m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), 30, LibAvWriter::FourCc( 'F','F','V','1' ) );
            assert( streamOk );
        }
    }
    else
    {
        ok = false;
    }

    fprintf( stderr, "OpenGL version: %s\n", glGetString( GL_VERSION ) );

    glPointSize( 4.f );

    // setup a font for rendering:
    m_font = new GLK::GlFont( m_fontLibrary, DEFAULT_FONT );
    if ( m_font->FontLoaded() )
    {
        fprintf( stderr, "Successfully loaded font '%s, %s'\n", m_font->GetFace().GetFamilyName(), m_font->GetFace().GetStyleName() );
        m_font->SetSizePx( 10 );
    }
    else
    {
        fprintf( stderr, "Could not load font '%s'\n", DEFAULT_FONT );
        ok = false;
    }

    return ok;
}

void CameraWindow::DestroyGL()
{
    if ( m_camera->IsOpen() )
    {
        m_camera->StopCapture();

        glDeleteTextures( 1, &m_lumTex );
        glDeleteTextures( 1, &m_rgbTex );
    }

    delete m_font;
}

void CameraWindow::Resize( const unsigned int w, const unsigned int h )
{
    Render();
}

bool CameraWindow::Update( unsigned int )
{
    bool captured = false;

    if ( m_camera->IsOpen() )
    {
        GLK::Timer waitTimer;
        m_camera->GetFrame();
        m_waitTime_ms = waitTimer.GetMicroSeconds() / 1000;

        m_camera->ExtractLuminanceImage( m_lum, m_camera->GetFrameWidth() );

        m_camera->ExtractBgrImage( m_rgb, 3*m_camera->GetFrameWidth() );

        bool frameOK = m_videoWriter->PutBgrFrame( (uint8_t*)m_rgb, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), 3*m_camera->GetFrameWidth() );
        assert( frameOK );

        glBindTexture( GL_TEXTURE_2D, m_lumTex );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, m_lum );

        glBindTexture( GL_TEXTURE_2D, m_rgbTex );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), GL_BGR, GL_UNSIGNED_BYTE, m_rgb );

        if ( m_showUndistorted && m_calibration->Calibrated() )
        {
            m_calibration->UndistortImage( m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), m_lum );
            glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, m_lum );
        }

        if ( m_tracking )
        {
            m_klt->Track( m_lum );
        }

        m_interFrameTime_ms = (m_camera->GetFrameTimestamp_us() - m_lastTimestamp) / 1000;
        m_lastTimestamp = m_camera->GetFrameTimestamp_us();
        m_camera->DoneFrame();        
        captured = true;
    }

    return captured;
}

void CameraWindow::TimerExpired( int id )
{
}

/**
    Used with CalibrationSystem::VisitCorners() to render detected corner locations.
*/
struct CornerPlotter
{
    CornerPlotter( int camWidth, int camHeight ) : w(camWidth), h(camHeight) {};
    void Visit( CvPoint2D32f corner ) { glVertex2f( corner.x/w, corner.y/h); };

    int w;
    int h;
};
template void CameraCalibrationSystem::VisitCorners( CornerPlotter& plotter ) const;

void CameraWindow::Render()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, 1, 1, 0, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glColor3f( 1.f, 1.f, 1.f );

    // Render the image from the camera:
    glViewport( 0, 0, m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
    glBindTexture( GL_TEXTURE_2D, m_lumTex );
glBindTexture( GL_TEXTURE_2D, m_rgbTex );
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

    glDisable( GL_TEXTURE_2D );

    if ( m_calibration->FoundAll() )
    {
        glColor3f( 0.f, 1.f, 0.f ); // green
    }
    else
    {
        glColor3f( 1.f, 0.f, 0.f ); // red
    }

    // Render detected calibration points and tracked points:
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_BLEND );
    glBegin( GL_POINTS );
    {
        CornerPlotter plotter( m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
        m_calibration->VisitCorners( plotter );

        if ( m_klt )
        {
            glColor3f( 0.f, 1.f, 0.f );
            for ( int i=0; i<m_klt->NumTracked(); ++i )
            {
                glVertex2f( (*m_klt)[i].x/m_camera->GetFrameWidth(), (*m_klt)[i].y/m_camera->GetFrameHeight() );
            }
        }
       
    }
    glEnd();
    //glDisable( GL_BLEND );
    glDisable( GL_POINT_SMOOTH );

    glColor4f( 1, 0 ,0, 0.5 );
    glBegin( GL_LINES );
    {
        glVertex2f( 0, 0.5);
        glVertex2f( 1, 0.5 );
        
        glVertex2f( 0, 0.25 );
        glVertex2f( 1, 0.25 );
        
        glVertex2f( 0, 0.75 );
        glVertex2f( 1, 0.75 );
        
        glVertex2f( 0.5, 0 );
        glVertex2f( 0.5, 1 );
        
        glVertex2f( 0.25, 0 );
        glVertex2f( 0.25, 1 );
        
        glVertex2f( 0.75, 0 );
        glVertex2f( 0.75, 1 );
    }
    glEnd();

    // Render textual info:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, GetWidth(), 0, 100, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
    glViewport( 0, m_camera->GetFrameHeight(), GetWidth(), 100 );

    glColor3f( 0.2f, 0.2f, 0.2f );

    glBegin( GL_QUADS );
    {
        glVertex2i( 0, 0 );
        glVertex2i( 0, 100 );
        glVertex2i( m_camera->GetFrameWidth(), 100 );
        glVertex2i( m_camera->GetFrameWidth(), 0 );
    }
    glEnd();

    glTranslatef(4,80,0);
    glColor3f( 1.f, 1.f, 1.f );

    const int bufferSize = 1024;
    char info[bufferSize];
    int n = snprintf( info, bufferSize, "Inter-frame time: %ums\nTime waiting for frame: %ums\n", m_interFrameTime_ms, m_waitTime_ms );
    if ( n < bufferSize )
    {
        m_calibration->ToString( info+n, bufferSize-n );
    }

    m_font->RenderString( info );

    SwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void CameraWindow::Key( GLK::Key k )
{
    switch (k)
    {
        case KeyReturn: // RETURN computes calibration
            m_calibration->ComputeCalibration();
        break;

        case KeySpace: // SPACE saves the calibration image
            m_calibration->AddCalibrationImage( m_camera->GetFrameWidth(), m_camera->GetFrameHeight(), m_lum );
        break;

        case Keyu: // Toggle display of original or undistorted image
            m_showUndistorted = !m_showUndistorted;
        break;

        case Keys: // Save the calibration result
            m_calibration->Print( stdout );
        break;

        case Keyp: // Save an image
            WritePgm( "shot.pgm", m_lum, m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
        break;

        case Keyt: // start tracking points using optical flow
        {
            m_klt->DetectFeatures( m_lum );
            m_tracking = true;
        }
        break;

        case KeyEscape:
            GlWindow::Quit(0);
        break;

        default:
        break;
    }
}

bool CameraWindow::WasUpdated()
{
    return false;
}

void CameraWindow::Activated()
{
    
}

void CameraWindow::Deactivated()
{
    
}

