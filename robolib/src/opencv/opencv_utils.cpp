#include "opencv_utils.h"

#include <stdio.h>

/**
    Fill an IplImage from the specified buffer.
    The IplImage must have already been created
    with the correct width, height, and channels.
**/
void FillIplImage( const uint8_t* buffer, IplImage* img )
{
    uint8_t* pImg = reinterpret_cast<uint8_t*>( img->imageData );
    unsigned int jump = img->widthStep - (img->width*img->nChannels);
    for ( int r=0; r<img->height; ++r )
    {
        for ( int c=0; c<img->width; ++c )
        {
            for ( int ch=0; ch<img->nChannels; ++ch )
            {
                *pImg++ = *buffer++;
            }
        }
        pImg += jump;
    }
}

/**
    Spill an IplImage into the specified buffer.
    The buffer must have already been created
    with the correct width, height, and channels.
**/
void SpillIplImage( const IplImage* img, uint8_t* buffer )
{
    uint8_t* pImg = reinterpret_cast<uint8_t*>( img->imageData );
    unsigned int jump = img->widthStep - (img->width*img->nChannels);
    for ( int r=0; r<img->height; ++r )
    {
        for ( int c=0; c<img->width; ++c )
        {
            for ( int ch=0; ch<img->nChannels; ++ch )
            {
                *buffer++ = *pImg++;
            }
        }
        pImg += jump;
    }
}

/**
    Saves an image as a simple raw pgm.
*/
void WritePgm( const char* fileName, const uint8_t* buffer, const uint32_t width, const uint32_t height )
{
    FILE* fp = fopen( fileName, "w" );
    assert( fp != 0 );
    if ( fp )
    {
        fprintf( fp, "P5 %d %d 255\n", width, height );
        fwrite( buffer, sizeof(uint8_t), width*height, fp );
        fclose( fp );
    }
}

double compute_reprojection_error( const CvMat* object_points,
        const CvMat* rot_vects, const CvMat* trans_vects,
        const CvMat* camera_matrix, const CvMat* dist_coeffs,
        const CvMat* image_points, const CvMat* point_counts,
        CvMat* per_view_errors )
{
    CvMat* image_points2 = cvCreateMat( image_points->rows,
        image_points->cols, image_points->type );
    int i, image_count = rot_vects->rows, points_so_far = 0;
    double total_err = 0, err;
    
    for( i = 0; i < image_count; i++ )
    {
        CvMat object_points_i, image_points_i, image_points2_i;
        int point_count = point_counts->data.i[i];
        CvMat rot_vect, trans_vect;

        cvGetCols( object_points, &object_points_i,
            points_so_far, points_so_far + point_count );
        cvGetCols( image_points, &image_points_i,
            points_so_far, points_so_far + point_count );
        cvGetCols( image_points2, &image_points2_i,
            points_so_far, points_so_far + point_count );
        points_so_far += point_count;

        cvGetRow( rot_vects, &rot_vect, i );
        cvGetRow( trans_vects, &trans_vect, i );

        cvProjectPoints2( &object_points_i, &rot_vect, &trans_vect,
                          camera_matrix, dist_coeffs, &image_points2_i,
                          0, 0, 0, 0, 0 );
        err = cvNorm( &image_points_i, &image_points2_i, CV_L1 );
        if( per_view_errors )
            per_view_errors->data.db[i] = err/point_count;
        total_err += err;
    }
    
    cvReleaseMat( &image_points2 );
    return total_err/points_so_far;
}

