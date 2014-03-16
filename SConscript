import os

LIB_NAME = 'freetypecpp'
SONAME = 'libfreetypecpp.so'

HEADER_FILES = Glob('src/*.h')
SRC_FILES  = Glob('src/*.cpp')

SYSROOT = '/home/mark/beagleboardxm/deploy_final'

INC_DIRS  = [ SYSROOT + '/include/freetype2' ]
LIBDIRS = [ SYSROOT + '/lib' ]

libs = [ 'freetype' ]

cxx = 'arm-none-linux-gnueabi-g++'
cxxflags = '-std=c++11 -O3'

path='/home/mark/x-tools/arm-none-linux-gnueabi/bin:/bin'

env = Environment( ENV = {'PATH' : path} )
env.Append( CPPPATH=INC_DIRS )
env.Append( LINKFLAGS = [ '-Wl,--soname=' + SONAME ] )
env.Append( LIBPATH=LIBDIRS )
env.Append( LIBS=libs )
builtLibrary = env.SharedLibrary( target=LIB_NAME, source=SRC_FILES, CXX=cxx, CXXFLAGS=cxxflags )

INSTALL_PREFIX = os.path.join( SYSROOT, 'usr/local' )
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

