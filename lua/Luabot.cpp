#include "Luabot.h"

MotionMind*     Luabot::ms_motion   = 0;
Joystick*       Luabot::ms_joystick = 0;
GLK::StdOut*    Luabot::ms_output   = 0;
GLK::Timer      Luabot::ms_time;

/**
    Construct a Luabot providing the Lua state and motion controller to use.

    This constructo exports the robot control functions to the specified Lua state.
    The client can then control the robot from a Lua console or from script (using the specified Lua state).

    @param lua The Lua state that the client will control the robot through.
    @param motionControl A motion control class for sending commands to a diff-drive robot.
**/
Luabot::Luabot( Lua::LuaState& lua, MotionMind& motion, Joystick& joystick, GLK::StdOut& output )
:
    m_lua       ( lua )
{
    ms_motion = &motion;
    assert( ms_motion->Available() );
    
    ms_joystick = &joystick;
    ms_output = &output;
    
    // Create the Lua API by registering funcs:
    LUA_REGISTER_FUNCTION( &m_lua, setSpeed );
    LUA_REGISTER_FUNCTION( &m_lua, setMove );
    LUA_REGISTER_FUNCTION( &m_lua, setupMotor );
    LUA_REGISTER_FUNCTION( &m_lua, readReg );
    LUA_REGISTER_FUNCTION( &m_lua, getJoyAxes );
    LUA_REGISTER_FUNCTION( &m_lua, getJoyButtonEvent );
}

/**
    Removes the API functions from the lua state.
**/
Luabot::~Luabot( )
{
    // unregister the functions?
}


/**
    Function to send the motion-mind a speed command from Lua.

    Expects 2 arguments:
    1:  motor address               (INTEGER)
    2:  speed in counts per second  (INTEGER)

    Returns 0 or 2 results.
    1:  timestamp in millisecs (INTEGER)
    2:  encoder position       (INTEGER)
**/
int Luabot::setSpeed( lua_State* L )
{
    GLK::Timer t;

    Lua::LuaState lua( L );

    if ( ms_motion->Available() )
    {
        if ( lua.IsNumber(1) && lua.IsNumber(2) )
        {
            int32_t addr  = lua.ToInteger(1);
            int32_t speed = lua.ToInteger(2);
            int32_t pos;
            uint32_t time = ms_time.GetMilliSeconds();
            bool ok = ms_motion->SetSpeed( addr, speed, pos );
            if ( ok )
            {
                lua.PushNumber( time );
                lua.PushNumber( pos );
                return 2;            
            }
        }
        else
        {
            lua.GetGlobal( "print" );
            lua.PushString( "usage: time, position = setSpeed( motor-address, counts-per-second )\n" );
            lua.Call( 1 );
        }
    }
    else
    {
        lua.GetGlobal( "print" );
        lua.PushString( "ERROR: communication link to motors is unavailable!\n" );
        lua.Call( 1 );
    }

    return 0;
}

/**
    Function to send the motion-mind a relative-move command from Lua.

    Expects 2 arguments:
    1:  Motor id
    2:  Required position-change (INTEGER)

    Returns 0 or 2 results.
    1:  timestamp in millisecs (INTEGER)
    2:  encoder position       (INTEGER)
**/
int Luabot::setMove( lua_State* L )
{
    Lua::LuaState lua( L );

    if ( ms_motion->Available() )
    {
        if ( lua.IsNumber(1) && lua.IsNumber(2) )
        {
            int32_t addr   = lua.ToInteger(1);
            int32_t counts = lua.ToInteger(2);
            int32_t position;
            uint32_t time = ms_time.GetMilliSeconds();
            bool ok = ms_motion->Move( addr, counts, position );
            if ( ok )
            {
                lua.PushNumber( time );
                lua.PushNumber( position );
                return 2;
            }
        }
    }
    else
    {
        lua.GetGlobal( "print" );
        lua.PushString( "ERROR: communication link to motors is unavailable!\n" );
        lua.Call( 1 );
    }

    return 0;
}

/**
    Function to set the motion-mind PID values from Lua.

    Expects 2 args:
    1: Motor address. (INTEGER)
    2: Set of register/value pairs. (TABLE)

    Returns 1 result:
    1: True or False for success or failure (BOOLEAN)
**/
int Luabot::setupMotor( lua_State* L )
{
    bool ok = false;
    Lua::LuaState lua( L );
    
    if ( ms_motion->Available() )
    {
        int nargs = lua.GetStackSize();
        if ( nargs == 2 && lua.IsNumber(1) && lua.IsTable(2) )
        {
            ok = true;
            int32_t addr = lua.ToInteger(1);
            // Iterate over {key,value} pairs in table
            lua.PushNil(); // To start iteration set key to nil
            while ( lua.Next(2) )
            {                
                if ( lua.IsString(-2) && lua.IsNumber(-1) )
                {
                    // Write register value to motion-mind controller:
                    MotionMind::Register reg = ms_motion->StringToRegister( lua.ToString(-2) );
                    int32_t value = lua.ToInteger( -1 );

                    ms_output->Printf( "Setting motor[%d] %s(%d) = %d", addr, lua.ToString(-2), reg, lua.ToInteger(-1) );
                    if ( ms_motion->WriteRegister( addr, reg, value ) )
                    {
                        ok &= true;
                        ms_output->Printf( " - ok!\n" );
                    }
                    else
                    {
                        ok = false;
                        ms_output->Printf( " - failed!\n" );                    
                    }
                }
                lua.Pop( 1 );
            }
            lua.Pop( 1 );
        }
        else
        {
            lua.GetGlobal( "print" );
            lua.PushString( "usage: setupMotor( table ) \n" );
            lua.Call( 1 );            
        }
    }
    else
    {
        lua.GetGlobal( "print" );
        lua.PushString( "ERROR: communication link to motors is unavailable!\n" );
        lua.Call( 1 );
    }

    lua.PushBoolean( ok );
    return 1;
}

/**
    Expects 1 arg:
    1: Reg name (STRING)

    returns 0 or 2 results:
    1: Timestamp in millisecs (INTEGER)    
    2: Reg-value (INTEGER)
**/
int Luabot::readReg( lua_State* L )
{   
    if ( ms_motion->Available() )
    {
        Lua::LuaState lua( L );
        int nargs = lua.GetStackSize();
        if ( nargs == 2 && lua.IsNumber(1) && lua.IsString(2) )
        {
            int32_t value;
            uint32_t time = ms_time.GetMilliSeconds();
            if ( ms_motion->ReadRegister( lua.ToInteger(1) , ms_motion->StringToRegister( lua.ToString(2) ), value ) )
            {
                lua.PushNumber( time );
                lua.PushNumber( value );
                return 2;
            }    
        }
    }

    return 0;
}

/**
    Expects 0 args:

    Returns joystick x and y values: - if no joystick available then it returns no results.

    returns 0 or 2 results:
    1: X-axis value
    2: Y-axis value 
**/
int Luabot::getJoyAxes( lua_State* L )
{
    if ( ms_joystick->IsAvailable() )
    {
        Lua::LuaState lua( L );
        lua.PushNumber( ms_joystick->GetAxis(2) );
        lua.PushNumber( ms_joystick->GetAxis(1) );
        return 2;
    }

    return 0;
}

/**
    Expects 0 args:

    Returns joystick button events: - if there are no events in the queue then it returns no results.

    returns 0 or 2 results:
    1: button id
    2: button state 
**/
int Luabot::getJoyButtonEvent( lua_State* L )
{
    if ( ms_joystick->IsAvailable() )
    {
        Lua::LuaState lua( L );
        
        Joystick::ButtonEvent b;
        if ( ms_joystick->GetButtonEvent( b ) )
        {
            lua.PushNumber( b.id );
            lua.PushNumber( b.pressed );
            return 2;
        }
    }

    return 0;
}

