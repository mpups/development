#include "VectorVector.h"

#include <memory.h>

///  Bog-standard vector-vector implementation

float sum(unsigned int n, float* a)
{
  float sum = 0.f;
  
  float* end = a+n;
  while (a<end)
    {
      sum += *a++;
    }

  return sum;
}

void scale(unsigned int n, float val, float* a)
{
  float* end = a+n;

  while (a<end)
    {
      *a++ *= val;
    }
}

void set(unsigned int n, float val, float* a)
{
  float* end = a+n;

  while (a<end)
    {
      *a++ = val;
    }
}

float dot(unsigned int n, float* a, float* b)
{
  float dot = 0.f;

  float* end = a+n;
  while (a<end)
    {
      dot += (*a++) * (*b++);
    }

  return dot;
}

float ssd(unsigned int n, float* a, float* b)
{
  float* end = a+n;

  float sum = 0.f;

  while (a<end)
    {
      float diff = (*a++) - (*b++); 
      sum += diff*diff;
    }

  return sum;
}

void mac(unsigned int n, float val, float* a, float* b)
{
  float* end = a+n;

  while (a<end)
    {
      *b += val * (*a);
      ++b; ++a;
    }
}

void copy(unsigned int n, float* a, float* b)
{
  memcpy(a,b,n*sizeof(float));
}

void swap(unsigned int n, float* a, float* b)
{
  float* end = a+n;

  while (a<end)
    {
      float tmp = *a;
      *a++ = *b;
      *b++ = tmp;
    }
}

void  vabsf (unsigned int n, float* a, float* b)
{
  float* end = a+n;

  while (a<end)
    {
      *b = fabsf(*a);
      b++; a++;
    }
}

void add(unsigned int n, float* a, float* b, float* c)
{
  float* end = a+n;

  while (a<end)
    {
      *c++ = (*a++) + (*b++);
    }
}

void sub(unsigned int n, float* a, float* b, float* c)
{
  float* end = a+n;

  while (a<end)
    {
      *c++ = (*a++) - (*b++);
    }
}
