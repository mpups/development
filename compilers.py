import sys

class Compiler:
    cmd   = ""
    path  = "/bin"
    flags = "-std=c++11"
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
    c.sysroot = "/home/mark/beagleboardxm/deploy_final"
    c.AppendFlags( "-mtune=cortex-a8 -mfpu=neon" )
    c.AppendFlags( c.sysroot+"/lib" )
    return c

def makeAndroid():
    c = Compiler()
    c.cmd     = "arm-linux-androideabi-g++"
    c.path   += MakeAndroidPath( '/home/mark/code/android-sdk-linux_x86', '/home/mark/code/android-ndk-r9b' )
    c.sysroot = "/home/mark/code/android-ndk-r9b/platforms/android-3/arch-arm"
    c.AppendFlags( "-mfloat-abi=softfp" )
    c.AppendFlags( c.sysroot+"/usr/lib" )
    return c

def MakeAndroidPath( sdkRoot, ndkRoot ):
    path = ":" + sdkRoot + "/tools:" + sdkRoot + "/platform-tools:" + ndkRoot + ":" + ndkRoot + "/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin"
    return path

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


