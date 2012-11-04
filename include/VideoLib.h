#ifndef VIDEO_LIB_H
#define VIDEO_LIB_H

#include "../src/video/CameraCapture.h"
#include "../src/video/UnicapCamera.h"

#include "../src/video/video_conversion.h"
#include "../src/video/VideoFrame.h"
#include "../src/video/LibAvCapture.h"
#include "../src/video/LibAvWriter.h"
#include "../src/video/FFmpegCustomIO.h"
#include "../src/video/FFmpegSocketIO.h"
#include "../src/video/FFmpegBufferIO.h"

#include "../src/io/Ipv4Address.h"
#include "../src/io/Socket.h"
#include "../src/io/TcpSocket.h"
#include "../src/io/UdpSocket.h"

#ifndef ARM_BUILD
// These components are not ready to be used on
// ARM systems yet:

#include "../src/video/Dc1394Camera.h"

#endif

#endif // VIDEO_LIB_H

