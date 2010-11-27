/*
**  Copyright (c) Mark Pupilli 2007, all rights reserved.
**
**  SSE, Streaming SIMD Extended, image processing operations.
*/
#ifndef IMAGE_PROC_SSE_H
#define IMAGE_PROC_SSE_H

void zero_sse(unsigned int n, unsigned char* a);

void sdiff_sse(unsigned int n, unsigned char* a, unsigned char* b, unsigned char* c); ///< saturated difference

unsigned int sad_sse(unsigned int n, unsigned char* a, unsigned char* b); ///< sum of absolute differences

void invert_sse(unsigned int n, unsigned char* a, unsigned char* b);

void threshold_sse(unsigned int n, unsigned char threshold, unsigned char* a, unsigned char* b);

void minmax_sse(unsigned int n, unsigned char* a, unsigned char& min, unsigned char& max);

#endif
