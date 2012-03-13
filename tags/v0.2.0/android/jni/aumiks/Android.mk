LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

LOCAL_MODULE    := aumiks

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += aumiks/aumiks.cpp
LOCAL_SRC_FILES += aumiks/WavSound.cpp

LOCAL_CFLAGS := -Wno-div-by-zero #disable integer division by zero warning as it is sometimes useful when working with templates
# LOCAL_CFLAGS += -DDEBUG

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../ting

LOCAL_LDLIBS    := -lOpenSLES

LOCAL_SHARED_LIBRARIES := ting

include $(BUILD_SHARED_LIBRARY)

