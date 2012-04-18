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
    '/usr/include/opencv',
    '/usr/include/dc1394',
}

LIB_DIRS = {
    '/home/mark/tmp_installs/lib', -- has to come first so libav links to my own installation
    '/usr/lib',
    '/usr/local/lib',
    '/usr/local/glk/lib',
    '../../builds/linux_build', -- robolib libraries built here
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib:/home/mark/tmp_installs/lib' }
LINKS = { 'pthread', 'rt', 'Xrender', 'X11', 'GL', 'freetype', 'lua5.1', 'dc1394', 'cxcore', 'cv', 'highgui', 'unicap', 'avcodec', 'avformat', 'avutil', 'swscale' }
