LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

LOCAL_MODULE    := aumiks

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += aumiks/aumiks.cpp
LOCAL_SRC_FILES += aumiks/WavSound.cpp

LOCAL_CFLAGS := -Wno-div-by-zero #disable integer division by zero warning as it is sometimes useful when working with templates
# LOCAL_CFLAGS += -DDEBUG

LOCAL_LDLIBS    := -lOpenSLES

LOCAL_SHARED_LIBRARIES := ting

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

