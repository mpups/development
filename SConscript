import os
import utils
import build

# Standard initialisation:
Import('env','compiler')
target = env['platform']

# Platform dependent includes
includeMap = {
    'native' : ['/home/mark/tmp_installs/include',
                '/usr/local/glk/include',
                '/usr/local/videolib/include',
                '/usr/include/lua5.2',
                '/usr/include/freetype2',
                '/usr/include/gtest',
                '/usr/include/',
                '/usr/include/unicap',
                '#cereal-1.0.0/include'],
    'beagle' : ['#videolib/include',
                compiler.sysroot + '/include',
                compiler.sysroot + '/include/unicap',
                '#cereal-1.0.0/include'],
    'android' : ['#videolib/include',
                 '/home/mark/code/android-ffmpeg-build/armeabi/include',
                 '#cereal-1.0.0/include']
}
inc = includeMap[target]

libPathMap = {
    'native' : ['/home/mark/tmp_installs/lib',
                '/usr/lib',
                '/usr/local/glk/lib',
                '/usr/local/videolib/lib',
                '/usr/lib/x86_64-linux-gnu',
                '/home/mark/code/free_type_cpp/build'],
    'beagle' : ['#build/beagle/videolib/release'],
    'android' : ['#build/android/videolib/release']
}
libPath = libPathMap[target]

# Platform dependent rpath:
rpathMap = {
    'native' : ['/lib', '/usr/local/lib', '/usr/local/videolib/lib'],
    'beagle' : ['/lib', '/usr/local/lib'],
    'android' : []
}
rpath = rpathMap[target]

src = utils.RecursivelyGlobSourceInPaths( 'cpp', [ './src/packetcomms','./src/network','src/robotcomms' ] )

# Android is a PIA of course:
if target == 'android':
    env.Append( CPPDEFINES=['__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )

robolib = build.SharedLibrary(ENV=env,
                              NAME='robolib',
                              RPATH=rpath,
                              CPPPATH=inc,
                              LIBS=['videolib'],
                              LIBPATH=libPath,
                              SRC=src,
                              SUPPORTED_PLATFORMS=['native','beagle','android']
)

