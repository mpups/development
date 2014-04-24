import os
import utils

# Build an executable
def Program( ENV, NAME, CPPPATH, LIBS, RPATH, SRC ):
    env = ENV.Clone()
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LINKFLAGS = [ '-Wl,-rpath=' + RPATH ] )
    return env.Program( target=NAME, source=SRC )

# Build a shared library
def SharedLibrary( ENV, NAME, CPPPATH, LIBS, LIBPATH, RPATH, SRC ):
    env = ENV.Clone()
    soname = 'lib' + NAME + '.so'
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( CPPFLAGS = '-fPIC' )
    env.Append( LINKFLAGS = [ '-Wl,--soname=' + soname + ',-rpath=' + RPATH ] )
    return env.SharedLibrary( target=NAME, source=SRC )
