LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE    := arm
LOCAL_CFLAGS += -DFCAM_PLATFORM_ANDROID

#Set FCAM debug level 
# * Our convention is:
# * 0: Error conditions also get printed.
# * 1: Warnings and unusual conditions get printed.
# * 2: Major normal events like mode switches get printed.
# * 3: Minor normal events get printed. 
# * 4+: Varying levels of trace */
ifeq ($(NDK_DEBUG),1)
  LOCAL_CFLAGS += -DFCAM_DEBUG_LEVEL=4
  LOCAL_CFLAGS += -DDEBUG
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON  := true
  ARCH_ARM_HAVE_NEON := true
endif
endif

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += Action.cpp AutoExposure.cpp AutoFocus.cpp
LOCAL_SRC_FILES += AutoWhiteBalance.cpp AsyncFile.cpp 
LOCAL_SRC_FILES += Base.cpp Device.cpp Event.cpp Flash.cpp
LOCAL_SRC_FILES += Frame.cpp Image.cpp Lens.cpp Shot.cpp
LOCAL_SRC_FILES += Sensor.cpp Time.cpp TagValue.cpp processing/DNG.cpp
LOCAL_SRC_FILES += processing/TIFF.cpp processing/TIFFTags.cpp processing/Dump.cpp
LOCAL_SRC_FILES += processing/JPEG.cpp processing/Demosaic.cpp processing/Color.cpp
LOCAL_SRC_FILES += Tegra/AutoFocus.cpp Tegra/Shot.cpp
LOCAL_SRC_FILES += Tegra/Platform.cpp Tegra/Sensor.cpp Tegra/Frame.cpp
LOCAL_SRC_FILES += Tegra/Statistics.cpp Tegra/Lens.cpp Tegra/Flash.cpp
LOCAL_SRC_FILES += Tegra/Daemon.cpp Tegra/YUV420.cpp

LOCAL_MODULE    := fcamlib
LOCAL_MODULE_FILENAME := libFCam

LOCAL_STATIC_LIBRARIES += libjpeg
LOCAL_EXPORT_STATIC_LIBRARIES += libjpeg

LOCAL_SHARED_LIBRARIES += fcamhal
LOCAL_EXPORT_SHARED_LIBRARIES := fcamhal

LOCAL_LDLIBS := -llog
LOCAL_EXPORT_LDLIBS := -llog

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include

LOCAL_CFLAGS += -O3

# This is taken care of by the master Android.mk.
# $(call import-module, fcam/external/libjpeg)

include $(BUILD_SHARED_LIBRARY)
