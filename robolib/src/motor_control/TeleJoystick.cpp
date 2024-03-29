#include "TeleJoystick.h"

#include <arpa/inet.h>
#include <iostream>

#include "../packetcomms/PacketMuxer.h"
#include "../packetcomms/PacketSerialisation.h"
#include "../packetcomms/PacketDemuxer.h"
#include "DiffDrive.h"
#include "../utility/Timer.h"

TeleJoystick::TeleJoystick( std::pair<PacketMuxer&,PacketDemuxer&> muxers )
:
    m_muxer     ( muxers.first ),
    m_demuxer   ( muxers.second ),
    m_drive     ( nullptr ),
    m_terminate ( false ),
    m_thread    ( std::bind( &TeleJoystick::Run, std::ref(*this) ) )
{
}

TeleJoystick::TeleJoystick( std::pair<PacketMuxer&,PacketDemuxer&> muxers, DiffDrive* drive )
:
    m_muxer     ( muxers.first ),
    m_demuxer   ( muxers.second ),
    m_drive     ( drive ),
    m_terminate ( false ),
    m_thread    ( std::bind( &TeleJoystick::Run, std::ref(*this) ) )
{
}

TeleJoystick::~TeleJoystick()
{
    m_terminate = true;
    m_thread.join();
}

void TeleJoystick::Run()
{
    Timer timer(CLOCK_MONOTONIC);
    float timeSinceLastCommand_secs = 0.f;
    const float motionTimeout_secs  = 1.f; // Motion will be stopped for safety if no command in this time.
    const float taskTimeout_secs    = 5.f; // Task will terminate if no command in this time.

    int16_t jx = 0;
    int16_t jy = 0;
    int16_t jmax = 1;

    // Setup subscription callback to enqueue joystick packets:
    SimpleQueue joyPackets;
    PacketSubscription joystickSubscription = m_demuxer.Subscribe( "Joystick", [&]( const ComPacket::ConstSharedPacket& packet )
    {
        assert( packet->GetType() == m_demuxer.GetIdManager().ToId("Joystick") );
        joyPackets.Emplace( packet );
    });

    constexpr std::chrono::milliseconds wait(200);
    while ( !m_terminate && m_demuxer.Ok() && timeSinceLastCommand_secs < taskTimeout_secs )
    {
        timeSinceLastCommand_secs = timer.GetSeconds();
        {
            // Sleep with timeout until a packet is received:
            joyPackets.Lock().WaitNotEmpty( wait );

            if ( joyPackets.Empty() == false )
            {
                timer.Reset();

                // Only interested in latest packet if there are multiple (more control responsiveness):
                while ( joyPackets.Size() > 1 )
                {
                    joyPackets.Pop();
                }

                ProcessPacket( joyPackets, jx, jy, jmax );
            }
            else if ( timeSinceLastCommand_secs > motionTimeout_secs )
            {
                // Set speeds to zero since we have lost comms (maybe temporarily)
                fprintf( stderr, "Warning: Joystick comms timedout - stopping motion until new comms received.\n" );
                jx = 0;
                jy = 0;
            }
        }

        if ( m_drive )
        {
            DiffDrive::MotorData odometry = m_drive->JoyControl( jx, jy, jmax );
            if ( odometry.valid == true )
            {
                Serialise( m_muxer, "Odometry", odometry );
            }
        }
        else
        {
            fprintf( stderr, "Drive control (interval %f secs) := %d,%d (%d)\n", timeSinceLastCommand_secs, jx, jy, jmax );

            // Send a faked (and invalid) odometry packet for testing/debugging:
            Serialise( m_muxer, "Odometry", DiffDrive::MotorData{0,0,0,0,false});
        }
    }

    if ( m_drive )
    {
        m_drive->SetMotion( 0.f, 0.f );
    }

    m_terminate = true; /// @note - need this to indicate thread has stopped running
    fprintf( stderr, "Telejoy: thread finished\n" );
}

/**
    Extract the joystick data from the front of joystick packet queue into separate variables for X and Y axis data and the max value for either axis reading.
    @param joyPackets Packet queu to read from - front item will be read an dpopped from the queue.
    @param jx x-axis reading from the packet
    @param jy y-axis reading from the packet
    @param jmax The max value an axis reading can take.
*/
void TeleJoystick::ProcessPacket(SimpleQueue& joyPackets, int16_t& jx, int16_t& jy, int16_t& jmax )
{
    const ComPacket::ConstSharedPacket packet = joyPackets.Front();
    Deserialise( packet, jx, jy, jmax );
    joyPackets.Pop();
}

bool TeleJoystick::IsRunning() const
{
    return (!m_terminate) && m_thread.joinable();
}
