// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef UNICAP_CAMERA_H
#define UNICAP_CAMERA_H

#include <unicap.h>
#include <stdint.h>

#include <glk.h>

#include "CameraCapture.h"

/**
    Class for accessing USB cameras through libunicap.

**/
class UnicapCamera : public CameraCapture
{
public:
    UnicapCamera( unsigned long long guid = 0 );
    ~UnicapCamera();

    bool IsAvailable() const;
    void StartCapture();
    void StopCapture();

    void GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    uint64_t GetFrameTimestamp() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t* );
    void ExtractRgbImage( uint8_t* );

private:
    bool OpenDevice();
    bool FindFormat( int width, int height, unsigned int fourcc, unicap_format_t& format );

    static void NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data );    

    unicap_handle_t m_handle;
    GLK::Semaphore  m_frameReady;
    uint8_t*        m_buffer;
    uint32_t        m_width;
    uint32_t        m_height;
    uint64_t        m_guid;
    GLK::String     m_vendor;
    GLK::String     m_model;
    volatile uint64_t m_time;
    volatile bool   m_captureFrame;
};

#endif // UNICAP_CAMERA_H

