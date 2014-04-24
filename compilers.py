import sys

class Compiler:
    cmd   = ""
    path  = "/bin"
    flags = "-std=c++11"
    defines = []
    includes = []
    libpath = []
    libs = []
    sysroot = "/"
    debug = False
    def AppendFlags( self, newFlags ):
        self.flags += " " + newFlags

def makeNative():
    c = Compiler()
    c.cmd   = "/usr/bin/c++"
    c.path += ":/usr/bin"
    return c

def makeBeagle():
    c = Compiler()
    c.cmd     = "arm-none-linux-gnueabi-g++"
    c.path   += ":/home/mark/x-tools/arm-none-linux-gnueabi/bin"
    c.defines = ['ARM_BUILD']
    c.sysroot = "/home/mark/beagleboardxm/deploy_final"
    c.libpath = c.sysroot + '/lib'
    c.AppendFlags( "-mtune=cortex-a8 -mfpu=neon" )
    c.AppendFlags( c.sysroot+"/lib" )
    return c

# Android is a little bitch so we always
# set up the compiler with fixed version of sdk/ndk
# and always use gnu_stl to simplify things:
def makeAndroid():
    SDK_ROOT = '/home/mark/code/android-sdk-linux_x86'
    NDK_ROOT = '/home/mark/code/android-ndk-r9b'

    c = Compiler()
    c.cmd     = 'arm-linux-androideabi-g++'
    c.path   += MakeAndroidPath( SDK_ROOT, NDK_ROOT )
    c.defines = ['ANDROID', 'ARM_BUILD']
    c.includes = MakeAndroidIncludes( NDK_ROOT )
    c.libpath = MakeAndroidLibPath( NDK_ROOT )
    c.libs = ['gnustl_shared']
    c.sysroot = NDK_ROOT + '/platforms/android-8/arch-arm'
    c.AppendFlags( '-mfloat-abi=softfp' )
    c.AppendFlags( c.sysroot + '/usr/lib' )
    return c

def MakeAndroidPath( sdkRoot, ndkRoot ):
    path = ":" + sdkRoot + "/tools:" + sdkRoot + "/platform-tools:" + ndkRoot + ":" + ndkRoot + "/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin"
    return path

def MakeAndroidIncludes( NDK_ROOT ):
    ANDROID_INC = NDK_ROOT + '/platforms/android-8/arch-arm/usr/include'
    ANDROID_STL_PATH = NDK_ROOT + '/sources/cxx-stl/gnu-libstdc++/4.8'
    ANDROID_STL_INC = ANDROID_STL_PATH + '/include'
    ANDROID_STL_BITS_INC = ANDROID_STL_PATH + '/libs/armeabi/include'
    inc = [ ANDROID_INC, ANDROID_STL_INC, ANDROID_STL_BITS_INC ]
    return inc

def MakeAndroidLibPath(NDK_ROOT):
    ANDROID_LIB = NDK_ROOT + '/platforms/android-8/arch-arm/usr/lib'
    ANDROID_STL = NDK_ROOT + '/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi'
    libpath=[ ANDROID_LIB, ANDROID_STL ]
    return libpath

def makeCompilerFor( target, buildType ):
    if target == "native":
        compiler = makeNative()
    elif target == "beagle":
        compiler = makeBeagle()
    elif target == "android":
        compiler = makeAndroid()
    else:
        print "Error: unknown build target: " + target
        sys.exit()

    if ( buildType == 'debug' ):
        compiler.AppendFlags( "-g -O0" )
        compiler.debug = True
    elif ( buildType == 'release' ):
        compiler.AppendFlags( "-O3" )

    return compiler


