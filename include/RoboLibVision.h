#ifndef ROBO_LIB_VISION_H
#define ROBO_LIB_VISION_H

#include "RoboLibCore.h"

#include "../src/camera_capture/CameraCapture.h"
#include "../src/camera_capture/UnicapCamera.h"
#include "../src/vision/Image.h"

#ifndef ARM_BUILD
#include "../src/camera_capture/Dc1394Camera.h"
#include "../src/sse/ImageProcSse.h"
#include "../src/sse/ImageProc.h"
#endif

#endif // ROBO_LIB_VISION_H

