-- You can change these variables so that
-- they are appropriate for your system:

LIB_TYPE = 'SharedLib'

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '/home/mark/tmp_installs/include',
    '/usr/local/glk/include',
    '/usr/local/videolib/include',
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/home/mark/code/cereal-0.9.1/include'
}

LIB_DIRS = {
    '/home/mark/tmp_installs/lib',
    '/usr/lib',
    '/usr/local/glk/lib',
    '/usr/local/videolib/lib',
    '/usr/lib/x86_64-linux-gnu'
}

BUILD_OPTIONS = { '-std=c++0x -msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib:/usr/local/videolib/lib:/home/mark/tmp_installs/lib' }

OPENCV_LINKS = { 'opencv_imgproc','opencv_calib3d','opencv_highgui', 'opencv_core' }
OPENGL_LINKS = { 'Xrender', 'X11', 'GL' }
SYSTEM_LINKS = { 'pthread', 'rt' }
LINKS = { 'lua5.1', 'freetype', 'unicap' }
FFMPEG_LINKS = { 'avcodec', 'avformat', 'avutil', 'swscale' }
GLK_LINKS = { 'glk', 'glkcore' }
VIDEO_LINKS = { 'videolib' }

-- need these defines because ffmpeg libs use C99 standard macros which are not in any C++ standards.
DEFINES = { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }

