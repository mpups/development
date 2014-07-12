import compilers

cbeagle = compilers.makeBeagle()

incpath = {
    'native' : ['/usr/include','/usr/include/freetype2'],
    'beagle' : [cbeagle.sysroot+'/include', cbeagle.sysroot+'/include/freetype2']
}

libpath = {
    'native' : ['/usr/lib/x86_64-linux-gnu'],
    'beagle' : [cbeagle.sysroot + '/lib']
}

libs = {
    'native' : ['freetype'],
    'beagle' : ['freetype']
}

rpath = {
    'native' : [], # Shouldn't normally need an rpath for native build as freetype is in standard location
    'beagle' : ['/lib']
}

