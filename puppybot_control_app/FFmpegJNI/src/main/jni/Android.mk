LOCAL_PATH := $(call my-dir)

PREBUILD_PATH := $(LOCAL_PATH)/prebuilt/install/$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := $(PREBUILD_PATH)/ffmpeg/lib/libavcodec.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/ffmpeg/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := $(PREBUILD_PATH)/ffmpeg/lib/libavformat.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/ffmpeg/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := $(PREBUILD_PATH)/ffmpeg/lib/libavutil.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/ffmpeg/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := $(PREBUILD_PATH)/ffmpeg/lib/libswscale.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/ffmpeg/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := videolib
LOCAL_SRC_FILES := $(PREBUILD_PATH)/usr/local/lib/libvideolib.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/usr/local/include
$(warning $(LOCAL_EXPORT_C_INCLUDES))
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := robolib
LOCAL_SRC_FILES := $(PREBUILD_PATH)/usr/local/lib/librobolib.so
LOCAL_EXPORT_C_INCLUDES := $(PREBUILD_PATH)/usr/local/include/
LOCAL_EXPORT_C_INCLUDES += $(PREBUILD_PATH)/usr/local/src/motor_control
LOCAL_EXPORT_C_INCLUDES += $(PREBUILD_PATH)/usr/local/src/packetcomms
LOCAL_EXPORT_C_INCLUDES += $(PREBUILD_PATH)/usr/local/src/network
LOCAL_EXPORT_C_INCLUDES += $(PREBUILD_PATH)/usr/local/src/robotcomms
LOCAL_EXPORT_C_INCLUDES += /home/markp/development/cereal-1.2.2/include
include $(PREBUILT_SHARED_LIBRARY)

include $(LOCAL_PATH)/src/Android.mk
include $(CLEAR_VARS)
