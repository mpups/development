import os
from SCons.Script import Dir,Return,Glob

def TargetIsSupported( target, supported ):
    return target in supported

def EndScriptIfTargetNotSupported( target, supported ):
    if not TargetIsSupported(target,supported):
        print 'INFO: Ignoring Project ' + Dir('.').path + ' (does not support platform: ' + target + ')'
        Return()

def TargetIsValid( target ):
    validTargets = [ 'native','beagle','android' ]
    return target in validTargets;

def BuildTypeIsValid( type ):
    validTypes = [ 'debug', 'release' ]
    return type in validTypes;

def FindSconsDirs( root ):
    sconsDirs = []
    for file in os.listdir( root ):
        fullpath = os.path.join(root,file)
        if os.path.isdir(fullpath):
            if 'SConscript' in os.listdir(fullpath):
                sconsDirs.append(fullpath)

    return sconsDirs

def RecursivelyGlobSourceInPaths( extension, dirList ):
    # For each entry in list we glob all the source
    # files in all subdirs
    src = []

    for dir in dirList:
        # traverse root directory, and list directories as dirs and files as files
        for path, dirs, files in os.walk(dir):
            src += Glob( os.path.join(path,'*.'+extension) );

    return src

def RemoveFiles( glob, filterList ):
    for m in filterList:
        for f in glob:
            if ( m in f.abspath ):
                glob.remove(f)
