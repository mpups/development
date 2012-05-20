-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '/usr/local/glk/include',
    '../../include', -- Robolib includes.
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/include/dc1394',
}

LIB_DIRS = {
    --'/home/mark/tmp_installs/lib', -- has to come first so libav links to my own installation
    '/usr/lib',
    '/usr/lib/x86_64-linux-gnu',
    '/usr/local/lib',
    '/usr/local/glk/lib',
    '../../builds/linux_build', -- robolib libraries built here
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib' }
LINKS = { 'pthread', 'rt', 'Xrender', 'X11', 'GL', 'freetype', 'lua5.1', 'dc1394', 'opencv_imgproc', 'opencv_video', 'opencv_calib3d', 'opencv_core', 'unicap', 'avcodec', 'avformat', 'avutil', 'swscale' }

-- need these defines because ffmpeg libs use C99 standard macros which are not in any C++ standards.
DEFINES = { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }

