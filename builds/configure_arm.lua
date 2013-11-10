-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagle_build_g++4.8'

TOOLCHAIN = 'arm-linux-gnueabi-'
LIB_TYPE = 'SharedLib'

ARM_DEPLOYMENT = '/home/mark/beagle_kernel_3.7/deploy'

INCLUDE_DIRS = {
    '../../glk-1.0/include',
    '../../videolib/include',
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2',
    '/home/mark/code/cereal-0.9.1/include'
}

LIB_DIRS = {
    './',
    ARM_DEPLOYMENT .. '/lib',
    ARM_DEPLOYMENT .. '/usr/local/lib', -- THIS IS STUPID, KEEP PICKING UP OLD VERSION OF THE LIBRARY I AM BUILDING
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '-std=c++11 -static -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp --sysroot=' .. ARM_DEPLOYMENT }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/lib:/usr/local/lib' }

OPENCV_LINKS = { 'opencv_core', 'opencv_contrib' }
OPENGL_LINKS = {}
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }
FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = {}

GLK_LINKS = { 'glkcore', 'lua5.2', 'freetype' }
VIDEO_LINKS = { 'videolib','unicap' }

CONFIGURING_ARM = true

