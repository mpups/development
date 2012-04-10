/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __PGM_H__
#define __PGM_H__

#include "Image.h"

namespace robo
{

/**
    Write a pgm file from the specified image. This will overwrite
    any existing file with the same file name.

    @param fileName name of resulting file.
    @param image image to write.
*/
bool WritePgm( const char* fileName, const Image<uint8_t>& image );

/**
    Read a pgm file with the specified fileName.

    The image will be (re-)allocated to the correct size.
*/
bool ReadPgm( const char* fileName, Image<uint8_t>& image );

} // end of namespace robo

#endif /* __PGM_H__ */

