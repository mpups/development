/*
    Copyright (C) Mark Pupilli 2012, All rights reserved

    Implementations of very basic image processing operations.
*/
#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

#include "Image.h"

namespace robo
{

uint32_t Sad8x8( const Image<uint8_t>& image, const PixelCoord& pos, const Image<uint8_t>& patch );


} // end of namespace robo

#endif /* __IMAGE_PROCESSING_H__ */

