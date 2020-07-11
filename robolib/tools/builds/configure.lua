-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '/home/mark/tmp_installs/include',
    '/usr/local/glk/include',
    '/usr/local/videolib/include',
    '/usr/include/lua5.2',
    '../../include', -- Robolib includes.
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/home/mark/code/cereal-0.9.1/include',
    '/usr/include/dc1394',
}

LIB_DIRS = {
    '/home/mark/tmp_installs/lib', -- has to come first so libav links to my own installation
    '/usr/lib',
    '/usr/local/lib',
    '/usr/local/glk/lib',
    '/usr/local/videolib/lib',
    '/usr/lib/x86_64-linux-gnu',
    '/home/mark/code/free_type_cpp/build',
    '../../builds/linux_build', -- Robolib includes.
}

BUILD_OPTIONS = { '-std=c++11 -msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib:/usr/local/videolib/lib:/home/mark/tmp_installs/lib:/home/mark/code/free_type_cpp/build' }

OPENCV_LINKS = { 'opencv_imgproc','opencv_video','opencv_calib3d','opencv_highgui', 'opencv_core' }
OPENGL_LINKS = { 'Xrender', 'X11', 'GL' }
SYSTEM_LINKS = { 'pthread', 'rt' }
LINKS = { 'lua5.2', 'freetype', 'unicap' }
FFMPEG_LINKS = { 'avcodec', 'avformat', 'avutil', 'swscale' }
GLK_LINKS = { 'glk', 'glkcore' }
VIDEO_LINKS = { 'videolib' }

-- need these defines because ffmpeg libs use C99 standard macros which are not in any C++ standards.
DEFINES = { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }

