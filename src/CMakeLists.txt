
project( videolib )

add_library( videolib SHARED
    video/CameraCapture.h
    video/Capture.h
    video/Dc1394Camera.cpp
    video/Dc1394Camera.h
    video/FFmpegStdFunctionIO.h
    video/FFmpegStdFunctionIO.cpp
    video/FFmpegCustomIO.cpp
    video/FFmpegCustomIO.h
    video/FrameConverter.cpp
    video/FrameConverter.h
    video/LibAvCapture.cpp
    video/LibAvCapture.h
    video/LibAvVideoStream.cpp
    video/LibAvVideoStream.h
    video/LibAvWriter.cpp
    video/LibAvWriter.h
    video/UnicapCamera.cpp
    video/UnicapCamera.h
    video/UnicapCapture.cpp
    video/UnicapCapture.h
    video/video_conversion.cpp
    video/video_conversion.h
    video/VideoFrame.cpp
    video/VideoFrame.h
)

target_link_libraries( videolib ${LIBS} )
install( TARGETS videolib DESTINATION lib )
