#include "DiffDriveConfig.h"

/**
    Construct a differential drive configuration object.

    @param base_m Wheel base in metres.
    @param cpm Encoder counts per metre of travel.
    @param gr Gear ratio.
    @param cpri Counts per 1 revolution of the input shaft.
**/
DiffDriveConfig::DiffDriveConfig( float base_m, float cpm, float gr, float cpri )
:
    m_wheelbaseMetres ( base_m ),
    m_countsPerMetre  ( cpm ),
    m_gearRatio       ( gr ),
    m_countsPerRevIn  ( cpri )  
{
    
}

DiffDriveConfig::~DiffDriveConfig()
{
}

