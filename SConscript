import os

LIB_NAME = 'freetypecpp'
SONAME = 'libfreetypecpp.so'

HEADER_FILES = Glob('src/*.h')
SRC_FILES  = Glob('src/*.cpp')

INC_DIRS  = [ '/usr/include/freetype2' ]
LIBDIRS = [ '/usr/lib/x86_64-linux-gnu/' ]

libs = [ 'freetype' ]

cxx = '/usr/bin/c++'
cxxflags = '-std=c++11 -O3'

env = Environment( ENV = {'PATH' : os.environ['PATH']} )
env.Append( CPPPATH=INC_DIRS )
env.Append( LINKFLAGS = [ '-Wl,--soname=' + SONAME ] )
env.Append( LIBPATH=LIBDIRS )
env.Append( LIBS=libs )
builtLibrary = env.SharedLibrary( target=LIB_NAME, source=SRC_FILES, CXX=cxx, CXXFLAGS=cxxflags )

INSTALL_PREFIX = '/home/mark/tmp_installs/lib'
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

