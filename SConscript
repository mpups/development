import os
import utils

# TODO : This file contains all of the things I regularly want to do
# (e.g. build progs and libs for multiple platforms, excluding some files or progs on some platforms)
# Need to change site_scons scripts so they make all of this easy (ideally whole script should be only a few lines)

Import('env','target','compiler')

utils.IgnoreIfTargetNotSupported( target, ['native','beagle','android'] )

inc = []
if target == 'native':
    inc += [ '/usr/include/unicap',
             '/home/mark/tmp_installs/include' ]
    rpath = '/home/mark/tmp_installs/lib'
elif target == 'beagle':
    inc += [ compiler.sysroot + '/include',
             compiler.sysroot + '/include/unicap' ]
    rpath = '/lib:/usr/local/lib'
    env.Append( CPPDEFINES='ARM_BUILD' )
elif target == 'android':
    # Android is a little bitch:
    ANDROID_FFMPEG = '/home/mark/code/android-ffmpeg-build/armeabi'
    NDK_DIR = '/home/mark/code/android-ndk-r9b'
    ANDROID_PLATFORM = NDK_DIR + '/platforms/android-8/arch-arm'
    ANDROID_STL_PATH = NDK_DIR + '/sources/cxx-stl/gnu-libstdc++/4.8'
    ANDROID_STL_INC = ANDROID_STL_PATH + '/include'
    ANDROID_STL_BITS_INC = ANDROID_STL_PATH + '/libs/armeabi/include'
    inc += [ ANDROID_FFMPEG + '/include',
             ANDROID_PLATFORM + '/usr/include',
             ANDROID_STL_INC, ANDROID_STL_BITS_INC
              ]
    rpath = ANDROID_PLATFORM + '/usr/lib'
    env.Append( CPPDEFINES=['ANDROID', 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )
    env.Append( LIBPATH=[ANDROID_PLATFORM + '/usr/lib',ANDROID_STL_PATH+'/libs/armeabi',ANDROID_FFMPEG+'/lib'] )

libs = [
    'avformat', 'avcodec', 'avutil', 'swscale'
    ]

if target in ['native','beagle']:
    libs += ['pthread', 'rt', 'dl','unicap']

if target == 'native':
    libs += [ 'dc1394' ]

if target == 'android':
    libs += ['gnustl_shared']

sharedLibName = 'videolib'
soname = 'lib' + sharedLibName + '.so'

env.Append( CPPPATH=inc )
env.Append( LIBS=libs )
env.Append( CPPFLAGS = '-fPIC' )
env.Append( LINKFLAGS = [ '-Wl,--soname=' + soname + ',-rpath='+rpath ] )

libsrc = utils.RecursivelyGlobSourceInPaths( 'cpp', [ './src/video' ] )

if (target in ['beagle','android']):
    utils.RemoveFiles( libsrc, [ 'Dc1394Camera.cpp' ] )

if (target in ['android']):
    utils.RemoveFiles( libsrc, ['UnicapCapture.cpp','UnicapCamera.cpp'] )

gametoolLib = env.SharedLibrary( target=sharedLibName, source=libsrc )

