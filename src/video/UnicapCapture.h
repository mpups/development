#ifndef UNICAPCAPTURE_H
#define UNICAPCAPTURE_H

#include "UnicapCamera.h"
#include "CameraCapture.h"

#include "FrameConverter.h"

/**
    Class which wraps a UnicapCamera providing it with the
    interface for CameraCapture objects.

    @deprecated For backwards compatibility only.
*/
class UnicapCapture : public CameraCapture
{
public:
    UnicapCapture();
    virtual ~UnicapCapture();

    bool IsOpen() const;
    void StartCapture();
    void StopCapture();

    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    int64_t GetFrameTimestamp_us() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t*, int );
    void ExtractRgbImage( uint8_t*, int );
    void ExtractBgrImage( uint8_t* img, int stride );

    uint8_t* UnsafeBufferAccess() { return m_buffer; };

protected:
    void FrameConversion( PixelFormat format, uint8_t* data, int stride );

private:
    void OnCapture( uint8_t* buffer, const timespec& time );

    UnicapCamera m_camera;
    uint8_t*     m_buffer;

    pthread_cond_t  m_cond;
    pthread_mutex_t m_mutex;
    uint32_t m_frameCount;
    uint32_t m_retrievedCount;
    volatile uint64_t m_time;

    FrameConverter m_converter;
};

#endif // UNICAPCAPTURE_H
