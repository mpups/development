#ifndef DIFF_DRIVE_H
#define DIFF_DRIVE_H

#include "MotionMind.h"
#include "DiffDriveConfig.h"

/**
    Class for controlling simple differential drive robots.
    
    Functionality includes configuration management and speed settings.
**/
class DiffDrive
{
public:

    struct MotorData
    {
        double  leftTime;
        double  rightTime;
        int32_t leftPos;
        int32_t rightPos;
        bool    valid;
    };

    DiffDrive( MotionMind& motors );
    virtual ~DiffDrive();
    
    MotorData SetSpeeds( float left_counts_per_sec, float right_counts_per_sec );
    MotorData SetMoves( float left_counts, float right_counts );
    MotorData SetSpeeds_mps( float left_metres_per_sec, float right_metres_per_sec );
    MotorData SetMotion( float velocity_metres_per_sec, float angular_deg_per_sec );
    MotorData JoyControl( int32_t leftVal, int32_t rightVal, int32_t maxVal );
    
    float GetLeftAmps();
    float GetRightAmps();
    float GetLeftPwm();
    float GetRightPwm();
    
protected:
    float ComputeCountsPerRev() const;
    float ComputeRevsPerMetre() const;
    
private:
    static const int LEFT_WHEEL  = 1;
    static const int RIGHT_WHEEL = 2;
    
    MotionMind& m_motors;
    const DiffDriveConfig m_config;
};

template<typename T>
void save(T& archive, const DiffDrive::MotorData& d)
{
    archive(d.leftPos,d.leftTime,d.rightPos,d.rightTime,d.valid);
}

template<typename T>
void load(T& archive, DiffDrive::MotorData& d)
{
    archive(d.leftPos,d.leftTime,d.rightPos,d.rightTime,d.valid);
}

#endif // DIFF_DRIVE_H

