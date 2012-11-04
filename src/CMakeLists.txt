
project( videolib )

add_library( videolib SHARED
    io/Ipv4Address.cpp
    io/Ipv4Address.h
    io/SerialPort.h
    io/Socket.cpp
    io/Socket.h
    io/TcpSocket.cpp
    io/TcpSocket.h
    io/UdpSocket.cpp
    io/UdpSocket.h
    video/CameraCapture.h
    video/Capture.h
    video/Dc1394Camera.cpp
    video/Dc1394Camera.h
    video/FFmpegBufferIO.cpp
    video/FFmpegBufferIO.h
    video/FFmpegCustomIO.cpp
    video/FFmpegCustomIO.h
    video/FFmpegSocketIO.cpp
    video/FFmpegSocketIO.h
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
    video/video_conversion.cpp
    video/video_conversion.h
    video/VideoFrame.cpp
    video/VideoFrame.h
)

target_link_libraries( videolib ${LIBS} )
install( TARGETS videolib DESTINATION lib )

