import os
import compilers

LIB_NAME = 'freetypecpp'
SONAME = 'libfreetypecpp.so'
HEADER_FILES = Glob('src/*.h')
SRC_FILES  = Glob('src/*.cpp')
libs = [ 'freetype' ]

# Setup the compiler and environment:
target = GetOption('build-for')
compiler = compilers.makeCompilerFor(target)
print "Building for target '" + target + "':"

compiler.AppendFlags( "-O3 -Wall -pedantic -Werror" )

INC_DIRS = [ '/usr/include','/usr/include/freetype2' ]
if ( target == "beagle" ):
    INC_DIRS = [ compiler.sysroot + '/include/freetype2' ]

LIBDIRS = [ compiler.sysroot + '/lib' ]

env = Environment( ENV = {'PATH' : compiler.path} )
env.Append( CPPPATH=INC_DIRS )
env.Append( LINKFLAGS = [ '-Wl,--soname=' + SONAME ] )
env.Append( LIBPATH=LIBDIRS )
env.Append( LIBS=libs )

builtLibrary = env.SharedLibrary( target=LIB_NAME, source=SRC_FILES, CXX=compiler.cmd, CXXFLAGS=compiler.flags )

INSTALL_PREFIX = os.path.join( compiler.sysroot, 'usr/local' )
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

