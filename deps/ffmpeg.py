import os
import compilers

bc = compilers.makeBeagle()
# TODO: to support different arm archs need access to arch here - where is install root itself?:
android_install_root = '/home/markp/development/install/android/x86/ffmpeg'

incpath = {
    'native'  : ['/home/mark/local_installs/include'],
    'beagle'  : [os.path.join(bc.sysroot, '/include')],
    'android' : [os.path.join(android_install_root, 'include')]
}

libpath = {
    'native'  : ['/home/mark/local_installs/lib'],
    'beagle'  : [os.path.join(bc.sysroot, '/lib')],
    'android' : [os.path.join(android_install_root, 'lib')]
}

libs = {
    'native'  : ['avformat', 'avcodec', 'avutil', 'swscale'],
    'beagle'  : ['avformat', 'avcodec', 'avutil', 'swscale'],
    'android' : ['avformat', 'avcodec', 'avutil', 'swscale']
}

rpath = {
    'native'  : ['/home/mark/local_installs/lib'],
    'beagle'  : ['/lib'],
    'android' : []
}
