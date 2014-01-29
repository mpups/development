-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagleboardxm_gcc-4.8.2'

ARM_PACKAGES = '/home/mark/beagleboardxm/build_packages_gcc-4.8.2'
ARM_DEPLOYMENT = '/home/mark/beagleboardxm/deploy_gcc-4.8.2'
TOOLCHAIN = 'arm-linux-gnueabihf-'
LIB_TYPE = 'SharedLib'

INCLUDE_DIRS = {
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
}

LIB_DIRS = {
    ARM_DEPLOYMENT .. '/lib',
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '-std=c++0x -static -mtune=cortex-a8 -mfpu=neon' }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/lib:/usr/local/lib' }

FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = { 'unicap' }
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }

CONFIGURING_ARM = true
