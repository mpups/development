import os
Import('env','target','compiler')

# Different build types have different include paths for some libs:
cppIncludes = [ '/usr/include','/usr/include/freetype2' ]
if ( target == "beagle" ):
    cppIncludes = [ os.path.join(compiler.sysroot, 'include/freetype2') ]
env.Append( CPPPATH=cppIncludes )

LIB_NAME = 'freetypecpp'
SONAME   = 'lib' + LIB_NAME + '.so'
LINKS = [ 'freetype' ]
env.Append( LIBS=LINKS )
env.Append( LINKFLAGS = [ '-Wl,--soname=' + SONAME ] )

SRC_FILES    = Glob('src/*.cpp')
builtLibrary = env.SharedLibrary( target=LIB_NAME, source=SRC_FILES )

INSTALL_PREFIX = os.path.join( compiler.sysroot, 'usr/local' )
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

