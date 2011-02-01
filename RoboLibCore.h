#ifndef ROBO_LIB_CORE_H
#define ROBO_LIB_CORE_H

#include "motor_control/MotionMind.h"
#include "io/SerialPort.h"
#include "io/linux/Joystick.h"

#ifndef ARM_BUILD
#include "sse/VectorVector.h"
#include "sse/BatchOperations.h"
#include "sse/SimdKernels.h"
#include "sse/BatchOperationsSse.h"
#include "sse/VectorVectorSse.h"
#endif

#endif // ROBO_LIB_CORE_H

