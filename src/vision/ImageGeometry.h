/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __IMAGE_GEOMETRY_H__
#define __IMAGE_GEOMETRY_H__

#include <stdint.h>

namespace robo
{

struct PixelCoord
{
    int32_t x;
    int32_t y;
};

struct AlignedBox
{
    PixelCoord pos; // positoin of top left corner
    int32_t w; // width
    int32_t h; // height
};

} // end of namespace robo

#endif /* __IMAGE_GEOMETRY_H__ */

