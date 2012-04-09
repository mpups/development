/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __PGM_H__
#define __PGM_H__

#include "Image.h"

namespace robo
{

bool WritePgm( const char* fileName, const Image<uint8_t>& image )
{
    FILE* fp = fopen( fileName, "w" );
    assert( fp != 0 );
    if ( fp )
    {
        fprintf( fp, "P5 %d %d 255\n", image.Width(), image.Height() );

        for ( uint32_t i=0; i<image.Height(); ++i ) {
            fwrite( image[i], sizeof(uint8_t), image.Width(), fp );
        }
        fclose( fp );
        return true;
    }

    return false;
}

} // end of namespace robo

#endif /* __PGM_H__ */

