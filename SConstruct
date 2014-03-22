import os
import compilers
import envBuilder

AddOption( '--build-for',
           dest='build-for',
           type='string',
           nargs=1,
           action='store',
           metavar='ARCHITECTURE',
           default='native',
           help='Specify architecture to build for: native,beagle,android')

# Get target from cmd line:
target = GetOption('build-for')
buildDir = target + '_build'
print "Building for target '" + target + "':"

# Setup the compiler:
compiler = compilers.makeCompilerFor(target)
compiler.AppendFlags( "-O3 -Wall -pedantic -Werror" ) # TODO choose flags based on build type (e.g. debug/release)

# Initialise the environment to use this compiler:
env = envBuilder.makeEnvForCompiler(compiler)

Export('env','target','compiler')
SConscript( 'SConscript', variant_dir=buildDir, duplicate=0 )

