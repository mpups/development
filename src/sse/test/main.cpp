#include <stdio.h>
#include <stdlib.h>

#include <glkcore.h>

#include "../SimdKernels.h"

#define ITRS 100000

int main(int argc, char** argv)
{
  float mat[16] __attribute__ ((aligned (16)))
  = {
    .5,0,0,0,
    0,1,0,0,
    0,0,2,0,
    1,0,0,3
  };

  float vec[12] __attribute__ ((aligned (16)))
  = {
    1,-2,-3,4,-5,6,7,-8,-1.5,-2.5,-0.f,7.f
  };

  for(int i=0;i<12;++i) { fprintf(stderr," %f",vec[i]); } fprintf(stderr," ->");
  MulMatVecs_sse(3,mat,vec,vec);
  for(int i=0;i<12;++i) { fprintf(stderr," %f",vec[i]); } fprintf(stderr,"\n");
  //return 0;

  bool verbose = true;//false;
  unsigned int SIZE = 0;
  
  if (argc==2) sscanf(argv[1],"%ud",&SIZE);
  else SIZE=256;

  //if (SIZE>16384) SIZE=0;

  fprintf(stderr,"Array lengths: %d\n",SIZE);

  GLK::Timer t;
  float time,time_sse;
  float r,r_sse;

  float* a;
  posix_memalign((void**)&a,16,SIZE*sizeof(float));
  float* b;
  posix_memalign((void**)&b,16,SIZE*sizeof(float));

  for (unsigned int i=0;i<SIZE;i++)
    {
      a[i] = i ;
      b[i] = i + (i/(float)SIZE);
    } 

  t.Reset();
  for (unsigned int i=0;i<ITRS;++i)
    {
      r = sum(SIZE,a);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r_sse = sum_sse(SIZE,a);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSummation:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%f\n%f (sse gain: %fx)\n",r,r_sse,time/time_sse);

  t.Reset();
  for (unsigned int i=0;i<ITRS;++i)
    {
      r = square(SIZE,a);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r_sse = square_sse(SIZE,a);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSquare:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%f\n%f (sse gain: %fx)\n",r,r_sse,time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r = dot(SIZE,a,b);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r_sse = dot_sse(SIZE,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nDot prod:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%f\n%f (sse gain: %fx)\n\n",r,r_sse,time/time_sse);
  
  float* c;
  posix_memalign((void**)&c,16,SIZE*sizeof(float));

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      add(SIZE,a,b,c);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      add_sse(SIZE,a,b,c);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nAddition:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      sub(SIZE,a,b,c);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      sub_sse(SIZE,a,b,c);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSubtraction:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      set(SIZE,0.f ,a);
      set(SIZE,0.5f,b);
      set(SIZE,1.f ,c);
    }
  time = t.GetSeconds();

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      set_sse(SIZE,0.f ,a);
      set_sse(SIZE,0.5f,b);
      set_sse(SIZE,1.f ,c);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSet:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r=ssd(SIZE,a,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      r_sse=ssd_sse(SIZE,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSSD:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%f\n%f (sse gain: %fx)\n\n",r,r_sse,time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      scale(SIZE,3.5f,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      scale_sse(SIZE,3.5f,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nScale:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      copy(SIZE,a,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      copy_sse(SIZE,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nCopy:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      swap(SIZE,a,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      swap_sse(SIZE,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nSwap:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      vabsf(SIZE,a,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      vabsf_sse(SIZE,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nVector Abs(float):\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      mac(SIZE,i/50000.f,a,b);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      mac_sse(SIZE,i/50000.f,a,b);
    }
  time_sse = t.GetSeconds();

  fprintf(stderr,"\nMAC:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);

  free(c);
  free(b);
  free(a);
  
  fprintf(stderr,"Working with integer types:\n");

  unsigned char* chrs1;
  posix_memalign((void**)&chrs1,16,SIZE*sizeof(unsigned char));
  unsigned char* chrs2;
  posix_memalign((void**)&chrs2,16,SIZE*sizeof(unsigned char));
  unsigned char* chrs3;
  posix_memalign((void**)&chrs3,16,SIZE*sizeof(unsigned char));

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      zero(SIZE,chrs1);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      zero_sse(SIZE,chrs1);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nClear chars:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);
  
  for (unsigned int i=0;i<SIZE;++i)
    {
      chrs1[i] = i%256;
      chrs2[i] = i%128;
    }

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      sdiff(SIZE,chrs1,chrs2,chrs3);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      sdiff_sse(SIZE,chrs1,chrs2,chrs3);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nSaturated difference:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);

  unsigned int v,v_sse;

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      v = sad(SIZE,chrs1,chrs2);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      v_sse = sad_sse(SIZE,chrs1,chrs2);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nSAD:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%u\n%u\n\t\t(sse gain: %fx)\n\n",v,v_sse,time/time_sse);

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      invert(SIZE,chrs1,chrs1);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      invert_sse(SIZE,chrs1,chrs1);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nInverse image:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);

  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      threshold(SIZE,127,chrs1,chrs1);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      threshold_sse(SIZE,127,chrs1,chrs1);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nThreshold image:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"\t\t(sse gain: %fx)\n\n",time/time_sse);

  unsigned char min,max,min_sse,max_sse;
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      minmax(SIZE,chrs1,min,max);
    }
  time = t.GetSeconds();
  t.Reset();
  for (unsigned int i=0; i<ITRS; ++i)
    {
      minmax_sse(SIZE,chrs1,min_sse,max_sse);
    }
  time_sse = t.GetSeconds();
  
  fprintf(stderr,"\nMinMax:\n");
  if (verbose) fprintf(stderr,"\ntime: %f\ntime sse: %f\n",time,time_sse);
  fprintf(stderr,"%u < %u\n%u < %u\n\t\t(sse gain: %fx)\n\n",min,max,min_sse,max_sse,time/time_sse);

  free(chrs1);
  free(chrs2);
  free(chrs3);

  return EXIT_SUCCESS;
}
