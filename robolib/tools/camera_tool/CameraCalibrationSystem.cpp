#include "CameraCalibrationSystem.h"

#include <RoboLibOpenCv.h>

/**
    Initialise the camera calibration system.

    @param boardWidth Width of calibration pattern (number of squares, not number of corners).
    @param boardWidth Height of calibration pattern (number of squares, not number of corners).
    @param squareSize Dimension of each calibration square in millimetres [mm].
**/
CameraCalibrationSystem::CameraCalibrationSystem( int boardWidth, int boardHeight, float squareSize )
:
    m_hzCorners     ( boardWidth-1 ),
    m_vtCorners     ( boardHeight-1 ),
    m_squareSize_mm ( squareSize ),
    m_numCorners    ( 0 ),
    m_foundAll      ( 0 ),
    m_cameraMatrix  ( 0 ),
    m_distCoeff     ( 0 ),
    m_undistortX    ( 0 ),
    m_undistortY    ( 0 )
{
    
}

/**
    Frees all saved images and other resources.
**/
CameraCalibrationSystem::~CameraCalibrationSystem()
{
    // free all the calibration images that were saved:
    for( IplImage*& ptr : m_calibrationImages )
    {
        cvReleaseImage( &ptr );
    }
    cvReleaseMat( &m_cameraMatrix );
    cvReleaseMat( &m_distCoeff );
    cvReleaseImage( &m_undistortX );
    cvReleaseImage( &m_undistortY );
}

/**
    Add a new image to be included in the calibration set.

    The image will only be added if the chessboard can be detected.

    @param w Width of calibration image in pixels
    @param h Height of calibration image in pixels
    @buffer Pointer to grey-scale image buffer of w*h pixels.
**/
void CameraCalibrationSystem::AddCalibrationImage( uint32_t w, uint32_t h, const uint8_t* buffer )
{
    IplImage* img = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 1 );
    FillIplImage( buffer, img );

    m_foundAll = cvFindChessboardCorners( img, cvSize( m_hzCorners, m_vtCorners ), m_corners, &m_numCorners, CV_CALIB_CB_ADAPTIVE_THRESH );
    if ( m_foundAll )
    {
        // all found so refine their positions:
        cvFindCornerSubPix( img, m_corners, m_numCorners, cvSize(3,3), cvSize(-1,-1), cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 ) );

        // Add image to calibration list:
        m_calibrationImages.push_back( img );

        fprintf( stderr, "Added calibration image (found %d corners).\n", m_numCorners );
    }
    else
    {
        // This image is no good for calibration so delete it:
        cvReleaseImage( &img );
        fprintf( stderr, "No chess-board found.\n" );
    }
}

/**
    Computes the camera calibration using the images that have been added to the
    calibration set so far.
**/
void CameraCalibrationSystem::ComputeCalibration()
{
    const size_t imageCount = m_calibrationImages.size();

    if ( imageCount < 2 )
    {
        return;
    }
 
    CvSize imageSize = cvSize( m_calibrationImages.front()->width, m_calibrationImages.front()->height );
    uint32_t pointCount = m_hzCorners*m_vtCorners;
  
    fprintf( stderr, "\nAttempting calibration with %lu images of %d points...\n", imageCount, pointCount );

    CvMat* imagePoints  = cvCreateMat( 1, imageCount*pointCount, CV_32FC2 );
    CvMat* objectPoints = cvCreateMat( 1, imageCount*pointCount, CV_32FC3 );

    size_t i = 0;
    for ( const IplImage* img : m_calibrationImages )
    {
        CvPoint2D32f* pImgPoints = reinterpret_cast<CvPoint2D32f*>( imagePoints->data.fl ) + (i * pointCount);      
        int foundAll = cvFindChessboardCorners( img, cvSize( m_hzCorners, m_vtCorners ), pImgPoints, &m_numCorners, CV_CALIB_CB_ADAPTIVE_THRESH );
        assert( foundAll ); // We checked each image had all corners detected when we saved it!
        cvFindCornerSubPix( img, pImgPoints, m_numCorners, cvSize(3,3), cvSize(-1,-1), cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 ) );

        // Create store the calibration object points for each image:
        CvPoint3D32f* pObj = reinterpret_cast<CvPoint3D32f*>( objectPoints->data.fl ) + (i * pointCount);
        for ( uint32_t r = 0; r < m_vtCorners; r++ )
        {
            for ( uint32_t c = 0; c < m_hzCorners; c++ )
            {
                *pObj++ = cvPoint3D32f( r * m_squareSize_mm, c * m_squareSize_mm, 0.f );
            }
        }

        i += 1;
    }

    CvMat* pointCounts = cvCreateMat( 1, imageCount, CV_32SC1 );
    cvSet( pointCounts, cvScalar(pointCount) );

    CvMat* extrParams = cvCreateMat( imageCount, 6, CV_32FC1 );

    if ( m_distCoeff == 0 )
    {    
        // calibration parameters:
        m_distCoeff    = cvCreateMat( 1, 5, CV_64F );
        m_cameraMatrix = cvCreateMat( 3, 3, CV_64F );
        
        // undistortion maps:
        m_undistortX = cvCreateImage( imageSize, IPL_DEPTH_32F, 1 );
        m_undistortY = cvCreateImage( imageSize, IPL_DEPTH_32F, 1 );
    }

    CvMat rotVects, transVects;
    cvGetCols( extrParams, &rotVects, 0, 3 );
    cvGetCols( extrParams, &transVects, 3, 6 );

    cvZero( m_cameraMatrix );
    cvZero( m_distCoeff );

    int flags = CV_CALIB_ZERO_TANGENT_DIST;
    m_cameraMatrix->data.db[0] = 380.f;
    m_cameraMatrix->data.db[4] = 380.f;
    m_cameraMatrix->data.db[2] = 320.f;
    m_cameraMatrix->data.db[5] = 240.f;
    m_cameraMatrix->data.db[8] = 1.f;

    // This function computes the calibration:
    cvCalibrateCamera2( objectPoints, imagePoints, pointCounts,
                        imageSize, m_cameraMatrix, m_distCoeff,
                        &rotVects, &transVects, flags );

    // Compute the reprojection errors for the calibration:
    CvMat* reprojErrs = cvCreateMat( 1, imageCount, CV_64FC1 );
    m_avgError = compute_reprojection_error( objectPoints,
                                                  &rotVects, &transVects,
                                                  m_cameraMatrix, m_distCoeff,
                                                  imagePoints, pointCounts, reprojErrs );

    // Compute and keep an undistortion map for later use:
    cvInitUndistortMap( m_cameraMatrix, m_distCoeff, m_undistortX, m_undistortY );

    // free all data    
    cvReleaseMat( &reprojErrs );
    cvReleaseMat( &extrParams );

    cvReleaseMat( &pointCounts );
    cvReleaseMat( &objectPoints );
    cvReleaseMat( &imagePoints );

    m_numCorners = 0;

    fprintf( stderr, "\t...done.\n");
}

/**
    Undistorts the specified image in-place using the current calibration parameters.
**/
void CameraCalibrationSystem::UndistortImage( uint32_t w, uint32_t h, uint8_t* buffer )
{
    IplImage* img  = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 1 );
    IplImage* img2 = cvCloneImage( img );

    FillIplImage( buffer, img );
    assert( m_undistortX != 0 );
    assert( m_undistortY != 0 );
    cvRemap( img, img2, m_undistortX, m_undistortY );
    SpillIplImage( img2, buffer );    

    cvReleaseImage( &img );
    cvReleaseImage( &img2 );
}

/**
    Return true if the calibration has been computed, false otherwise.

    This function returning true says nothing about the quality of the
    calibration, only that a calibration has been computed.

    @return True if calibrated, false otherwise.
**/
bool CameraCalibrationSystem::Calibrated()
{
    return m_undistortX != 0;
}

/**
    @return true if the calibration system found and ordered all corners in the last image
    it was given, false otherwise.
*/
bool CameraCalibrationSystem::FoundAll() const
{
    return m_foundAll;
}

void CameraCalibrationSystem::Print( FILE* fp )
{
    if ( m_distCoeff )
    {
        //fprintf( fp, "\ncamera.width=%d\ncamera.height=%d\n", m_camera->GetFrameWidth(), m_camera->GetFrameHeight() );
        //fprintf( fp, "camera.guid=\"%llX\"\ncamera.cx=%f\ncamera.cy=%f\n", m_camera->GetGuid(), m_cameraMatrix->data.db[2],m_cameraMatrix->data.db[5] );
        fprintf( fp, "camera.fx=%f\ncamera.fy=%f\n", m_cameraMatrix->data.db[0],m_cameraMatrix->data.db[4] );
        fprintf( fp, "camera.k1=%f\ncamera.k2=%f\ncamera.k3=%f\n", m_distCoeff->data.db[0], m_distCoeff->data.db[1], m_distCoeff->data.db[4] );
        fprintf( fp, "camera.residual=%f\n", m_avgError );
    }
}

void CameraCalibrationSystem::ToString( char* info, int size )
{
    if ( m_distCoeff )
    {
        float k1 = m_distCoeff->data.db[0];
        float k2 = m_distCoeff->data.db[1];
        float k3 = m_distCoeff->data.db[4];

        int n = snprintf( info, size, "\nCalibration result (average error = %f pixels):\nk1, k2, k3 = %f, %f, %f\n", m_avgError, k1, k2, k3 );

        if ( n >= size )
        {
            return;
        }

        snprintf( info + n, size - n, "cx, cy = %f, %f\nfx, fy = %f, %f", m_cameraMatrix->data.db[2],m_cameraMatrix->data.db[5],m_cameraMatrix->data.db[0], m_cameraMatrix->data.db[4] );
    }
}


