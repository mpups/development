import sys
import os

class Compiler:
    cmd   = ""
    path  = "/bin:/usr/bin"
    flags = "-std=c++14"
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
    return c

def makeBeagle():
    c = Compiler()
    c.cmd     = "arm-none-linux-gnueabi-g++"
    c.path   += ":/home/mark/software/x-tools/arm-none-linux-gnueabi/bin"
    c.defines = ['ARM_BUILD']
    c.sysroot = "/home/mark/beagleboardxm/deploy_final"
    c.libpath = c.sysroot + '/lib'
    c.AppendFlags( "-mtune=cortex-a8 -mfpu=neon" )
    return c

SDK_ROOT = '/home/markp/Android/Sdk/'
PLATFORM_VERSION = '29'
NDK_VERSION = '21.1.6352462'
PLATFORM = 'android-' + PLATFORM_VERSION
NDK_ROOT = os.path.join(SDK_ROOT, 'ndk', NDK_VERSION)
PLATFORM_ROOT = os.path.join(NDK_ROOT, 'platforms', PLATFORM)

# TODO: How to do NDK x86 support? Need to be able to build all there variants:
LLVM_PREFIX = 'i686' # 'armv7a' 'i686' # 'x86_64'
LIB_ARCH = 'arch-x86' # 'arch-arm' 'arch-arm64'  'arch-x86' 'arch-x86_64'
ABI_NAME = '-linux-android' # '-linux-androideabi' '-linux-android'

# For Android we have to set a lot of things,
# paths to binaries especially must be correct:
def makeAndroid():
    c = Compiler()
    c.cmd = LLVM_PREFIX + ABI_NAME + PLATFORM_VERSION + '-clang++'
    c.path += ":" + MakeAndroidPath(SDK_ROOT, NDK_ROOT)
    c.defines = ['ANDROID', 'ARM_BUILD']
    c.includes = MakeAndroidIncludes()
    c.libpath = MakeAndroidLibPath()
    c.libs = []
    c.sysroot = os.path.join(NDK_ROOT, 'toolchains/llvm/prebuilt/linux-x86_64/sysroot')
    c.AppendFlags( '-mfloat-abi=softfp' )
    return c

def MakeAndroidPath(sdkRoot, ndkRoot):
    sdkSubdirs = []#['tools/bin', 'tools', 'platform-tools']
    ndkSubdirs = [
        #'toolchains/llvm/prebuilt/linux-x86_64/arm-linux-androideabi/bin',
        #'toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin',
        'toolchains/llvm/prebuilt/linux-x86_64/bin']

    path = ':'.join([os.path.join(sdkRoot, d) for d in sdkSubdirs] + [os.path.join(ndkRoot, d) for d in ndkSubdirs])
    return path

def MakeAndroidIncludes():
    ANDROID_STL_ROOT = os.path.join(NDK_ROOT, 'sources/cxx-stl')
    ANDROID_STL_INC = os.path.join(ANDROID_STL_ROOT, 'llvm-libc++/include')
    inc = [] # Latest clang knows where everything is.
    return inc

def MakeAndroidLibPath():
    ANDROID_LIB = os.path.join(PLATFORM_ROOT, LIB_ARCH, 'usr/lib')
    ANDROID_STL = os.path.join(NDK_ROOT, 'sources/cxx-stl/llvm-libc++/libs/armeabi-v7a')
    libpath = [ANDROID_LIB, ANDROID_STL]
    return libpath

def makeCompilerFor(target, buildType):
    if target == "native":
        compiler = makeNative()
    elif target == "beagle":
        compiler = makeBeagle()
    elif target == "android":
        compiler = makeAndroid()
    else:
        raise RuntimeError("Error: unknown build target: " + target)

    if ( buildType == 'debug' ):
        compiler.AppendFlags( "-g -O0" )
        compiler.debug = True
    elif ( buildType == 'release' ):
        compiler.AppendFlags( "-O3" )

    return compiler


