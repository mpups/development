import os
import utils
import deps

# Build an executable
def Program( ENV, NAME, SRC, SUPPORTED_PLATFORMS, DEPS=[], CPPPATH=[], LIBS=[], LIBPATH=[], RPATH=[] ):
    if not utils.TargetIsSupported(ENV['platform'],SUPPORTED_PLATFORMS):
        return None
    env = ENV.Clone()
    deps.List(env,DEPS)
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( RPATH=RPATH )
    return env.Program( target=NAME, source=SRC )

# Build a shared library
def SharedLibrary( ENV, NAME, SRC, SUPPORTED_PLATFORMS, DEPS=[], CPPPATH=[], LIBS=[], LIBPATH=[], RPATH=[] ):
    if not utils.TargetIsSupported(ENV['platform'],SUPPORTED_PLATFORMS):
        return None
    env = ENV.Clone()
    deps.List(env,DEPS)
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( CPPFLAGS = '-fPIC' )
    env.Append( RPATH=RPATH )

    soname = 'lib' + NAME + '.so'
    env.Append( LINKFLAGS = [ '-Wl,--soname=' + soname ] )
    return env.SharedLibrary( target=NAME, source=SRC )

# Build a shared library
def StaticLibrary( ENV, NAME, SRC, SUPPORTED_PLATFORMS, DEPS=[], CPPPATH=[], LIBS=[], LIBPATH=[], RPATH=[] ):
    if not utils.TargetIsSupported(ENV['platform'],SUPPORTED_PLATFORMS):
        return None
    env = ENV.Clone()
    deps.List(env,DEPS)
    env.Append( CPPPATH=CPPPATH )
    env.Append( LIBS=LIBS )
    env.Append( LIBPATH=LIBPATH )
    env.Append( RPATH=RPATH )
    return env.StaticLibrary( target=NAME, source=SRC )
