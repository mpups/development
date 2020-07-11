import os
import fnmatch
from SCons.Script import Dir, Return, Glob

def TargetIsSupported(target, supported):
    return target in supported

def EndScriptIfTargetNotSupported(target, supported):
    if not TargetIsSupported(target,supported):
        print 'INFO: Ignoring Project ' + Dir('.').path + ' (does not support platform: ' + target + ')'
        Return()

def TargetIsValid(target):
    validTargets = [ 'native','beagle','android' ]
    return target in validTargets;

def BuildTypeIsValid(type):
    validTypes = [ 'debug', 'release' ]
    return type in validTypes;

def FindSconsDirs(root):
    sconsDirs = []
    for file in os.listdir( root ):
        fullpath = os.path.join(root,file)
        if os.path.isdir(fullpath):
            if 'SConscript' in os.listdir(fullpath):
                sconsDirs.append(fullpath)

    return sconsDirs

def RecursivelyGlobSourceInPaths(extension, dirList):
    # For each entry in list we glob all the source
    # files in all subdirs
    src = []

    basePath = Dir('.').srcnode().abspath
    dirs = [os.path.join(basePath, d) for d in dirList]
    for d in dirs:
        # traverse root directory, and list directories as dirs and files as files
        for path, dirs, files in os.walk(d):
            glob_pattern = os.path.join(path, '*.' + extension)
            glob = Glob(glob_pattern)
            src += glob

    return src

def RemoveFiles(glob, filterList):
    if type(filterList) is str:
        raise Exception("RemoveFiles requires filterList to be a list of strings.")
    for m in filterList:
        for f in glob:
            if ( m in f.abspath ):
                glob.remove(f)

# This is not to be used to pass source files to SCons (it will not work).
# Use RecursivelyGlobSourceInPaths instead.
def RecursivelyFindFiles(extension, topDir):
    src = []
    pattern = '*.' + extension
    # traverse root directory, and list directories as dirs and files as files
    for path, dirs, files in os.walk(topDir):
        for file in files:
            if fnmatch.fnmatch(file, pattern):
                src.append(os.path.join(path, file))
    return src

def GenerateHeaderInstallActions(env, installPath, searchDir, projectDir):
    realDir = Dir('.').srcnode().abspath
    searchPath = os.path.join(realDir, searchDir)
    sources = RecursivelyFindFiles('h', searchPath)
    return [env.Install(os.path.join(installPath, os.path.dirname(h.split(projectDir+'/')[1])), h) for h in sources]

