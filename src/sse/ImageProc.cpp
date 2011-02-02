#include "ImageProc.h"

#include <stdlib.h>
#include <memory.h>

void zero(unsigned int n, unsigned char* a)
{
  memset(a,0,n*sizeof(unsigned char));
}

void sdiff(unsigned int n, unsigned char* a, unsigned char* b, unsigned char* c)
{
  unsigned char* end = a+n;

  while (a<end)
    {
      short diff = (*a++) - (*b++);
      if (diff<0) diff = 0;
      *c++ = diff;
    }

}

unsigned int sad(unsigned int n, unsigned char* a, unsigned char* b)
{
  unsigned int sum = 0;

  unsigned char* end = a+n;
  while (a<end)
    {
      sum += abs((*a) - (*b));
      ++a; ++b;
    }

  return sum;
}

void invert(unsigned int n, unsigned char* a, unsigned char* b)
{
  unsigned char* end = a+n;
  while (a!=end)
    {
      (*b) = 255 - (*a);
      ++a; ++b;
    }
}

void threshold(unsigned int n, unsigned char threshold, unsigned char* a, unsigned char* b)
{
  unsigned char tab[256];
  for (unsigned int i=0;i<256;++i) {
    if (i<threshold) tab[i] = 0;
    else tab[i] = 255;
  }

  unsigned char* end = a+n;
  while (a!=end)
    {
      *b = tab[*a];// < threshold ? 0 : 255;
      ++a; ++b;
    }
}

void minmax(unsigned int n, unsigned char* a, unsigned char& min, unsigned char& max)
{
  unsigned char* end = a+n;
  min = max = *a++;
  while (a<end)
    {
      if ((*a) > max) max = (*a);
      else if ( (*a) < min) min = (*a);
      ++a;
    }
}
