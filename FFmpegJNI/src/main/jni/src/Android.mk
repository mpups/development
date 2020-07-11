LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

CPPFILES := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_MODULE := ffmpegjni
LOCAL_SRC_FILES := $(CPPFILES)
LOCAL_SHARED_LIBRARIES := avcodec avformat avutil swscale videolib robolib
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -ljnigraphics
include $(BUILD_SHARED_LIBRARY)