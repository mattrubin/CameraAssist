# FCamera Android.mk
# This builds all external dependencies, ranging from
# FCam, ImageStack to OpenCV.
#
FCAMERA_PATH := $(call my-dir)
LOCAL_PATH := $(call my-dir)

# Define configuration.
ifeq ($(NDK_DEBUG),1)
  PREBUILT_DIR := debug
else
  PREBUILT_DIR := release
endif

# Define the prebuilt hal library module
include $(CLEAR_VARS)
LOCAL_MODULE := fcamhal
LOCAL_SRC_FILES := prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libFCamTegraHal.so
include $(PREBUILT_SHARED_LIBRARY)

# Propagate BUILD_FROM_SRC to all submodules.
BUILD_FROM_SRC := $(strip $(PREBUILD))
BUILD_FCAM_FROM_SRC := $(BUILD_FROM_SRC)
BUILD_JPEG_FROM_SRC := $(BUILD_FROM_SRC)
BUILD_PNG_FROM_SRC := $(BUILD_FROM_SRC)
BUILD_IMAGESTACK_FROM_SRC := $(BUILD_FROM_SRC)

# If BUILD_FROM_SRC is off, check that the libraries do exist.
ifneq ($(BUILD_FROM_SRC),1)
  $(call __ndk_info,Using prebuilt libaries when possible.)
  ifeq (,$(strip $(wildcard $(LOCAL_PATH)/prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libFCam.so)))
    BUILD_FCAM_FROM_SRC := 1
  endif
  ifeq (,$(strip $(wildcard $(LOCAL_PATH)/prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libjpeg.a)))
    BUILD_JPEG_FROM_SRC := 1
  endif
  ifeq (,$(strip $(wildcard $(LOCAL_PATH)/prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libpng.a)))
    BUILD_PNG_FROM_SRC := 1
  endif
  ifeq (,$(strip $(wildcard $(LOCAL_PATH)/prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libImageStack.so)))
    BUILD_IMAGESTACK_FROM_SRC := 1
  endif
endif

## PNG module
ifneq ($(BUILD_PNG_FROM_SRC),1)
$(call __ndk_info,Using prebuilt png library)
include $(CLEAR_VARS)
LOCAL_PATH := $(FCAMERA_PATH)
LOCAL_MODULE := libpng
LOCAL_MODULE_FILENAME := libpng
LOCAL_EXPORT_C_INCLUDES := $(FCAMERA_PATH)/external/libpng
LOCAL_SRC_FILES := prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libpng.a
include $(PREBUILT_STATIC_LIBRARY)
else
$(call __ndk_info,Will rebuild libpng library if not up to date)
$(call import-module, fcam/external/libpng)
endif

## JPEG module
ifneq ($(BUILD_JPEG_FROM_SRC),1)
$(call __ndk_info,Using prebuilt jpeg library)
include $(CLEAR_VARS)
LOCAL_PATH := $(FCAMERA_PATH)
LOCAL_MODULE := libjpeg
LOCAL_MODULE_FILENAME := libjpeg
LOCAL_EXPORT_C_INCLUDES := $(FCAMERA_PATH)/external/libjpeg
LOCAL_SRC_FILES := prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)
else
$(call __ndk_info,Will rebuild libjpeg library if not up to date)
$(call import-module, fcam/external/libjpeg)
endif

# FCam module
ifneq ($(BUILD_FCAM_FROM_SRC),1)
$(call __ndk_info,Using prebuilt FCam library)
include $(CLEAR_VARS)
LOCAL_PATH := $(FCAMERA_PATH)
LOCAL_MODULE := fcamlib
LOCAL_MODULE_FILENAME := libFCam
LOCAL_EXPORT_C_INCLUDES := $(FCAMERA_PATH)/include
LOCAL_SRC_FILES := prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libFCam.so
include $(PREBUILT_SHARED_LIBRARY)
else
$(call __ndk_info,Will rebuild FCam library if not up to date)
$(call import-module, fcam/external/FCam)
endif

## ImageStack module
ifneq ($(BUILD_IMAGESTACK_FROM_SRC),1)
$(call __ndk_info,Using prebuilt ImageStack library)
include $(CLEAR_VARS)
LOCAL_PATH := $(FCAMERA_PATH)
LOCAL_MODULE := imagestack
LOCAL_MODULE_FILENAME := libImageStack
LOCAL_EXPORT_C_INCLUDES := $(FCAMERA_PATH)/include
LOCAL_SRC_FILES := prebuilt/$(PREBUILT_DIR)/lib/armeabi-v7a/libImageStack.so
include $(PREBUILT_SHARED_LIBRARY)
else
$(call __ndk_info,Will rebuild ImageStack library if not up to date)
$(call import-module, fcam/external/libImageStack)
endif
