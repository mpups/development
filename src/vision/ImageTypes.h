/*
    Copyright (C) Mark Pupilli 2012, All rights reserved

    This file contains definitions for common image and pixel types.
*/
#ifndef __IMAGE_TYPES_H__
#define __IMAGE_TYPES_H__

#include "Image.h"

struct RgbPixel
{
    uint8_t r,g,b,a;
};

struct RgbaPixel
{
    uint8_t r,g,b,a;
};

typedef Image<uint8_t> GreyImage;
typedef Image<RgbPixel> RgbImage;
typedef Image<RgbaPixel> RgbaImage;

#endif /* __IMAGE_TYPES_H__ */

