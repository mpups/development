import os
import utils

# Build an executable
def Program( ENV, NAME, SRC, SUPPORTED_PLATFORMS, CPPPATH='', LIBS='', LIBPATH='', RPATH='' ):
    if not utils.TargetIsSupported(ENV['platform'],SUPPORTED_PLATFORMS):
        return None
    env = ENV.Clone()
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( RPATH=RPATH )
    return env.Program( target=NAME, source=SRC )

# Build a shared library
def SharedLibrary( ENV, NAME, SRC, SUPPORTED_PLATFORMS, CPPPATH='', LIBS='', LIBPATH='', RPATH='' ):
    if not utils.TargetIsSupported(ENV['platform'],SUPPORTED_PLATFORMS):
        return None
    env = ENV.Clone()
    soname = 'lib' + NAME + '.so'
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( CPPFLAGS = '-fPIC' )
    env.Append( RPATH=RPATH )
    env.Append( LINKFLAGS = [ '-Wl,--soname=' + soname ] )
    return env.SharedLibrary( target=NAME, source=SRC )
