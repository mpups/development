import os
import utils
import build

# Standard initialisation:
Import('env','compiler')
target = env['platform']
installPath = os.path.join(env['installPath'],'usr','local')

# Platform dependent includes
includeMap = {
    'native' : ['/home/mark/tmp_installs/include',
                '/usr/local/glk/include',
                '/usr/include/lua5.2',
                '/usr/include/gtest',
                '/usr/include/',
                '/usr/include/unicap',
                '#videolib/include',
                '#cereal-1.0.0/include'],
    'beagle' : [compiler.sysroot + '/include',
                compiler.sysroot + '/include/unicap',
                '#cereal-1.0.0/include',
                '#videolib/include',
                '#free_type_cpp/include'],
    'android' : ['#videolib/include',
                 '/home/mark/code/android-ffmpeg-build/armeabi/include',
                 '#cereal-1.0.0/include']
}
inc = includeMap[target]

localLibPaths = [os.path.join(installPath,'lib')]
libPathMap = {
    'native' : ['/home/mark/tmp_installs/lib',
                '/usr/lib',
                '/usr/local/glk/lib',
                '/usr/lib/x86_64-linux-gnu',
                '/home/mark/code/free_type_cpp/build'],
    'beagle' : [],
    'android' : []
}
libPath = libPathMap[target] + localLibPaths

# Platform dependent rpath:
rpathMap = {
    'native' : ['/lib', '/usr/local/lib'],
    'beagle' : ['/lib', '/usr/local/lib'],
    'android' : []
}
rpath = rpathMap[target]

src = utils.RecursivelyGlobSourceInPaths( 'cpp', [ 'src/packetcomms','src/network','src/robotcomms','src/motor_control','src/io/linux' ] )

# Android is a PIA of course:
if target == 'android':
    env.Append( CPPDEFINES=['__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS'] )
    utils.RemoveFiles(src,['src/io/linux/Joystick.cpp'])

robolib = build.SharedLibrary(ENV=env,
                              NAME='robolib',
                              SRC=src,
                              SUPPORTED_PLATFORMS=['native','beagle','android'],
                              DEPS=['freetype2'],
                              RPATH=rpath,
                              CPPPATH=inc,
                              LIBS=['videolib'],
                              LIBPATH=libPath
)

# Programs:
progLibs = ['pthread','videolib','freetypecpp']
if target=='native':
    progLibs += ['glk','glkcore']
progLibs += ['robolib']

if target=='beagle':
    env.Append(LINKFLAGS='-Wl,--allow-shlib-undefined')

commsSrc = Glob('./src/puppybot/*.cpp')
utils.RemoveFiles(commsSrc,['src/puppybot/puppybot_test.cpp'])
robotcomms = build.Program(ENV=env,
                           NAME='puppybot-comms',
                           SRC=commsSrc,
                           SUPPORTED_PLATFORMS=['native','beagle'],
                           DEPS=['freetype2'],
                           CPPPATH=inc,
                           LIBS=progLibs,
                           LIBPATH=libPath + ['/usr/local/glk/lib'],
                           RPATH=rpath + localLibPaths + ['/usr/local/glk/lib']
)

env.Alias( 'install',
    env.Install( os.path.join( installPath,'lib' ), robolib ),
    env.Install( os.path.join( installPath,'bin' ), robotcomms )
)

