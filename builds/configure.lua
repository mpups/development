-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '../../glk/include',
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/include/opencv'
}

LIB_DIRS = {
    '/use/local/lib',
    '../../glk/builds/linux_build/debug',
    '../../glk/builds/linux_build/release',
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = {}
LINKS = { 'lua5.1', 'freetype', 'pthread', 'rt', 'cxcore', 'cvaux', 'unicap', 'Xrender', 'X11', 'GL' }
GLK_LINKS = { 'glk', 'glkcore' }

