import compilers

bc = compilers.makeBeagle()

incpath = {
    'native'  : ['/usr/include/gtest'],
    'beagle'  : [bc.sysroot + '/include']
}

libpath = {
    'native' : ['/usr/lib'],
    'beagle' : [bc.sysroot + '/lib']
}

libs = {
    'native' : ['gtest_main','gtest'],
    'beagle' : ['gtest_main','gtest']
}

rpath = {
    'native' : [],
    'beagle' : []
}
