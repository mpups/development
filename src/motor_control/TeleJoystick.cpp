#include "TeleJoystick.h"

#include <arpa/inet.h>
#include <iostream>

#include <VideoLib.h>
#include "../puppybot/PacketDemuxer.h"

TeleJoystick::TeleJoystick( PacketDemuxer& muxer )
:
    m_thread    ( *this ),
    m_muxer     ( muxer ),
    m_drive     ( 0 ),
    m_terminate (false)
{
}

TeleJoystick::TeleJoystick( PacketDemuxer& muxer, DiffDrive* drive )
:
    m_thread    ( *this ),
    m_muxer     ( muxer ),
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

    int32_t data[3] = { 0, 0, 1 };

    // Setup subscription so joystick data is enqueued:
    GLK::Mutex queueLock;
    GLK::ConditionVariable queueNotEmpty;
    std::queue< ComPacket::ConstSharedPacket > joyPackets;
    PacketSubscription joystickSubscription = m_muxer.Subscribe( ComPacket::Type::Joystick, [&]( const ComPacket::ConstSharedPacket& packet )
    {
        assert( packet->GetType() == ComPacket::Type::Joystick );
        joyPackets.emplace( packet );
        queueNotEmpty.WakeOne();
    });

    while ( !m_terminate && m_muxer.Ok() && timeSinceLastCommand_secs < taskTimeout_secs )
    {
        timeSinceLastCommand_secs = timer.GetSeconds();
        {
            GLK::MutexLock lock( queueLock );
            constexpr int wait_ms = 200;
            queueNotEmpty.TimedWait( queueLock, wait_ms ); // sleep until a packet is received

            if ( joyPackets.empty() == false )
            {
                timer.Reset();

                // Only interested in latest packet if there are multiple (more control responsiveness):
                while ( joyPackets.size() > 1 )
                {
                    joyPackets.pop();
                }

                const ComPacket::ConstSharedPacket packet = joyPackets.front();
                const int32_t* networkIntData = reinterpret_cast<const int32_t*>(packet->GetDataPtr());
                data[0] = ntohl( networkIntData[0] );
                data[1] = ntohl( networkIntData[1] );
                data[2] = ntohl( networkIntData[2] );
                joyPackets.pop();
            }
            else if ( timeSinceLastCommand_secs > motionTimeout_secs )
            {
                // Set speeds to zero since we have lost comms (maybe temporarily)
                fprintf( stderr, "Warning: Joystick comms timedout - stopping motion until new comms received.\n" );
                data[0] = 0;
                data[1] = 0;
            }
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

