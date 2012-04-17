#include <RoboLib.h>

#include <glkcore.h>
#include <glk.h>

/**
    Program for controlling robot-motors using lua script console.
**/
int main( int argc, char** argv )
{
    // start console
    GLK::ConsoleWindow* console = new GLK::ConsoleWindow( "Motor Control Console", 600, 480, GLK::String(">> ") );
    
    // setup Lua
    Lua::LuaState lua;
    lua.OpenLibraries();
    
    LUA_REGISTER_SLEEP( &lua );
    console->SetupLua( lua ); // Sets up console to be Lua friendly.

    // Setup Lua based robot control:
    MotionMind motors( DEFAULT_SERIAL_PORT );
    Joystick joystick( "/dev/input/js0" );
    Luabot robot( lua, motors, joystick, *console );
   
    if ( joystick.IsAvailable() )
    {
        joystick.Start();
    }

    if ( argc == 2 )
    {
        lua.DoFile( argv[1] );
    }

    // Start loop which processes command-messages from console.
    GLK::String cmd;
    while ( console->IsRunning() )
    {
        // First burn through all commands in buffer as fast as possible:
        while ( console->TryCommand( cmd ) )
        {
            int err = lua.DoString( cmd.cStr() );
            lua.PrintError( err, *console );
        }
        
        // Then sleep while buffer is empty so we don't waste cpu time.
        console->WaitCommand();
    }

    int exitCode = console->WaitForClose();

    delete console;

    // ensure robot is stopped on exit
    int32_t pos;
    motors.SetSpeed( 1, 0, pos );
    motors.SetSpeed( 2, 0, pos );

    // setting relative moves of 0 ensures minimum current draw:
    motors.Move( 1, 0, pos );
    motors.Move( 2, 0, pos );

    return exitCode;
}
