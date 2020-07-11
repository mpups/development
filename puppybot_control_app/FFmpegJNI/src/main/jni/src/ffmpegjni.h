/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FFMPEG_JNI_H__
#define __FFMPEG_JNI_H__

#include <VideoLib.h>
#include <TcpSocket.h>
#include <PacketComms.h>
#include <PacketSerialisation.h>

#include <jni.h>
#include <android/log.h>

extern "C" {

jint JNI_OnLoad(JavaVM *vm, void *reserved);

JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_createStream( JNIEnv* env, jobject object, jstring addressString, jint port, jint timeoutInSeconds );
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameWidth( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameHeight( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jlong JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameCount( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jdouble JNICALL
Java_pups_ffmpegjni_VideoStream_getStamp( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jfloat JNICALL
Java_pups_ffmpegjni_VideoStream_getFrameRate( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_setFrameReceiver( JNIEnv* env, jobject object, jint streamId, jobject jbitmap );
JNIEXPORT jboolean JNICALL
Java_pups_ffmpegjni_VideoStream_startStreaming( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_deleteStream( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jint JNICALL
Java_pups_ffmpegjni_VideoStream_waitForFrame( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT jboolean JNICALL
Java_pups_ffmpegjni_VideoStream_isStreaming( JNIEnv* env, jobject object, jint streamId );
JNIEXPORT void JNICALL
Java_pups_ffmpegjni_VideoStream_sendJoystickUpdate( JNIEnv* env, jobject object, jint streamId, jint x, jint y, jint max );

}

static const std::vector<std::string> g_packetIds{"AvInfo", "AvData", "Odometry", "Joystick"};

#endif /* __FFMPEG_JNI_H__ */

