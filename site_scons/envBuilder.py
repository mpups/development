# This script supports creating the build environment
from SCons.Script import Environment
import compilers

def makeEnvForCompiler( compiler ):
    # Create a build environment which uses the compiler:
    env = Environment(
        ENV = {'PATH' : compiler.path},
        CXX = compiler.cmd,
        CXXFLAGS = compiler.flags,
        LIBPATH = compiler.libpath,
        SYSROOT = compiler.sysroot,
        CPPDEFINES = compiler.defines,
        CPPPATH = compiler.includes,
        LIBS = compiler.libs
    )

    if ( compiler.debug == False ):
        env.Append( CPPDEFINES=['NDEBUG'] )

    return env
