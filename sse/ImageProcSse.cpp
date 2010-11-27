#include "ImageProcSse.h"

#include <assert.h>
#include <emmintrin.h>

void zero_sse(unsigned int n, unsigned char* a)
{
  __m128i v;
  v = _mm_setzero_si128();

  unsigned int m = n%16;
  n = (n/16)*16;
  unsigned char* end = a+n;
  
  while (a<end)
    {
      _mm_store_si128((__m128i*)a,v);
      a += 16;
    }

  while (m--)
    {
      *a++ = 0;
    }  
}

void sdiff_sse(unsigned int n, unsigned char* a, unsigned char* b, unsigned char* c)
{
  __m128i* a128 = (__m128i*)a;
  __m128i* b128 = (__m128i*)b;
  __m128i* c128 = (__m128i*)c;
  __m128i d1, d2;

  unsigned int m = n%32;
  n = (n/32)*32;
  __m128i* end = a128+(n/16);

  while (a128<end)
    {
      d1 = _mm_subs_epu8(*a128,*b128);
      d2 = _mm_subs_epu8(*(a128+1),*(b128+1));
      a128 += 2; b128 += 2;

      _mm_store_si128(c128,d1);
      _mm_store_si128(c128+1,d2);
      c128 += 2;
    }

  // left overs
  if (m)
    {
      if (m>=16)
	{
	  m -= 16;
	  
	  d1 = _mm_subs_epu8(*a128,*b128);
	  _mm_store_si128(c128,d1);
       	  
	  n += 16;
	}
      
      a += n;
      b += n;
      c += n;
      
      while (m--)
	{
	  *c = (*a) - (*b);
	  ++a; ++b; ++c;
	}

    }
}

unsigned int sad_sse(unsigned int n, unsigned char* a, unsigned char* b)
{
  __m128i* a128 = (__m128i*)a;
  __m128i* b128 = (__m128i*)b;
  __m128i d1, d2, sum1, sum2;

  sum1 = _mm_setzero_si128();
  sum2 = sum1;

  unsigned int m = n%32;
  n = (n/32)*32;
  __m128i* end = a128+(n/16);

  while (a128<end)
    {
      d1 = _mm_sad_epu8(*a128,*b128);
      d2 = _mm_sad_epu8(*(a128+1),*(b128+1));
      a128 += 2; b128 += 2;
    
      sum1 = _mm_add_epi64(sum1,d1);
      sum2 = _mm_add_epi64(sum2,d2);
    }

  // account for m leftovers
  unsigned int leftOvers = 0;
  if (m)
    {
      if (m>=16)
	{
	  m -= 16;
	  
	  d1 = _mm_sad_epu8(*a128,*b128);
	  sum1 = _mm_add_epi64(sum1,d1);

	  n += 16;
	}

      a += n; b += n;

      while (m--)
	{
	  leftOvers += abs((*a) - (*b));
	  a++; b++;
	}
    }

  sum1 = _mm_add_epi64(sum1,sum2);
  
  unsigned int suml = _mm_cvtsi128_si32(sum1); // lower 64 bits
  _mm_slli_epi64(sum1,64);
  unsigned int sumh = _mm_cvtsi128_si32(sum1); // upper 64-bits

  return suml + sumh + leftOvers;
}

#include <stdio.h>

void invert_sse(unsigned int n, unsigned char* a, unsigned char* b)
{
  __m128i* a128 = (__m128i*)a;
  __m128i* b128 = (__m128i*)b;
  
  unsigned int m = n%32;
  n = (n/32)*32;
  __m128i* end = a128+(n/16);

  __m128i m255i;
  m255i = _mm_cmpeq_epi32(m255i,m255i);//_mm_set1_epi8(255);
  
  __m128i v1,v2;
  while (a128<end)
    {
      v1 = _mm_load_si128(a128);
      v2 = _mm_load_si128(a128+1);

      v1 = _mm_subs_epu8(m255i,v1);
      v2 = _mm_subs_epu8(m255i,v2);
      
      _mm_store_si128(b128,v1);
      _mm_store_si128(b128+1,v2);
      a128 += 2; b128 += 2;
    }
  
  if (m)
    {
      if (m>=16)
	{
	  m -= 16;
	  *b128 = _mm_subs_epu8(m255i,*a128);
	  n += 16;
	}

      a += n;
      b += n;
      while (m--)
	{
	  (*b) = 255 - (*a);
	  ++a; ++b;
	}
    }

}

void threshold_sse(unsigned int n, unsigned char threshold, unsigned char* a, unsigned char* b)
{
  __m128i* a128 = (__m128i*)a;
  __m128i* b128 = (__m128i*)b;
  
  unsigned int m = n%32;
  n = (n/32)*32;
  __m128i* end = a128+(n/16);
  
  __m128i T    = _mm_set1_epi8(threshold+128);
  __m128i v128 = _mm_set1_epi8(128);
  __m128i v1,v2;
  while (a128<end)
    {
      v1 = _mm_load_si128(a128);
      v2 = _mm_load_si128(a128+1);

      v1 = _mm_add_epi8(v1,v128);
      v2 = _mm_add_epi8(v2,v128);

      v1 = _mm_cmpgt_epi8(v1,T);
      v2 = _mm_cmpgt_epi8(v2,T);
    
      _mm_store_si128(b128,v1);
      _mm_store_si128(b128+1,v2);
      a128 += 2; b128 += 2;
    }

  // TODO: leftovers
  if (m)
    {
    }
}

void minmax_sse(unsigned int n, unsigned char* a, unsigned char& min, unsigned char& max)
{
    __m128i* a128 = (__m128i*)a;
    __m128i min1, max1, min2, max2;
    
    min1 = _mm_load_si128(a128);
    max1 = max2 = min2 = min1;

    unsigned int m = n%16;
    n = n/16;
    __m128i* end = a128+n;
    n *= 16;
    
    __m128i v1,v2;
    while (a128<end)
      {
	v1 = _mm_load_si128(a128++);
	v2 = _mm_load_si128(a128++);

	min1 = _mm_min_epu8(min1,v1);
	max1 = _mm_max_epu8(max1,v1);

	min2 = _mm_min_epu8(min2,v2);
	max2 = _mm_max_epu8(max2,v2);
      }

    // TODO: leftovers
    if (m)
      {
	if (m>=16)
	  {
	    m -= 16;
	  
	    n += 16;
	  }
	a += n;

	while(m--)
	  {
	    
	    ++a;
	  }
      }

    min1 = _mm_min_epu8(min1,min2);
    max1 = _mm_max_epu8(max1,max2);

    unsigned char* lmin = (unsigned char*)(&min1);
    unsigned char* lmax = (unsigned char*)(&max1);
    unsigned char* e = lmin+16;
    min = *lmin++;
    max = *lmax++;
    while (lmin<e)
      {
	if ((*lmin) < min) min = (*lmin);
	if ((*lmax) > max) max = (*lmax);
	++lmin; ++lmax;
      }
}
