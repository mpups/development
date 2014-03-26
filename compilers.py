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
    return c

def makeCompilerFor( target, buildType ):
    if ( target == "native" ):
        compiler = makeNative()
    elif ( target == "beagle" ):
        compiler = makeBeagle()
    else:
        print "Error: unknown build target: " + target
        sys.exit()

    if ( buildType == 'debug' ):
        compiler.AppendFlags( "-g -O0" )
        compiler.debug = True
    elif ( buildType == 'release' ):
        compiler.AppendFlags( "-O3" )

    return compiler


