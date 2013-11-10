-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagle_build_g++4.8'

ARM_PACKAGES = '/home/mark/beagle_kernel_3.7/packages'
ARM_DEPLOYMENT = '/home/mark/beagle_kernel_3.7/deploy'
TOOLCHAIN = 'arm-linux-gnueabi-'
LIB_TYPE = 'SharedLib'

INCLUDE_DIRS = {
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
}

LIB_DIRS = {
    ARM_DEPLOYMENT .. '/lib',
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '--sysroot=/usr/arm-linux-gnueabi -std=c++0x -static -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp' }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/lib:/usr/local/lib' }

FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = { 'unicap' }
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }

CONFIGURING_ARM = true
