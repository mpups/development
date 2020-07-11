#ifndef DIFF_DRIVE_CONFIG_H
#define DIFF_DRIVE_CONFIG_H

/**
    A DiffDriveConfig object holds all the parameters
    needed to work with a differential drive system.

    It is assumed both sets of wheels, motors, and encoders have the same specification.
**/
class DiffDriveConfig
{
public:
    DiffDriveConfig( float base_m, float cpm, float gr, float cpri );
    virtual ~DiffDriveConfig();

    float WheelBaseInMetres() const { return m_wheelbaseMetres; };
    float CountsPerMetre()    const { return m_countsPerMetre; };
    float GearRatio()         const { return m_gearRatio; };
    float CountsPerRevOnInputShaft()  const { return m_countsPerRevIn; };
    float CountsPerRevOnOutputShaft() const { return m_countsPerRevOut; };

private:
    float m_wheelbaseMetres;
    float m_countsPerMetre;
    float m_gearRatio;
    float m_countsPerRevIn;
    float m_countsPerRevOut;
};

#endif // DIFF_DRIVE_CONFIG_H

