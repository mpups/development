-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagleboardxm_gcc-4.8.1'

TOOLCHAIN = 'arm-none-linux-gnueabi-'
LIB_TYPE = 'SharedLib'

ARM_DEPLOYMENT = '/home/mark/beagleboardxm/deploy_final'

INCLUDE_DIRS = {
    '../../glk-1.0/include',
    '../../videolib/include',
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2',
    '/home/mark/code/cereal-0.9.1/include'
}

VIDEOLIB_DIR='/home/mark/code/videolib/builds/beagleboardxm_gcc-4.8.1'

LIB_DIRS = {
    './',
    ARM_DEPLOYMENT .. '/lib',
    VIDEOLIB_DIR
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '-std=c++11 -static -mtune=cortex-a8 -mfpu=neon' }
LINK_OPTIONS = { '-Wl,-rpath=/lib:/usr/local/lib' }

OPENCV_LINKS = {} --{ 'opencv_core', 'opencv_contrib' }
OPENGL_LINKS = {}
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }
FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = {}

GLK_LINKS = {} --{ 'glkcore','lua5.2', 'freetype' }
VIDEO_LINKS = { 'videolib','unicap' }

CONFIGURING_ARM = true

