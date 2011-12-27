LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

LOCAL_MODULE    := aumiks_test

LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += AssetFile/AssetFile.cpp

#LOCAL_CFLAGS += -DDEBUG

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
