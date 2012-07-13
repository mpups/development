#ifndef ROBO_LIB_VISION_H
#define ROBO_LIB_VISION_H

#include "RoboLibCore.h"

#include "../src/video/CameraCapture.h"
#include "../src/video/UnicapCamera.h"
#include "../src/vision/Image.h"
#include "../src/vision/FastCornerThread.h"
#include "../src/vision/LoadBalancingCornerDetector.h"
#include "../src/vision/FastCornerSearch.h"

#include "../src/video/LibAvCapture.h"
#include "../src/video/LibAvWriter.h"
#include "../src/video/FFmpegCustomIO.h"
#include "../src/video/FFmpegSocketIO.h"

// These are not intended to be used on embedded ARM systems yet:
#ifndef ARM_BUILD

#include "../src/video/Dc1394Camera.h"

#include "../src/sse/ImageProcSse.h"
#include "../src/sse/ImageProc.h"

#include "../src/vision/Image.h"

#include "../src/visualisation/AnnotatedImage.h"

#endif

#endif // ROBO_LIB_VISION_H

