// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef OPENCV_UTILS_H
#define OPENCV_UTILS_H

#include <cv.h>

void FillIplImage( const uint8_t* buffer, IplImage* img );
void SpillIplImage( const IplImage* img, uint8_t* buffer );

void WritePgm( const char* fileName, const uint8_t* buffer, const uint32_t width, const uint32_t height );

double compute_reprojection_error( const CvMat* object_points,
        const CvMat* rot_vects, const CvMat* trans_vects,
        const CvMat* camera_matrix, const CvMat* dist_coeffs,
        const CvMat* image_points, const CvMat* point_counts,
        CvMat* per_view_errors );

#endif // OPENCV_UTILS_H

