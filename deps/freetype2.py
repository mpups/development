import deps

module = deps.Make('freetype2')

module.incpath = {
    'native' : ['/usr/include','/usr/include/freetype2'],
    'beagle' : ['/home/mark/beagleboardxm/deploy_final/include/freetype2']
}

module.libpath = {
    'native' : ['/usr/include','/usr/include/freetype2'],
    'beagle' : ['/home/mark/beagleboardxm/deploy_final/lib']
}

module.libs = {
    'native' : ['freetype'],
    'beagle' : ['freetype']
}

