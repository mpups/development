#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <stdint.h>

/**
    Abstract interface for all camera capture devices.
**/
class CameraCapture
{
public:
    CameraCapture() {};
    virtual ~CameraCapture() {};

    virtual bool IsAvailable() const = 0;
    virtual void StartCapture() = 0;
    virtual void StopCapture() = 0;

    virtual void GetFrame() = 0;
    virtual void DoneFrame() = 0;
    virtual int32_t GetFrameWidth() const = 0;
    virtual int32_t GetFrameHeight() const = 0;
    virtual uint64_t GetFrameTimestamp() const = 0;

    virtual uint64_t GetGuid() const = 0;
    virtual const char*   GetVendor() const = 0;
    virtual const char*   GetModel() const = 0;

    virtual void ExtractLuminanceImage( uint8_t* ) = 0;
    virtual void ExtractRgbImage( uint8_t* ) = 0;
    virtual void ExtractBgrImage( uint8_t* ) = 0;
};

#endif // CAMERA_CAPTURE_H

