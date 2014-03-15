import os

num_cpu = int(os.environ.get('NUM_CPU', 2))
SetOption('num_jobs', num_cpu)

AddOption('--build-for',
           dest='build-for',
           type='string',
           nargs=1,
           action='store',
           metavar='DIR',
           help='Specify architecture to build for: native,beagleboard,android')

buildDir=GetOption('build-for') + '_build'
SConscript( 'SConscript', variant_dir=buildDir, duplicate=0 )

