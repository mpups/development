import os
import utils
import build

# Standard initialisation:
Import('env','compiler')
target = env['platform']
installPath = os.path.join(env['installPath'],'usr','local')
libPath = [os.path.join(installPath,'lib')]

src = utils.RecursivelyGlobSourceInPaths( 'cpp', [ './src/video' ] )

# Arrange platform dependent stuff:
deps = ['ffmpeg']
if target in ['native','beagle']:
    deps += ['unicap']
if target in ['beagle','android']:
    utils.RemoveFiles( src, [ 'Dc1394Camera.cpp' ] )
if target in ['android']:
    utils.RemoveFiles( src, ['UnicapCapture.cpp','UnicapCamera.cpp'] )
    env.Append( CPPDEFINES=['__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )
if target in ['native']:
    deps += ['dc1394']

videolib = build.SharedLibrary(
    ENV=env,
    NAME='videolib',
    SRC=src,
    SUPPORTED_PLATFORMS=['native','beagle','android'],
    DEPS=deps
)

# capture test program - only supported on native builds
env.Append(LIBPATH=['/usr/local/lib','./'])
capture = build.Program(
    ENV=env,
    NAME='capture',
    CPPPATH=['#videolib/include', '/usr/local/glk/include'],
    LIBS= ['videolib'],
    SRC='./src/tests/tools/capture.cpp',
    SUPPORTED_PLATFORMS=['native'],
    DEPS=['glk','freetype2'] + deps,
    RPATH=libPath
)

# GoogleTest unit tests:
test = build.Program(
    ENV=env,
    NAME='utest',
    LIBS = ['videolib'],
    SRC=['./src/tests/unit/gtests.cpp','./src/tests/unit/VideoTests.cpp'],
    SUPPORTED_PLATFORMS=['native','beagle'],
    DEPS=deps+['gtest']
)

# Installing libs/executables is easy:
if installPath:
    installLib = env.Install( os.path.join( installPath, 'lib' ), videolib )
    installCapture = env.Install( os.path.join( installPath, 'bin' ), capture )

    # Installing the headers is a PIA:
    installHeaders = utils.GenerateHeaderInstallActions(env,installPath,'include','videolib')
    installSourceHeaders = utils.GenerateHeaderInstallActions(env,installPath,'src','videolib')
    env.Alias( 'install', [ installLib, installCapture, installHeaders, installSourceHeaders] )
