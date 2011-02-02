/*
**  Copyright (c) Mark Pupilli 2007, all rights reserved.
**
**  Standard, non-vectorised, vector-vector operations.
*/

#ifndef VECTOR_VECTOR_H
#define VECTOR_VECTOR_H

#include <stdio.h>
#include <math.h>

float sum   (unsigned int n, float* a); ///< sum a[i]

void  scale (unsigned int n, float val, float* a); ///< a[i] *= val
void  set   (unsigned int n, float val, float* a); ///< a[i]  = val

void  mac   (unsigned int n, float val, float* a, float* b); ///< b += val*a 

float dot   (unsigned int n, float* a, float* b); ///< sum a[i]*b[i]
float ssd   (unsigned int n, float* a, float* b); ///< sum (a[i]-b[i])^2


void  copy  (unsigned int n, float* a, float* b); ///< b[i]=a[i]
void  swap  (unsigned int n, float* a, float* b); ///< b[i]=a[i], a[i]=b[i]
void  vabsf (unsigned int n, float* a, float* b); ///< b[i]=abs(a[i])

void  add   (unsigned int n, float* a, float* b, float* c); ///< c[i]=a[i]+b[i]
void  sub   (unsigned int n, float* a, float* b, float* c); ///< c[i]=a[i]-b[i]

inline
float square(unsigned int n, float* a) { return dot(n,a,a); } 
inline
float nrm2  (unsigned int n, float* a) { return sqrtf(square(n,a)/n); }

#endif
