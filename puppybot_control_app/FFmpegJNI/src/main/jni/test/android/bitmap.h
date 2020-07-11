/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __MOCK_ANDROID_BITMAP_H__
#define __MOCK_ANDROID_BITMAP_H__

#include <jni.h>

struct AndroidBitmapInfo
{
    static const int stride = 0;
};

inline void AndroidBitmap_getInfo( JNIEnv* env, jobject& obj, AndroidBitmapInfo* info )
{
}

inline void AndroidBitmap_lockPixels( JNIEnv* env, jobject& obj, void** info )
{
    *info = nullptr; // In my code I always check if this
}

inline void AndroidBitmap_unlockPixels( JNIEnv* env, jobject& obj )
{
}

#endif /* __MOCK_ANDROID_BITMAP_H__ */

