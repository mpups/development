-- You can change these variables so that
-- they are appropriate for your system:

--TARGET_DIR   = 'beagle_build_g++4.6'
TARGET_DIR   = 'android_build'

ARM_PACKAGES = '/home/mark/beagle_kernel_2.6.32/packages'
ARM_DEPLOYMENT = '/home/mark/beagle_kernel_2.6.32/deploy'
--TOOLCHAIN = 'arm-linux-gnueabi-'
TOOLCHAIN = 'arm-linux-androideabi-'
LIB_TYPE = 'SharedLib'

INCLUDE_DIRS = {
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2',
    ARM_PACKAGES .. '/lua-5.1.4/src'
}

LIB_DIRS = {
    ARM_DEPLOYMENT .. '/lib',
    ARM_PACKAGES .. '/lua-5.1.4/src'
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '--sysroot=/home/mark/code/android-ndk-r8b/platforms/android-3/arch-arm -std=c++0x -static -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp' }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/usr/local/lib' }

FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = { 'lua', 'freetype', 'unicap' }
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }

CONFIGURING_ARM = true
