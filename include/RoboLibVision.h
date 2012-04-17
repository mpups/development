#ifndef ROBO_LIB_VISION_H
#define ROBO_LIB_VISION_H

#include "RoboLibCore.h"

#include "../src/video_capture/CameraCapture.h"
#include "../src/video_capture/UnicapCamera.h"
#include "../src/vision/Image.h"
#include "../src/vision/FastCornerThread.h"
#include "../src/vision/LoadBalancingCornerDetector.h"

#include "../src/video_capture/LibAvCapture.h"

// These are not intended to be used on embedded ARM systems yet:
#ifndef ARM_BUILD

#include "../src/video_capture/Dc1394Camera.h"

#include "../src/sse/ImageProcSse.h"
#include "../src/sse/ImageProc.h"

#include "../src/vision/Image.h"

#include "../src/visualisation/AnnotatedImage.h"

#endif

#endif // ROBO_LIB_VISION_H

