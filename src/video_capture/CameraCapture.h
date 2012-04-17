#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <stdint.h>

#include "Capture.h"

/**
    Abstract interface for all camera capture devices.

    Inherits from Capture and defines extra pure virtual methods
    which are only relevant for live capture from a camera.
**/
class CameraCapture : public Capture
{
public:
    CameraCapture() {};
    virtual ~CameraCapture() {};

    virtual void StartCapture() = 0;
    virtual void StopCapture() = 0;

    virtual uint64_t GetGuid() const = 0;
    virtual const char*   GetVendor() const = 0;
    virtual const char*   GetModel() const = 0;
};

#endif // CAMERA_CAPTURE_H

