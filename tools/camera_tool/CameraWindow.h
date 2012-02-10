// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef CAMERA_WINDOW_H
#define CAMERA_WINDOW_H

#include <glkcore.h>
#include <glk.h>
#include <RoboLib.h>
#include <cv.h>

// fwd decls:
class KltTracker;
class CameraCalibrationSystem;

/**
    A simple window app that can display a live camera feed,
    save calibration images and compute the camera calibration.
**/
class CameraWindow : public GLK::GlWindow, public GLK::KeyboardHandler
{
public:
    CameraWindow( GLK::String title );
    virtual ~CameraWindow();

    virtual bool InitGL();
    virtual void DestroyGL();
    virtual void Resize(const unsigned int, const unsigned int);
    virtual bool Update( unsigned int );
    virtual void Render();
    virtual void TimerExpired( int id );

    void Key( GLK::Key c );
    bool WasUpdated();
    void Activated();
    void Deactivated();

private:
    CameraCapture*       m_camera;

    Ft::Library          m_fontLibrary;
    GLK::GlFont*         m_font;
    GLuint               m_lumTex;
    GLuint               m_rgbTex;
    uint64_t             m_lastTimestamp;
    uint32_t             m_interFrameTime_ms;
    uint32_t             m_waitTime_ms;
    uint8_t*             m_lum;
    uint8_t*             m_rgb;
    
    // Camera Calibration
    CameraCalibrationSystem* m_calibration;
    bool m_showUndistorted;

    // LK Tracking
    KltTracker* m_klt;
    bool m_tracking;
};

#endif

