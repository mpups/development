/*
**  Copyright (c) Mark Pupilli 2007, all rights reserved.
**
**  This library contains a number of useful computational kernels.
**  Each kernel is implementated in both standard and vectorised forms.
**
**  Currently only SSE simd kernels are available but the library
**  can be easily extended to other instrucion sets such as altivec/vmx.
**
**  The library contains no functionality for detection of available
**  instruction sets or runtime multi-path code. This should be handled
**  by the application directly (or in another library).
** 
*/

#ifndef SIMD_KERNELS_H
#define  SIMD_KERNELS_H

#include "VectorVector.h"
#include "VectorVectorSse.h"

#include "ImageProc.h"
#include "ImageProcSse.h"

#include "BatchOperationsSse.h"

#endif
