#include "TeleJoystick.h"

#include <arpa/inet.h>

#include <VideoLib.h>

TeleJoystick::TeleJoystick( Socket& socket )
:
    m_thread    ( *this ),
    m_socket    ( socket ),
    m_drive     ( 0 ),
    m_terminate (false)
{
}

TeleJoystick::TeleJoystick( Socket& socket, DiffDrive* drive )
:
    m_thread    ( *this ),
    m_socket    ( socket ),
    m_drive     ( drive ),
    m_terminate (false)
{
}

TeleJoystick::~TeleJoystick()
{
    m_terminate = true;
    m_thread.Join();
}

void TeleJoystick::Run()
{
    GLK::Timer timer;
    float timeSinceLastCommand_secs = 0.f;
    const float motionTimeout_secs  = 1.f; // Motion will be stopped for safety if no command in this time.
    const float taskTimeout_secs    = 5.f; // Task will terminate if no command in this time.

    int data[3] = { 0, 0, 1 };

    while ( !m_terminate && timeSinceLastCommand_secs < taskTimeout_secs )
    {
        int timeoutCount = 0;
        int r = 0;
        int n = 3*sizeof(int32_t);
        int tmpData[3];
        char* pData = reinterpret_cast<char*>( tmpData );
        while ( n != 0 && r >= 0 && timeoutCount < 200 )
        {
            GLK::Thread::Sleep( 2 );
            r = m_socket.Read( pData, n );
            if ( r > 0 )
            {
                n -= r;
                pData += r;
            }
            else if ( r == 0 )
            {
                timeoutCount += 1;
            }
        }

        timeSinceLastCommand_secs = timer.GetSeconds();
        if ( n == 0 )
        {
            // Only update data if all necessary bytes were read:
            timer.Reset();
            
            data[0] = ntohl( tmpData[0] );
            data[1] = ntohl( tmpData[1] );
            data[2] = ntohl( tmpData[2] );
        }
        else if ( timeSinceLastCommand_secs > motionTimeout_secs )
        {
            // Set speeds to zero since we have lost comms (maybe temporarily)
            fprintf( stderr, "Warning: Joystick comms timedout - stopping motion until new comms received.\n" );
            data[0] = 0;
            data[1] = 0;
        }

        if ( m_drive )
        {
            DiffDrive::MotorData odometry = m_drive->JoyControl( data[0], data[1], data[2] );
        }
        else
        {
            fprintf( stderr, "Drive control (interval %f secs) := %d,%d (%d)\n", timeSinceLastCommand_secs, data[0], data[1], data[2] );    
        }
    }

    if ( m_drive )
    {
        m_drive->SetMotion( 0.f, 0.f );
    }

    fprintf( stderr, "Telejoy: thread finished\n" );
}

