from SCons.Errors import UserError

class Dep:
    def __init__(self,name):
        self.name = name
        self.incpath={}
        self.libpath={}
        self.libs={}

_db = {}

def _Add( env, name ):
    dep = _db[name]
    platform = env['platform']
    try:
        env.Append(CPPPATH=dep.incpath[platform])
        env.Append(LIBPATH=dep.libpath[platform])
        env.Append(LIBS=dep.libs[platform])
    except KeyError, e:
        raise UserError("Dependency '%s' does not support platform %s"%(name,e))

    return

def List(env, names):
    for name in names:
        _Add(env,name)

def Make(name):
    _db[name] = Dep(name)
    return _db[name]

import freetype2

