LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

LOCAL_ARM_MODE   := arm

LOCAL_MODULE    := ting_test

LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += ting/Thread.cpp
LOCAL_SRC_FILES += ting/WaitSet.cpp
LOCAL_SRC_FILES += ting/File.cpp
LOCAL_SRC_FILES += ting/FSFile.cpp
LOCAL_SRC_FILES += ting/Socket.cpp
LOCAL_SRC_FILES += ting/Timer.cpp

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
