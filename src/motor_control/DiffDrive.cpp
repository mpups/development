#include "DiffDrive.h"

DiffDrive::DiffDrive( MotionMind& motors )
:
m_motors    ( motors )
{
    m_wheelbaseMetres = 0.275;
    m_countsPerMetre  = 6800;
    m_gearRatio       = 52;
    m_countsPerRevIn  = 12;
}

DiffDrive::~DiffDrive()
{

}

/**
    Compute the number of encoder counts per wheel revolution from the
    gear ratio and encoder counts per rev on the input shaft.
**/
float DiffDrive::ComputeCountsPerRev() const
{
    return m_countsPerRevIn * m_gearRatio * 4;
}

/**
    Compute the number of revolutions per metre for the wheels
    from the counts per metre.
**/
float DiffDrive::ComputeRevsPerMetre() const
{
    return m_countsPerMetre / ComputeCountsPerRev();
}

/**
    Set individual wheel speeds in counts per second.
**/
DiffDrive::MotorData DiffDrive::SetSpeeds( float left_counts_per_sec, float right_counts_per_sec )
{
}

/**
    Command the wheels to move a set number of encoder counts.
**/
DiffDrive::MotorData DiffDrive::SetMoves( float left_counts, float right_counts )
{
}

/**
    Set individual wheel speeds in metres per second.
**/
DiffDrive::MotorData DiffDrive::SetSpeeds_mps( float left_metres_per_sec, float right_metres_per_sec )
{
}

/**
    Set the overall motion with a forward velocity and an angular velocity.
**/
DiffDrive::MotorData DiffDrive::SetMotion( float velocity_metres_per_sec, float angular_deg_per_sec )
{
}

/**
    Return the current reading from the left motor.
**/
float DiffDrive::GetLeftAmps()
{
    return 0.f;
}

/**
    Return the current reading from the right motor.
**/
float DiffDrive::GetRightAmps()
{
    return 0.f;
}

/**
    Return the PWM ratio from the left motor.
**/
float GetLeftPwm()
{
    return 0.f;
}

/**
    Return the PWM ratio from the right motor.
**/
float GetRightPwm()
{
    return 0.f;
}

