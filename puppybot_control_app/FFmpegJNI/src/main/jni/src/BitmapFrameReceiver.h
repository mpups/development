/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __BITMAP_FRAME_RECEIVER_H__
#define __BITMAP_FRAME_RECEIVER_H__

#include "FrameReceiver.h"

#include <jni.h>
#include <android/bitmap.h>

/**
    Implement the frame receiver interface and extract streamed frames into an Android native bitmap.
*/
class BitmapFrameReceiver : public FrameReceiver
{
public:
    BitmapFrameReceiver( JavaVM* env, jobject jbitmap );
    virtual ~BitmapFrameReceiver();

    virtual void LockReceiver();
    virtual void UnlockReceiver();
    virtual uint8_t* GetBuffer();
    virtual int GetStride();

private:
    JavaVM* m_jvm;
    jobject m_jbitmap;
    AndroidBitmapInfo m_bitmapInfo;
    uint16_t* m_bitmapData;
};

#endif /* __BITMAP_FRAME_RECEIVER_H__ */

