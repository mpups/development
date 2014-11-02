import compilers

bc = compilers.makeBeagle()
ac = compilers.makeAndroid()

incpath = {
    'native'  : ['/home/mark/local_installs/include'],
    'beagle'  : [bc.sysroot+'/include'],
    'android' : ['/home/mark/software/android-ffmpeg-install/armeabi/include']
}

libpath = {
    'native'  : ['/home/mark/local_installs/lib'],
    'beagle'  : [bc.sysroot+'/lib'],
    'android' : ['/home/mark/software/android-ffmpeg-install/armeabi/lib']
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
