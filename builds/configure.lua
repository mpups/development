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

EXTRA_BUILD_OPTIONS = { '-msse -msse2' }
