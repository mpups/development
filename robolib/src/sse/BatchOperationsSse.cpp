#include "BatchOperationsSse.h"

#include <xmmintrin.h>

void MulMatVecs_sse(unsigned int n, float* m, float* vIn, float* vOut)
{
  _mm_prefetch(vIn,_MM_HINT_NTA);
  _mm_prefetch(vOut,_MM_HINT_NTA);
  
  n >>= 1; // 2 vecs per iteration
  __m128 c0,c1,c2,c3; // matrix columns - m is in column major order
  c0 = _mm_load_ps(m);
  c1 = _mm_load_ps(m+4);
  c2 = _mm_load_ps(m+8);
  c3 = _mm_load_ps(m+12);
  __m128 b1,b2,r1,r2;
  while (n)
    {
      // Scale 1st column of matrix by vectors' x-coord, results in b1,b2
      b1 = _mm_load_ss(vIn);
      b2 = _mm_load_ss(vIn+4);
      b1 = _mm_shuffle_ps(b1,b1,0);
      b2 = _mm_shuffle_ps(b2,b2,0); 
      b1 = _mm_mul_ps(b1,c0);
      b2 = _mm_mul_ps(b2,c0);
      
      // Scale 2nd column of matrix by vectors' y-coord.
      // Put results in r1,r2 (accumulating with previous results)
      r1 = _mm_load_ss(vIn+1);
      r2 = _mm_load_ss(vIn+5);
      r1 = _mm_shuffle_ps(r1,r1,0);
      r2 = _mm_shuffle_ps(r2,r2,0);
      r1 = _mm_mul_ps(r1,c1); 
      r2 = _mm_mul_ps(r2,c1);      
      r1 = _mm_add_ps(r1,b1);  
      r2 = _mm_add_ps(r2,b2);

      // Scale 3rd column of matrix by vectors' z-coord.
      // Accumulate results in r1,r2
      b1 = _mm_load_ss(vIn+2);
      b2 = _mm_load_ss(vIn+6);
      b1 = _mm_shuffle_ps(b1,b1,0);
      b2 = _mm_shuffle_ps(b2,b2,0);
      b1 = _mm_mul_ps(b1,c2); 
      b2 = _mm_mul_ps(b2,c2);
      r1 = _mm_add_ps(r1,b1);
      r2 = _mm_add_ps(r2,b2);

      // Scale 4th column of matrix by vectors' w-coord.
      // Accumulate results in r1,r2
      b1 = _mm_load_ss(vIn+3);
      b2 = _mm_load_ss(vIn+7);
      b1 = _mm_shuffle_ps(b1,b1,0);
      b2 = _mm_shuffle_ps(b2,b2,0); 
      b1 = _mm_mul_ps(b1,c3); 
      b2 = _mm_mul_ps(b2,c3); 
      r1 = _mm_add_ps(r1,b1);
      r2 = _mm_add_ps(r2,b2);
      
      vIn+=8;
      _mm_store_ps(vOut,r1);
      _mm_store_ps(vOut+4,r2);
      vOut+=8;
      --n;
    }
}

void TransformVecs_sse(unsigned int n, float* m, float* vIn, float* vOut)
{
  _mm_prefetch(vIn,_MM_HINT_NTA);
  _mm_prefetch(vOut,_MM_HINT_NTA);
  
  n >>= 1; // 2 vecs per iteration
  __m128 c0,c1,c2,c3; // matrix columns - m is in column major order
  c0 = _mm_load_ps(m);
  c1 = _mm_load_ps(m+4);
  c2 = _mm_load_ps(m+8);
  c3 = _mm_load_ps(m+12);
  __m128 b1,b2,r1,r2;
  while (n)
    {
      // Scale 1st column of matrix by vectors' x-coord, results in b1,b2
      b1 = _mm_load_ss(vIn);
      b2 = _mm_load_ss(vIn+4);
      b1 = _mm_shuffle_ps(b1,b1,0);
      b2 = _mm_shuffle_ps(b2,b2,0); 
      b1 = _mm_mul_ps(b1,c0);
      b2 = _mm_mul_ps(b2,c0);
      
      // Scale 2nd column of matrix by vectors' y-coord.
      // Put results in r1,r2 (accumulating with previous results)
      r1 = _mm_load_ss(vIn+1);
      r2 = _mm_load_ss(vIn+5);
      r1 = _mm_shuffle_ps(r1,r1,0);
      r2 = _mm_shuffle_ps(r2,r2,0);
      r1 = _mm_mul_ps(r1,c1); 
      r2 = _mm_mul_ps(r2,c1);      
      r1 = _mm_add_ps(r1,b1);  
      r2 = _mm_add_ps(r2,b2);

      // Scale 3rd column of matrix by vectors' z-coord.
      // Accumulate results in r1,r2
      b1 = _mm_load_ss(vIn+2);
      b2 = _mm_load_ss(vIn+6);
      b1 = _mm_shuffle_ps(b1,b1,0);
      b2 = _mm_shuffle_ps(b2,b2,0);
      b1 = _mm_mul_ps(b1,c2); 
      b2 = _mm_mul_ps(b2,c2);
      r1 = _mm_add_ps(r1,b1);
      r2 = _mm_add_ps(r2,b2);

      // Add final column of matrix (comprising the traslation) 
      r1 = _mm_add_ps(r1,c3);
      r2 = _mm_add_ps(r2,c3);
      
      vIn+=8;
      _mm_store_ps(vOut,r1);
      _mm_store_ps(vOut+4,r2);
      vOut+=8;
      --n;
    }
}
