#include "TeleJoystick.h"

TeleJoystick::TeleJoystick( Socket& socket, DiffDrive& drive )
:
    m_socket    ( socket ),
    m_drive     ( drive )
{
    Start();
}

TeleJoystick::~TeleJoystick()
{
    Stop();
}

void TeleJoystick::Run()
{
    GLK::Timer time;
    
    while ( true )
    {
        int n = 3*sizeof(int);
        int data[3];
        char* pData = reinterpret_cast<char*>( data );
        while ( n != 0 )
        {
            int r = m_socket.Read( pData, n );
            n -= r;
            pData += r;
        }
        
        m_drive.JoyControl( data[0], data[1], data[2] );
        fprintf(stderr, "%d %d %d\n", data[0], data[1], data[2] );
        
        uint32_t ms = time.GetMilliSeconds();
        time.Reset();
        fprintf( stderr, "Loop time: %dms\n", ms );
        GLK::Thread::Sleep( 10 );
    }
    
    m_drive.SetMotion( 0.f, 0.f );
}
