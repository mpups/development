-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '../../glk/include',
    '/usr/include/lua5.1',
    '/usr/local/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/local/include/opencv'
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = {}
LINKS = { 'robolib', 'glkcore', 'lua5.1', 'freetype', 'pthread', 'rt', 'cxcore' }
