#ifndef BATCH_OPERATIONS_SSE_H
#define BATCH_OPERATIONS_SSE_H

void MulMatVecs_sse(unsigned int n, float* matrix, float* vecsIn, float* vecsOut); ///< batch multiply n vectors by 4x4 matrix

void TransformVecs_sse(unsigned int n, float* matrix, float* vecsIn, float* vecsOut); ///< batch transform n vectors by 4x4 matrix. Assumes w component of vectors==1

#endif
