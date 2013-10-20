// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef LUA_BOT_CLASS_H
#define LUA_BOT_CLASS_H

#include <ostream>

#include "../../src/utility/Timer.h"
#include "../../include/RoboLibCore.h"

/**
    CLass which encapsulates lua control of a differential drive robot.

    The robot is currently controlled by a motion mind controller.

**/
class Luabot
{
public:
    Luabot( Lua::State& lua, MotionMind& motion, Joystick& joystick, std::ostream& output );
    ~Luabot();

protected:
    static int setSpeed( lua_State* L );
    static int setMove( lua_State* L );
    static int setupMotor( lua_State* L );
    static int readReg( lua_State* L );
    static int getJoyAxes( lua_State* L );
    static int getJoyButtonEvent( lua_State* L );
    
private:
    static MotionMind*      ms_motion;
    static Joystick*        ms_joystick;
    static std::ostream*    ms_output;
    static Timer            ms_time;
        
    Lua::State&  m_lua;
};

#endif // LUA_BOT_CLASS_H


