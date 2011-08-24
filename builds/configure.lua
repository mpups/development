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

OPENCV_LINKS = { 'cxcore', 'cvaux' }
OPENGL_LINKS = { 'Xrender', 'X11', 'GL' }
SYSTEM_LINKS = { 'pthread', 'rt' }

function append( A, B )
  for i,v in pairs(B) do table.insert(A,v) end
end

-- Append individual link groups together so we can use LINKS to link to all libs:
LINKS = { 'lua5.1', 'freetype', 'unicap' }
append( LINKS, OPENCV_LINKS )
append( LINKS, OPENGL_LINKS )
append( LINKS, SYSTEM_LINKS )

--for i,v in pairs(LINKS) do print(i,v) end

GLK_LINKS = { 'glk', 'glkcore' }

