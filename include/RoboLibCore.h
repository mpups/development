#ifndef ROBO_LIB_CORE_H
#define ROBO_LIB_CORE_H

#include <VideoLib.h>

#include "../src/motor_control/MotionMind.h"
#include "../src/motor_control/DiffDrive.h"
#include "../src/io/SerialPort.h"
#include "../src/io/linux/Joystick.h"

#include "../src/utility/AsyncLooper.h"

#include "../src/motor_control/TeleJoystick.h"
#include "../src/motor_control/JoystickControl.h"

#include "../src/packetcomms/ComPacket.h"
#include "../src/packetcomms/PacketSubscriber.h"
#include "../src/packetcomms/PacketDemuxer.h"
#include "../src/packetcomms/PacketMuxer.h"
#include "../src/packetcomms/SimpleQueue.h"
#include "../src/packetcomms/PacketSerialisation.h"

#include "../src/robotcomms/VideoClient.h"

#ifndef ARM_BUILD

//#include "../src/lua/Luabot.h"

#include "../src/sse/VectorVector.h"
#include "../src/sse/BatchOperations.h"
#include "../src/sse/SimdKernels.h"
#include "../src/sse/BatchOperationsSse.h"
#include "../src/sse/VectorVectorSse.h"
#endif

#endif // ROBO_LIB_CORE_H
