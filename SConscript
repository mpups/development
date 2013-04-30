import os

LIB_NAME = 'robolib'
SONAME = 'librobolib.so'

NDKDIR = '/home/mark/code/android-ndk-r8e'
SYSROOT = NDKDIR + '/platforms/android-3/arch-arm'

STLPATH = '/home/mark/code/android-ndk-r8e/sources/cxx-stl/gnu-libstdc++/4.6'
STLINC = STLPATH + '/include'
STLBITS = STLPATH + '/libs/armeabi/include'

PLATFORMDIR = '/home/mark/code/android-ndk-r8e/platforms/android-8/arch-arm'
PLATFORMINC = PLATFORMDIR + '/usr/include/'

SRC_FILES  = Glob('src/packetcomms/*.cpp')
SRC_FILES += Glob('src/network/*.cpp')
SRC_FILES += Glob('src/robotcomms/*.cpp')

HEADER_FILES = Glob('src/packetcomms/*.h')
HEADER_FILES += Glob('src/network/*.h')
HEADER_FILES += Glob('src/robotcomms/*.h')

ANDROID_FFMPEG = '/home/mark/code/android-ffmpeg-build/armeabi'

INC_DIRS  = [
              '/usr/include/lua5.1',
              '/usr/include/freetype2',
              '/usr/local/videolib/include',
              '/usr/include/unicap',
              STLINC,
              STLBITS,
              '/home/mark/beagle_kernel_2.6.32/deploy/include', # Simply needed for freetype include - freetype not actually used
              PLATFORMINC
            ]

LIBDIRS = [ PLATFORMDIR + '/usr/lib',
            STLPATH + '/libs/armeabi',
            '/home/mark/code/videolib/builds/android_build/',
            ANDROID_FFMPEG + '/lib'
            ]
libs = [ 'avformat', 'avcodec', 'avutil', 'swscale', 'videolib', 'gnustl_shared' ]

cxx = 'arm-linux-androideabi-g++'
cxxflags = '-std=c++11 --sysroot=' + SYSROOT
defines = [ '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS', 'ANDROID' ]

env = Environment( ENV = {'PATH' : os.environ['PATH']} )
env.Append( CPPPATH=INC_DIRS )
env.Append( CPPDEFINES=defines )
env.Append( LINKFLAGS = [ '-Wl,--allow-shlib-undefined,--soname=' + SONAME ] )
env.Append( LIBPATH=LIBDIRS )
env.Append( LIBS=libs )
builtLibrary = env.SharedLibrary( target=LIB_NAME, source=SRC_FILES, CXX=cxx, CXXFLAGS=cxxflags )

INSTALL_PREFIX = '/home/mark/workspace/FFmpegJNI/jni/prebuilt/armeabi'
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

