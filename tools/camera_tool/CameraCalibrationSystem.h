// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef CAMERA_CALIBRATION_SYSTEM_H
#define CAMERA_CALIBRATION_SYSTEM_H

#include <cv.h>

#include <glkcore.h>

/**
    Class used for management of the camera calibration process.
**/
class CameraCalibrationSystem
{
public:
    CameraCalibrationSystem( int boardWidth, int boardHeight, float squareSize );
    ~CameraCalibrationSystem();

    void AddCalibrationImage( uint32_t w, uint32_t h, const uint8_t* img );
    void ComputeCalibration();

    void UndistortImage( uint32_t w, uint32_t h, uint8_t* img );

    bool Calibrated();

    void Print( FILE* fp );

private:
    const uint32_t m_hzCorners;
    const uint32_t m_vtCorners;
    const float m_squareSize_mm;

    CvPoint2D32f         m_corners[64];
    int                  m_numCorners;
    int                  m_foundAll;

    GLK::List<IplImage*> m_calibrationImages;

    CvMat* m_cameraMatrix;
    CvMat* m_distCoeff;
    float  m_avgError;
    IplImage* m_undistortX;
    IplImage* m_undistortY;
};

#endif // CALIBRATION_SYSTEM_H

