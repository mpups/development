import compilers

cbeagle = compilers.makeBeagle()

incpath = {
    'native' : ['/usr/include','/usr/include/unicap'],
    'beagle' : [cbeagle.sysroot + '/include', cbeagle.sysroot + '/include/unicap']
}

libpath = {
    'native' : ['/usr/lib'],
    'beagle' : [cbeagle.sysroot + '/lib']
}

libs = {
    'native' : ['unicap'],
    'beagle' : ['unicap']
}

rpath = {
    'native' : [],
    'beagle' : ['/lib']
}
