#include "TeleJoystick.h"

#include <arpa/inet.h>

TeleJoystick::TeleJoystick( Socket& socket )
:
    m_socket    ( socket ),
    m_drive     ( 0 )
{
    Start();
}

TeleJoystick::TeleJoystick( Socket& socket, DiffDrive* drive )
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
        
        data[0] = ntohl( data[0] );
        data[1] = ntohl( data[1] );
        data[2] = ntohl( data[2] );
        
        if ( m_drive )
        {
            m_drive->JoyControl( data[0], data[1], data[2] );
        }
        else
        {
            fprintf( stderr, "Drive control := %d,%d (%d)\n", data[0], data[1], data[2] );    
        }
    }
    
    if ( m_drive )
    {
        m_drive->SetMotion( 0.f, 0.f );
    }
}

