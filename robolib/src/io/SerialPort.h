#ifndef ROBO_PLATFORM_SELECT_SERIAL_PORT_H
#define ROBO_PLATFORM_SELECT_SERIAL_PORT_H

#ifdef __linux

#include "linux/SerialPort.h"

#elif defined WIN32

#include "win32/SerialPort.h"

#endif

#endif

