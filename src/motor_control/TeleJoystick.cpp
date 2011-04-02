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
            GLK::Thread::Sleep( 2 );
            int r = m_socket.Read( pData, n );
            n -= r;
            pData += r;
        }
        
        m_drive.JoyControl( data[0], data[1], data[2] );
        
        /*
        float lA = 0.f;
        lA = m_drive.GetLeftAmps();
        float rA = 0.f;
        rA = m_drive.GetRightAmps();
                
        uint32_t ms = time.GetMilliSeconds();
        time.Reset();
        fprintf( stderr, "Loop time: %dms (control = %d,%d) (amps: l=%f. r=%f)\n", ms, data[0], data[1], lA, rA );
        //GLK::Thread::Sleep( 10 );*/
    }
    
    m_drive.SetMotion( 0.f, 0.f );
}

