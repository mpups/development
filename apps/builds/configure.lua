-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '../../../glk/include',
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/include/opencv',
    '../../include'
}

LIB_DIRS = {
    '../../../glk/builds/linux_build/debug',
    '../../../glk/builds/linux_build/release',
    '../../builds/linux_build'
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = {}

OPENCV_LINKS = { 'ml', 'cvaux','highgui', 'cv', 'cxcore' }
OPENGL_LINKS = { 'Xrender', 'X11', 'GL' }
SYSTEM_LINKS = { 'pthread', 'rt' }
LINKS = { 'lua5.1', 'freetype', 'unicap' }

GLK_LINKS = { 'glk', 'glkcore' }

