/*
**  Copyright (c) Mark Pupilli 2007, all rights reserved.
**
**  SSE, Streaming SIMD Extended, vector-vector operations.
*/

#ifndef VECTOR_VECTOR_SSE_H
#define VECTOR_VECTOR_SSE_H

#include <stdio.h>
#include <math.h>

float sum_sse   (unsigned int n, float* a);

void  scale_sse (unsigned int n, float val, float* a);
void  set_sse   (unsigned int n, float val, float* a); 

void  mac_sse   (unsigned int n, float val, float* a, float* b);

float dot_sse   (unsigned int n, float* a, float* b);
float ssd_sse   (unsigned int n, float* a, float* b);

void copy_sse   (unsigned int n, float* a, float* b);
void swap_sse   (unsigned int n, float* a, float* b);
void vabsf_sse  (unsigned int n, float* a, float* b);

void  add_sse   (unsigned int n, float* a, float* b, float* c);
void  sub_sse   (unsigned int n, float* a, float* b, float* c);

inline
float square_sse(unsigned int n, float* a) { return dot_sse(n,a,a); }

inline
float nrm2_sse  (unsigned int n, float* a) { return sqrtf(square_sse(n,a)/n); }

#endif
