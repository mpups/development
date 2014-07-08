import os
import utils
import build

Import( 'env', 'compiler' )
target = env['platform']

utils.EndScriptIfTargetNotSupported( target, ['native','beagle'] )

builtLibrary = build.SharedLibrary(
                    ENV=env,
                    NAME='freetypecpp',
                    SRC=Glob('src/*.cpp'),
                    SUPPORTED_PLATFORMS=['native','beagle'],
                    DEPS=['freetype2']
                    )

INSTALL_PREFIX = os.path.join( env['installPath'], 'usr/local' )
env.Alias( 'install', env.Install( os.path.join( INSTALL_PREFIX, 'lib' ), builtLibrary ) )

