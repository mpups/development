import os

AddOption('--build-for',
           dest='build-for',
           type='string',
           nargs=1,
           action='store',
           metavar='ARCHITECTURE',
           default='native',
           help='Specify architecture to build for: native,beagle,android')

buildDir = GetOption('build-for') + '_build'
SConscript( 'SConscript', variant_dir=buildDir, duplicate=0 )

