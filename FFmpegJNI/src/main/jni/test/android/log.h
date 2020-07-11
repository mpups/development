/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __MOCK_ANDROID_LOG_H__
#define __MOCK_ANDROID_LOG_H__

#include <iostream>

enum LogType
{
    ANDROID_LOG_DEBUG=0,
    ANDROID_LOG_INFO,
};

inline void __android_log_write( LogType type, const char* tag, const char* msg )
{
    std::clog << tag << ": " << msg << std::endl;
}

inline void __android_log_print( LogType type, const char* tag, const char* msg, ... )
{
    va_list myargs;
    va_start(myargs, msg);
    std::clog << tag << ": ";
    vfprintf(stderr, msg, myargs);
    std::clog << std::endl;
    va_end(myargs);
}

#endif /* __FAKE_ANDROID_LOG_H__ */

