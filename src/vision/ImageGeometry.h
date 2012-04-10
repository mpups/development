/*
    Copyright (C) Mark Pupilli 2012, All rights reserved

    Contains definitions for various useful structures and
    types describing image coordinates, basic shapes, and
    measurements.
*/
#ifndef __IMAGE_GEOMETRY_H__
#define __IMAGE_GEOMETRY_H__

#include <stdint.h>

namespace robo
{

/**
    Stores the x and y (row and column) co-ordinates of a pixel.
*/
struct PixelCoord
{
    int32_t x;
    int32_t y;
};

/**
    Stores the position (of the top left corner) and width and height of a
    rectangle. The rectangle is aligned with image rows and columns.
*/
struct AlignedBox
{
    PixelCoord pos; //!< position of top left corner
    int32_t w; //!< width
    int32_t h; //!< height
};

} // end of namespace robo

#endif /* __IMAGE_GEOMETRY_H__ */

