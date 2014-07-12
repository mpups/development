from SCons.Errors import UserError
import importlib

class Dep:
    def __init__(self,name):
        self.name = name
        self.incpath={}
        self.libpath={}
        self.libs={}

def _Add( env, name ):
    platform = env['platform']

    try:
        module = importlib.import_module('deps.' + name)
    except ImportError, e:
        raise UserError("%s (Each external dependency needs a module in the deps folder.)"%(e))

    try:
        env.Append(CPPPATH=module.incpath[platform])
        env.Append(LIBPATH=module.libpath[platform])
        env.Append(LIBS=module.libs[platform])
        env.Append(RPATH=module.rpath[platform])
    except KeyError, e:
        raise UserError("Dependency '%s' does not support platform %s"%(name,e))

def List(env, names):
    for name in names:
        _Add(env,name)

