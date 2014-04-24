import os
import utils
import build

# Standard initialisation:
Import('env','compiler')
target = env['platform']

# Platform dependent includes
includeMap = {
    'native' : [ '/usr/include/unicap', '/home/mark/tmp_installs/include' ],
    'beagle' : [ compiler.sysroot + '/include', compiler.sysroot + '/include/unicap' ],
    'android' : [ '/home/mark/code/android-ffmpeg-build/armeabi/include' ]
}
inc = includeMap[target]

# Platform dependent libraries
libs = [ 'avformat', 'avcodec', 'avutil', 'swscale' ]
libMap = {
    'native' : ['unicap', 'dc1394'],
    'beagle' : ['unicap'], # These were linked before but not sure they were necessary: 'pthread', 'rt', 'dl',
    'android' : []
}
libs += libMap[target]

# Platform dependent rpath:
rpathMap = {
    'native' : '/home/mark/tmp_installs/lib',
    'beagle' : '/lib:/usr/local/lib',
    'android' : ''
}
rpath = rpathMap[target]

# platform dependent libpaths:
libpathMap = {
    'native' : ['/home/mark/tmp_installs/lib'],
    'beagle' : [],
    'android' : ['/home/mark/code/android-ffmpeg-build/armeabi/lib']
}
libpath = libpathMap[target]

# Android is a PIA of course:
if target == 'android':
    env.Append( CPPDEFINES=['__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )

src = utils.RecursivelyGlobSourceInPaths( 'cpp', [ './src/video' ] )

# Platform dependent source filters:
if (target in ['beagle','android']):
    utils.RemoveFiles( src, [ 'Dc1394Camera.cpp' ] )
if (target in ['android']):
    utils.RemoveFiles( src, ['UnicapCapture.cpp','UnicapCamera.cpp'] )

videolib = build.SharedLibrary(ENV=env,
                               NAME='videolib',
                               RPATH=rpath,
                               CPPPATH=inc,
                               LIBS=libs,
                               LIBPATH=libpath,
                               SRC=src,
                               SUPPORTED_PLATFORMS=['native','beagle','android']
)

# capture test program - only supported on native builds
inc += ['#videolib/include', '/usr/local/glk/include', '/usr/include/freetype2']
libs += ['videolib','glk','glkcore']
env.Append(LIBPATH=['/usr/local/lib','/usr/local/glk/lib','./'])
capture = build.Program(ENV=env,
                        NAME='capture',
                        CPPPATH=inc,
                        LIBS=libs,
                        RPATH='/usr/local/glk/lib',
                        SRC='./src/tests/tools/capture.cpp',
                        SUPPORTED_PLATFORMS=['native'])

