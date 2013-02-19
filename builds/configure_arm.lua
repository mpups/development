-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagle_build_g++4.6'

ARM_DEPLOYMENT = '/home/mark/code/beagle_builds_g++4.6/deploy'
TOOLCHAIN = 'arm-linux-gnueabi-'
LIB_TYPE = 'SharedLib'

INCLUDE_DIRS = {
    '../../glk/include',
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2'
}

LIB_DIRS = {
    '../../glk/premake/beagle_build/debug',
    '../../glk/premake/beagle_build/release',
    ARM_DEPLOYMENT .. '/lib',
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '-std=c++0x -static -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp' }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/usr/local/lib:/home/mark/code/beagle_builds_g++4.6/deploy' }

FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = { 'lua', 'freetype', 'unicap' }
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }

GLK_LINKS = { 'glkcore' }

CONFIGURING_ARM = true
