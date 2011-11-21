-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'arm_build'

ARM_DEPLOYMENT = '/home/mark/code/beagle_builds/deploy'
TOOLCHAIN = 'arm-angstrom-linux-gnueabi-'

INCLUDE_DIRS = {
    '../../glk/include',
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2'
}

LIB_DIRS = {
    '../../glk/builds/beagle_build/debug',
    '../../glk/builds/beagle_build/release',
    ARM_DEPLOYMENT .. '/lib',
}

DEFINES = { 'ARM_BUILD' }
BUILD_OPTIONS = { '-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp' }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/usr/local/lib' }

OPENCV_LINKS = { 'opencv_core', 'opencv_contrib' }
OPENGL_LINKS = {}
SYSTEM_LINKS = { 'pthread', 'rt' }
LINKS = { 'lua5.1', 'freetype', 'unicap' }

GLK_LINKS = { 'glkcore' }

CONFIGURING_ARM = 1
