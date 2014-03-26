import os

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

def findCppFiles( topDir ):
    cppFiles = []

    # traverse root directory, and list directories as dirs and files as files
    for path, dirs, files in os.walk(topDir):
        split = path.split('/')
        #print (len(split) - 1) *'--' , os.path.basename(path)
        for file in files:
            if fnmatch.fnmatch(file,'*.cpp'):
                #print len(split)*'--', file
                cppFiles.append( os.path.join( path, file ) )

    return cppFiles

# Find all cpps in the specified dirs.
# They are taken as relative to the SConscript dir
# in which you call the funciton:
def findCppInLocalDirs( src_dirs ):
    src = []
    for dir in src_dirs:
        src += ( findCppFiles( dir ) )
    return src

#src_dirs = [
#    'src/graphics',
#    'src/legacy'
#]

#src = findCppInLocalDirs( src_dirs )

#src2 = []
#for file in src:
#    src2.append( os.path.join('./build/native/release',file) )

#print src2
