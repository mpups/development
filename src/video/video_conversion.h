/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __VIDEO_CONVERSION_H__
#define __VIDEO_CONVERSION_H__

#include <stdint.h>

void halfscale_yuyv422_to_yuv420p( int w, int h, uint8_t* srcBuffer, uint8_t* dstBuffer );

void halfscale_yuyv422( int w, int h, uint8_t* srcBuffer, uint8_t* dstBuffer );

#endif /* __VIDEO_CONVERSION_H__ */

