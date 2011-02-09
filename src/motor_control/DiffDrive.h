#include "MotionMind.h"

/**
    Class for controlling simple differential drive robots.
    
    Functionality includes configuration management and speed settings.
**/
class DiffDrive
{
public:

    struct MotorData
    {
        double leftTime;
        double rightTime;
        double leftPos;
        double rightPos;
    };

    DiffDrive( MotionMind& motors );
    virtual ~DiffDrive();
    
    MotorData SetSpeeds( float left_counts_per_sec, float right_counts_per_sec );
    MotorData SetMoves( float left_counts, float right_counts );
    MotorData SetSpeeds_mps( float left_metres_per_sec, float right_metres_per_sec );
    MotorData SetMotion( float velocity_metres_per_sec, float angular_deg_per_sec );
    
    float GetLeftAmps();
    float GetRightAmps();
    float GetLeftPwm();
    float GetRightPwm();
    
protected:
    float ComputeCountsPerRev() const;
    float ComputeRevsPerMetre() const;
    
private:
    MotionMind& m_motors;
    float m_wheelbaseMetres;
    float m_countsPerMetre;
    float m_gearRatio;
    float m_countsPerRevIn;
};

