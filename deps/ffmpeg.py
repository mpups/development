import compilers

bc = compilers.makeBeagle()
ac = compilers.makeAndroid()

incpath = {
    'native'  : ['/home/mark/tmp_installs/include'],
    'beagle'  : [bc.sysroot+'/include'],
    'android' : ['/home/mark/code/android-ffmpeg-build/armeabi/include']
}

libpath = {
    'native'  : ['/home/mark/tmp_installs/lib'],
    'beagle'  : [bc.sysroot+'/lib'],
    'android' : ['/home/mark/code/android-ffmpeg-build/armeabi/lib']
}

libs = {
    'native'  : ['avformat', 'avcodec', 'avutil', 'swscale'],
    'beagle'  : ['avformat', 'avcodec', 'avutil', 'swscale'],
    'android' : ['avformat', 'avcodec', 'avutil', 'swscale']
}

rpath = {
    'native'  : ['/home/mark/tmp_installs/lib'],
    'beagle'  : ['/lib'],
    'android' : []
}
