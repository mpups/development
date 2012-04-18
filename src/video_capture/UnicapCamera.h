// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef UNICAP_CAMERA_H
#define UNICAP_CAMERA_H

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <stdint.h>

extern "C" {
#include <unicap.h>
#include <libswscale/swscale.h>
}

#include <glkcore.h>

#include "CameraCapture.h"

/**
    Class for accessing USB cameras through libunicap.

**/
class UnicapCamera : public CameraCapture
{
public:
    UnicapCamera( unsigned long long guid = 0 );
    ~UnicapCamera();

    bool IsOpen() const;
    void StartCapture();
    void StopCapture();

    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    uint64_t GetFrameTimestamp() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t*, int );
    void ExtractRgbImage( uint8_t*, int );
    void ExtractBgrImage( uint8_t* img, int stride );

protected:
    void FrameConversion( PixelFormat format, uint8_t* data, int stride );

private:
    bool OpenDevice();
    bool FindFormat( int width, int height, unsigned int fourcc, unicap_format_t& format );
    void EnumerateProperties();

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

    SwsContext* m_imageConversionContext;
};

#endif // UNICAP_CAMERA_H

