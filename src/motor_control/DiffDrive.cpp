#include "DiffDrive.h"

#include <math.h>

DiffDrive::DiffDrive( MotionMind& motors )
:
    m_motors    ( motors ),
    m_config    ( 0.275f, 6800.f, 52.f, 12.f )
{
    m_motors.WriteRegister( LEFT_WHEEL, MotionMind::POSITION, 0 );
    m_motors.WriteRegister( RIGHT_WHEEL, MotionMind::POSITION, 0 );
}

/**
    Stop the motors and then set move to zero.
    (On Motion-Minds this ensures minimum current draw.)
**/
DiffDrive::~DiffDrive()
{
    int32_t pos;
    m_motors.SetSpeed( LEFT_WHEEL,  0, pos );
    m_motors.SetSpeed( RIGHT_WHEEL, 0, pos );
    m_motors.Move( LEFT_WHEEL,  0, pos );
    m_motors.Move( RIGHT_WHEEL, 0, pos );
}

/**
    Compute the number of encoder counts per wheel (output shaft) assuming quadrature encoding.
**/
float DiffDrive::ComputeCountsPerRev() const
{
    return m_config.CountsPerRevOnOutputShaft() * 4;
}

/**
    Compute the number of revolutions per metre for the wheels
    from the counts per metre.
**/
float DiffDrive::ComputeRevsPerMetre() const
{
    return m_config.CountsPerMetre() / ComputeCountsPerRev();
}

/**
    Set individual wheel speeds in counts per second.
**/
DiffDrive::MotorData DiffDrive::SetSpeeds( float left_counts_per_sec, float right_counts_per_sec )
{
    MotorData data;
    data.leftTime = data.rightTime = 0;
    
    int32_t left  = left_counts_per_sec;
    int32_t right = right_counts_per_sec;
    
    //fprintf( stderr, "\n\t counts per sec: %d %d\n", left, right );
    m_motors.SetSpeed( LEFT_WHEEL,  left,  data.leftPos );
    m_motors.SetSpeed( RIGHT_WHEEL, right, data.rightPos );
    
    data.leftPos = -data.leftPos;
    return data;
}

/**
    Command the wheels to move a set number of encoder counts.
**/
DiffDrive::MotorData DiffDrive::SetMoves( float left_counts, float right_counts )
{
    MotorData data;
    data.leftTime = data.rightTime = 0;

    int32_t left  = left_counts;
    int32_t right = right_counts;
    m_motors.Move( LEFT_WHEEL,  left,  data.leftPos );
    m_motors.Move( RIGHT_WHEEL, right, data.rightPos );
    
    data.leftPos = -data.leftPos;
    return data;
}

/**
    Set individual wheel speeds in metres per second.
    
    Converts to counts per sec and calls DiffDrive::SetSpeeds.
**/
DiffDrive::MotorData DiffDrive::SetSpeeds_mps( float left_metres_per_sec, float right_metres_per_sec )
{
    float cpm = m_config.CountsPerMetre();
    return SetSpeeds( left_metres_per_sec * cpm, right_metres_per_sec * cpm );
}

/**
    Set the overall motion with a forward velocity and an angular velocity.
    
    Converts to left and right speeds in m/s and passes to SetSpeeds_mps;
**/
DiffDrive::MotorData DiffDrive::SetMotion( float velocity_metres_per_sec, float angular_deg_per_sec )
{
    float b = m_config.WheelBaseInMetres();
    float w = angular_deg_per_sec * b * (3.141592653/180.0);
    float vl = velocity_metres_per_sec - w;
    float vr = velocity_metres_per_sec + w;
    return SetSpeeds_mps( vl, vr );
}

/**
    Utility to compute sensible speeds based on joystick input.
**/
DiffDrive::MotorData DiffDrive::JoyControl( int32_t jx, int32_t jy, int32_t maxVal )
{
    const float m_maxSpeed_mps = .5f;
    float max = maxVal;

    float v = ( -jy / max ) * m_maxSpeed_mps; // forward speed in range (-max_speed,max_speed) m/sec
    float tr = (.5f * jx) / max; // turn rate
    float vl = v + tr;
    float vr = v - tr;

    // apply min speed limit (otherwise motors will judder)
    if ( fabsf(vl) < 0.075f ) { vl = 0.f; }
    if ( fabsf(vr) < 0.075f ) { vr = 0.f; }

    //fprintf( stderr, "Setting speeds: %f %f\n", vl, vr );
    return SetSpeeds_mps( vl, vr ); // sets left and right speeds in metres per second
}

/**
    Return the current reading from the left motor.
**/
float DiffDrive::GetLeftAmps()
{
    int32_t val;
    m_motors.ReadRegister( LEFT_WHEEL, MotionMind::AMPS, val );
    return val * 0.02f;
}

/**
    Return the current reading from the right motor.
**/
float DiffDrive::GetRightAmps()
{
    int32_t val;
    m_motors.ReadRegister( RIGHT_WHEEL, MotionMind::AMPS, val );
    return val * 0.02f;
}

/**
    Return the PWM percentage from the left motor.
**/
float DiffDrive::GetLeftPwm()
{
    int32_t val;
    m_motors.ReadRegister( LEFT_WHEEL, MotionMind::PWMLIMIT, val );
    return val * (100.f/1024.f);
}

/**
    Return the PWM percentage from the right motor.
**/
float DiffDrive::GetRightPwm()
{
    int32_t val;
    m_motors.ReadRegister( RIGHT_WHEEL, MotionMind::PWMLIMIT, val );
    return val * (100.f/1024.f);
}

