/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FRAME_RECEIVER_H__
#define __FRAME_RECEIVER_H__

#include <stdint.h>
#include <jni.h>

/**
    Abstract interface for frame-receiver objects. Such objects can be passed
    to a streamer object which will then pass frames from the stream into the
    FrameReceiver for external access/processing/display.
*/
class FrameReceiver
{
public:
    FrameReceiver() {};
    virtual ~FrameReceiver() {};

    virtual void LockReceiver() = 0; //< Prepare the receiver for frame extraction - after calling it's buffer must be ready to receive a new frame and the buffer must not be accessed/altered in any way.
    virtual void UnlockReceiver() = 0; //< Once unlocked the frame receiver can use the frame data in the receive buffer and or modify the buffer.

    virtual uint8_t* GetBuffer() = 0; //< Return ptr to buffer which is large enough to hold the an entire frame's worth of data. can return null if the receiver is not ready.
    virtual int GetStride() = 0; //< Return the number of bytes between rows in the buffer returned by GetBuffer().

private:
};

#endif /* __FRAME_RECEIVER_H__ */

