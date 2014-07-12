import os
import utils
import build

# Standard initialisation:
Import('env','compiler')
target = env['platform']

installPath = os.path.join(env['installPath'],'usr','local')
libPath = [os.path.join(installPath,'lib')]

inc = ['#free_type_cpp/include','#videolib/include']
src = utils.RecursivelyGlobSourceInPaths( 'cpp', [ 'src/packetcomms','src/network','src/robotcomms','src/motor_control','src/io/linux' ] )

# Android is a PIA of course:
if target == 'android':
    env.Append( CPPDEFINES=['__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )
    utils.RemoveFiles(src,['src/io/linux/Joystick.cpp'])

deps = []
if target in ['beagle', 'native']:
    deps += ['freetype2','unicap']
if target == 'native':
    deps += ['glk']
deps += ['cereal','ffmpeg']

robolib = build.SharedLibrary(ENV=env,
                              NAME='robolib',
                              SRC=src,
                              SUPPORTED_PLATFORMS=['native','beagle','android'],
                              DEPS=deps,
                              CPPPATH=inc,
                              LIBS=['videolib'],
                              LIBPATH=libPath,
                              RPATH=libPath
)

# Programs:
progLibs = ['pthread','videolib','freetypecpp','robolib']

commsSrc = Glob('./src/puppybot/*.cpp')
utils.RemoveFiles(commsSrc,['src/puppybot/puppybot_test.cpp'])
robotcomms = build.Program(ENV=env,
                           NAME='puppybot-comms',
                           SRC=commsSrc,
                           SUPPORTED_PLATFORMS=['native','beagle'],
                           DEPS=deps,
                           CPPPATH=inc,
                           LIBS=progLibs,
                           LIBPATH=libPath,
                           RPATH=libPath
)

env.Alias( 'install',
    env.Install( os.path.join( installPath,'lib' ), robolib ),
    env.Install( os.path.join( installPath,'bin' ), robotcomms )
)

