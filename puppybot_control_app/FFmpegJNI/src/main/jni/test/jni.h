/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __FAKE_JNI_H__
#define __FAKE_JNI_H__

/**
    This fakes the JNI defines so we can build JNI code for testing in plain C/C++.
    Don't accidentally put this header on the include search path in a real JNI build!
*/

#include <string>
#include <stdint.h>

#define JNIEXPORT
#define JNICALL

typedef int32_t jint;
typedef float   jfloat;
typedef double  jdouble;
typedef int64_t jlong;
typedef int8_t  jboolean;
static constexpr jint JNI_VERSION_1_2 = 12;
static constexpr jint JNI_OK = 10;

struct jobject
{
    jobject() {}
    jobject(int) {}
};

struct jstring
{
    jstring() {}
    jstring( const jstring& other ) : str(other.str) {}
    std::string str;
};

struct JNIEnv
{
    const char* GetStringUTFChars( jstring& j, jboolean* isCopy )
    {
        if ( isCopy != nullptr )
        {
            *isCopy = false;
        }
        return j.str.c_str();
    }

    void ReleaseStringUTFChars( jstring&, const char* utf ) {}

    jobject NewGlobalRef(jobject& jo) { return jo; }
    void DeleteGlobalRef(jobject& jo) {}
};

struct JavaVM
{
    JavaVM( JNIEnv* mockEnv ) { pEnv = mockEnv; }

    void AttachCurrentThread( JNIEnv** env, int ) {};
    void DetachCurrentThread() {};

    int GetEnv(void** env, const jint& ) { *reinterpret_cast<JNIEnv**>(env) = pEnv; return JNI_OK; }

    JNIEnv* pEnv;
}; 

#endif /* __FAKE_JNI_H__ */

