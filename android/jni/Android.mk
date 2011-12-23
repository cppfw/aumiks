LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

LOCAL_ARM_MODE   := arm

LOCAL_MODULE    := aumiks_test

LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += ting/Thread.cpp
LOCAL_SRC_FILES += ting/WaitSet.cpp
LOCAL_SRC_FILES += ting/File.cpp
LOCAL_SRC_FILES += ting/FSFile.cpp
LOCAL_SRC_FILES += ting/Socket.cpp
LOCAL_SRC_FILES += ting/Timer.cpp
LOCAL_SRC_FILES += aumiks/aumiks.cpp
LOCAL_SRC_FILES += aumiks/WavSound.cpp
LOCAL_SRC_FILES += AssetFile/AssetFile.cpp

LOCAL_CFLAGS := -Wno-div-by-zero #disable integer division by zero warning as it is sometimes useful when working with templates
LOCAL_CFLAGS += -DDEBUG

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lOpenSLES
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
