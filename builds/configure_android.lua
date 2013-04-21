-- You can change these variables so that
-- they are appropriate for your system

TARGET_DIR   = 'android_build'

NDK_DIR = '/home/mark/code/android-ndk-r8e'
SYS_ROOT = NDK_DIR .. '/platforms/android-3/arch-arm'

TOOLCHAIN = 'arm-linux-androideabi-'
LIB_TYPE = 'SharedLib'

SONAME = 'libvideolib.so'

ANDROID_STL_PATH = '/home/mark/code/android-ndk-r8e/sources/cxx-stl/gnu-libstdc++/4.6'
ANDROID_STL_INC = ANDROID_STL_PATH .. '/include'
ANDROID_STL_BITS_INC = ANDROID_STL_PATH .. '/libs/armeabi/include'
ANDROID_STL_LIBDIR = ANDROID_STL_PATH .. '/libs/armeabi/'
ANDROID_PLATFORM = '/home/mark/code/android-ndk-r8e/platforms/android-8/arch-arm'
ANDROID_PLATFORM_LIBS = ANDROID_PLATFORM .. '/usr/lib'

ANDROID_FFMPEG = '/home/mark/code/android-ffmpeg-build/armeabi'

INCLUDE_DIRS = {
    ANDROID_FFMPEG .. '/include',
    ANDROID_PLATFORM .. '/usr/include',
    ANDROID_STL_INC,
    ANDROID_STL_BITS_INC,
}

LIB_DIRS = {
    ANDROID_PLATFORM_LIBS,
    ANDROID_FFMPEG .. '/lib',
}

DEFINES = { 'ANDROID', 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '--sysroot=' .. SYS_ROOT .. ' -std=c++0x -mfloat-abi=softfp' }
LINK_OPTIONS = { '-Wl,-soname=' .. SONAME .. ',-rpath-link=' .. ANDROID_PLATFORM_LIBS .. ' -L' .. ANDROID_PLATFORM_LIBS .. ' -L' .. ANDROID_STL_LIBDIR }

FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = {}
SYSTEM_LINKS = { 'gnustl_shared' }

CONFIGURING_ARM     = true
CONFIGURING_ANDROID = true

-- Workaround for Android toolchain bug:
os.execute( 'ln -s ' .. ANDROID_PLATFORM_LIBS .. '/crtbegin_so.o ' .. TARGET_DIR .. '/crtbegin_so.o' )
os.execute( 'ln -s ' .. ANDROID_PLATFORM_LIBS .. '/crtend_so.o ' .. TARGET_DIR .. '/crtend_so.o' )

