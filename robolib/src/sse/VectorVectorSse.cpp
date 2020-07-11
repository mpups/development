#include "VectorVectorSse.h"

#include <xmmintrin.h>

/// SSE vector-vector implementations.

float sum_sse(unsigned int n, float* a)
{
  _mm_prefetch(a,_MM_HINT_NTA); // absolute first thing to do is prefetch data to L1 cache

  // multiple accumulators allow unrolled loops with fewer dependancies
  __m128 v1,sum1, v2,sum2, v3,sum3, v4,sum4;

  sum1 = _mm_xor_ps(sum1,sum1); // clear to zero
  sum2 = sum3 = sum4 = sum1; 
  
  unsigned int m = n%16;
  n = (n/16)*16;
  float* end = a+n;
  while (a<end)
    {
      v1 = _mm_load_ps(a);
      v2 = _mm_load_ps(a+4);
      v3 = _mm_load_ps(a+8);
      v4 = _mm_load_ps(a+12);
      a += 16;

      sum1 = _mm_add_ps(sum1,v1);      
      sum2 = _mm_add_ps(sum2,v2);
      sum3 = _mm_add_ps(sum3,v3);
      sum4 = _mm_add_ps(sum4,v4);
    }

  float leftOvers = 0.f;
  if (m)
    {
      if (m>=4)
	{
	  v1   = _mm_load_ps(a);
	  sum1 = _mm_add_ps(sum1,v1);
	  a += 4;
	  m -= 4;
	}
      if (m>=8)
	{
	  v1   = _mm_load_ps(a);
	  sum1 = _mm_add_ps(sum1,v1);
	  a += 4;
	  m -= 4;
	}   
      if (m>=12)
	{
	  v1   = _mm_load_ps(a);
	  sum1 = _mm_add_ps(sum1,v1);
	  a += 4;
	  m -= 4;
	}      
      
      while (m--)
	{
	  leftOvers += *a++;
	}
    }
  
  sum1 = _mm_add_ps(sum1,sum2);
  sum3 = _mm_add_ps(sum3,sum4);
  sum1 = _mm_add_ps(sum1,sum3);

  float r[4] __attribute__ ((aligned (16)));
  _mm_store_ps(r,sum1);
  return r[0]+r[1]+r[2]+r[3] + leftOvers;
}

void scale_sse(unsigned int n, float val, float* a)
{
  _mm_prefetch(a,_MM_HINT_NTA); 
  
  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;

  __m128 v1 = _mm_load1_ps(&val);
  __m128 p1,p2;

  while (a<end)
    {
      p1 = _mm_load_ps(a);
      p2 = _mm_load_ps(a+4);
     
      p1 = _mm_mul_ps(p1,v1);      
      p2 = _mm_mul_ps(p2,v1);

      _mm_store_ps(a,p1);
      _mm_store_ps(a+4,p2);

      a+=8;
    }

  if (m)
    {
      if (m>=4)
	{
	  p1   = _mm_load_ps(a);
	  p1   = _mm_mul_ps(p1,v1); 
	  _mm_store_ps(a,p1);
	  a += 4;
	  m -= 4;
	}
      
      while (m--)
	{
	  *a *= val;
	  ++a;
	}
    }

}

void set_sse(unsigned int n, float val, float* a)
{
  _mm_prefetch(a,_MM_HINT_NTA);

  unsigned int m = n%4;
  n = (n/4)*4;
  float* end = a+n;

  __m128 v = _mm_load1_ps(&val);

  while (a<end)
    {
      _mm_store_ps(a,v);
      a += 4;
    }

  while (m--)
    {
      *a++ = val;
    }

}

void mac_sse(unsigned int n, float val, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA); 
  _mm_prefetch(a,_MM_HINT_NTA);

  __m128 v1;
  v1 = _mm_load1_ps(&val);
  
  __m128 va1, vb1, va2, vb2;
  
  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;
  while (a<end)
    {
      va1 = _mm_load_ps(a);
      vb1 = _mm_load_ps(b);
      va2 = _mm_load_ps(a+4);
      vb2 = _mm_load_ps(b+4);
      
      va1 = _mm_mul_ps(va1,v1);      
      va2 = _mm_mul_ps(va2,v1);
      
      vb1 = _mm_add_ps(vb1,va1);      
      vb2 = _mm_add_ps(vb2,va2);
      a += 8;

      _mm_store_ps(b,vb1);
      _mm_store_ps(b+4,vb2);
      b += 8;
    }

  if (m)
    {
      if (m>=4)
	{
	  va1 = _mm_load_ps(a);
	  a += 4;	  
	  vb1 = _mm_load_ps(b); 
	  	  
	  va1 = _mm_mul_ps(va1,v1);
	  m -= 4;	  
	  vb1 = _mm_add_ps(vb1,va1);

	  _mm_store_ps(b,vb1);  
	  b += 4;
	}
      
      while (m--)
	{
	  *b += val * (*a);
	  ++b; ++a;
	}
    }

}

float dot_sse(unsigned int n, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA); 
  _mm_prefetch(a,_MM_HINT_NTA); 

  // We use two accumulators to have
  // two streams of instructions, with no data
  // dependancies between them.
  // Using four streams seems unbeneficial because it takes
  // too many xmm registers
  __m128 va1, vb1, p1, dot1;
  __m128 va2, vb2, p2, dot2;
  
  dot1 = _mm_xor_ps(dot1,dot1); // clear to zero
  dot2 = dot1;

  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;
  while (a<end)
    {
      va1  = _mm_load_ps(a);
      vb1  = _mm_load_ps(b);
      va2  = _mm_load_ps(a+4);
      vb2  = _mm_load_ps(b+4);
      
      p1   = _mm_mul_ps(va1,vb1);
      p2   = _mm_mul_ps(va2,vb2);
      
      a += 8;

      dot1 = _mm_add_ps(dot1,p1);
      dot2 = _mm_add_ps(dot2,p2);
            
      b+=8; 
    }

  float leftOvers = 0.f;
  if (m)
    {
      if (m>=4)
	{
	  va1  = _mm_load_ps(a);
	  vb1  = _mm_load_ps(b);
	  p1   = _mm_mul_ps(va1,vb1);
	  a += 4;

	  dot1 = _mm_add_ps(dot1,p1);
	  b += 4;
	  m -= 4;
	}

      while (m--)
	{
	  leftOvers += (*a) * (*b);
	  ++b; ++a;
	}
    }

  dot1 = _mm_add_ps(dot1,dot2);
  
  float r[4] __attribute__ ((aligned (16)));
  _mm_store_ps(r,dot1);
  return r[0]+r[1]+r[2]+r[3] + leftOvers;  
}

/**
   SSE sum of squared differences.
**/
float ssd_sse(unsigned int n, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA);
  _mm_prefetch(a,_MM_HINT_NTA);

  __m128 va1, vb1, sum1;
  __m128 va2, vb2, sum2;
  sum1 = _mm_xor_ps(sum1,sum1);
  sum2 = sum1;

  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;
  while (a<end)
    {
      va1  = _mm_load_ps(a);
      vb1  = _mm_load_ps(b); 
      va2  = _mm_load_ps(a+4);
      vb2  = _mm_load_ps(b+4);

      va1  = _mm_sub_ps(va1,vb1);  // diff
      va2  = _mm_sub_ps(va2,vb2);  

      vb1  = _mm_mul_ps(va1,va1);  // diff*diff
      vb2  = _mm_mul_ps(va2,va2);
      
      a+=8;

      sum1 = _mm_add_ps(sum1,vb1); // accumulate
      sum2 = _mm_add_ps(sum2,vb2);

      b+=8;
    }
  
  float leftOvers = 0.f;
  if (m)
    {
      if (m>=4)
	{
	  va1  = _mm_load_ps(a);
	  vb1  = _mm_load_ps(b);
	  va1  = _mm_sub_ps(va1,vb1);
	  vb1  = _mm_mul_ps(va1,va1);
	  sum1 = _mm_add_ps(sum1,vb1); 

	  a += 4; b += 4;
	  m -= 4;
	}
      
      while (m--)
	{
	  float diff = (*a) - (*b);
	  leftOvers += diff*diff;
	  ++b; ++a;
	}
    }
  
  sum1 = _mm_add_ps(sum1,sum2);
  
  float r[4] __attribute__ ((aligned (16)));
  _mm_store_ps(r,sum1);
  return r[0]+r[1]+r[2]+r[3] + leftOvers;
}

void add_sse(unsigned int n, float* a, float* b, float* c)
{
  _mm_prefetch(b,_MM_HINT_NTA); 
  _mm_prefetch(a,_MM_HINT_NTA);


  __m128 va1, vb1, va2, vb2;

  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;

  while (a<end)
    {
      va1 = _mm_load_ps(a);
      vb1 = _mm_load_ps(b);     
      va1 = _mm_add_ps(va1,vb1);

      va2 = _mm_load_ps(a+4);
      a+=8; 
      vb2 = _mm_load_ps(b+4);     
      b+=8;
      va2 = _mm_add_ps(va2,vb2);
      
      _mm_store_ps(c,va1); 
      _mm_store_ps(c+4,va2);

      c+=8;
    }

  if (m)
    {
      if (m>=4)
	{
	  va1 = _mm_load_ps(a);
	  vb1 = _mm_load_ps(b); 
	  va1 = _mm_add_ps(va1,vb1);
	  _mm_store_ps(c,va1); 	  
	  a += 4; b+=4; c+=4;
	  m -= 4;
	}
      
      while (m--)
	{
	  *c = (*a) + (*b);
	  ++b; ++a; ++c;
	}
    }

}

void sub_sse(unsigned int n, float* a, float* b, float* c)
{
  _mm_prefetch(b,_MM_HINT_NTA);
  _mm_prefetch(a,_MM_HINT_NTA);

  __m128 va1, vb1, va2, vb2;

  unsigned int m = n%8;
  n = (n/8)*8;
  float* end = a+n;

  while (a<end)
    {
      va1 = _mm_load_ps(a);
      vb1 = _mm_load_ps(b);
      va1 = _mm_sub_ps(va1,vb1);

      va2 = _mm_load_ps(a+4);
      a+=8;
      vb2 = _mm_load_ps(b+4);
      b+=8;
      va2 = _mm_sub_ps(va2,vb2);

      _mm_store_ps(c,va1);
      _mm_store_ps(c+4,va2);
      c+=8;
    }

  if (m)
    {
      if (m>=4)
	{
	  va1 = _mm_load_ps(a);
	  a+=4;
	  vb1 = _mm_load_ps(b); 
	  b+=4;
	  va1 = _mm_sub_ps(va1,vb1);
	  _mm_store_ps(c,va1); 	  
	  c+=4;
	  m -= 4;
	}
      
      while (m--)
	{
	  *c = (*a) - (*b);
	  ++b; ++a; ++c;
	}
    }

}

void copy_sse(unsigned int n, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA);
  _mm_prefetch(a,_MM_HINT_NTA);
  
  __m128 va1;

  unsigned int m = n%4;
  n = (n/4)*4;
  float* end = a+n;
  
  while (a<end)
    {
      va1 = _mm_load_ps(a);
      a+=4;
      _mm_store_ps(b,va1);
      b+=4;
    }
  
  if (m)
    {
      if (m>=4)
	{
	  va1 = _mm_load_ps(a);
	  a+=4;
	  _mm_store_ps(b,va1); 	  
	  b+=4;
	  m -= 4;
	}
      
      while (m--)
	{
	  *b++ = *a++;
	}
    }
  
}

void swap_sse(unsigned int n, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA);
  _mm_prefetch(a,_MM_HINT_NTA);
  
  __m128 va1, vb1;

  unsigned int m = n%4;
  n = (n/4)*4;
  float* end = a+n;
  
  while (a<end)
    {
      va1 = _mm_load_ps(a);
      vb1 = _mm_load_ps(b);
      
      _mm_store_ps(a,vb1);
      a+=4;
      _mm_store_ps(b,va1);
      b+=4;
    }

  if (m)
    {
      if (m>=4)
	{
	  va1 = _mm_load_ps(a);
	  vb1 = _mm_load_ps(b);
	  _mm_store_ps(a,vb1);
	  a+=4;
	  _mm_store_ps(b,va1);
	  b+=4;
	  m -= 4;
	}
      
      while (m--)
	{
	  float tmp = *a;
	  *a++ = *b;
	  *b++ = tmp;
	}
    }
  
}


void vabsf_sse(unsigned int n, float* a, float* b)
{
  _mm_prefetch(b,_MM_HINT_NTA);
  _mm_prefetch(a,_MM_HINT_NTA);
  __m128i* a128 = (__m128i*)a;
  __m128i* b128 = (__m128i*)b;
  
  unsigned int m = n%8;
  n = (n/8)*8;
  __m128i* end = a128+(n/4);

  __m128i mask;
  mask = _mm_cmpeq_epi32(mask,mask); // mask is all 1s
  mask = _mm_srli_epi32 (mask, 1);   // mask is now 0 followed by all 1s

  __m128i v1,v2;
  while (a128<end)
    {
      v1 = _mm_load_si128(a128);
      v2 = _mm_load_si128(a128+1);
      a128 += 2;
    
      v1 = _mm_and_si128(v1, mask);
      v2 = _mm_and_si128(v2, mask);
    
      _mm_store_si128(b128,v1);
      _mm_store_si128(b128+1,v2);
      b128 += 2;
    }

   if (m)
    {
      if (m>=4)
	{
	  m -= 4;
	  *b128 = _mm_and_si128(*a128,mask);
	  n += 4;
	}

      a += n;
      b += n;
      while (m--)
	{
	  (*b) = fabsf(*a);
	  ++a; ++b;
	}
    }

}
