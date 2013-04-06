// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef UNICAP_CAMERA_H
#define UNICAP_CAMERA_H

#include <stdint.h>

#include <string>
#include <functional>

extern "C" {
#include <unicap.h>
}

/**
    Class for accessing USB cameras through libunicap.

**/
class UnicapCamera
{
public:
    typedef  std::function< void( uint8_t* buffer, const timespec& ) > CaptureFunction;

    UnicapCamera( uint64_t guid = 0 );
    virtual ~UnicapCamera();

    bool IsOpen() const;
    void StartCapture();
    void StopCapture();

    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t*, int );

    std::size_t GetFormatBufferSize() const;

    void SetCaptureCallback( CaptureFunction&& callback );

protected:

private:
    bool OpenDevice();
    bool FindFormat( int width, int height, uint32_t fourcc, unicap_format_t& format );
    void EnumerateProperties();
    void SetDefaultProperties();

    static void NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data );

    unicap_handle_t m_handle;
    uint32_t        m_width;
    uint32_t        m_height;

    std::function< void( uint8_t* buffer, const timespec& ) > m_captureCallback;

    uint64_t        m_guid;
    std::string     m_vendor;
    std::string     m_model;
};

#endif // UNICAP_CAMERA_H

