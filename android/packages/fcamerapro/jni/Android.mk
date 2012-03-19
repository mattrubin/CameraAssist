LOCAL_PATH := $(call my-dir)

# Define the example build instructions
include $(CLEAR_VARS)

LOCAL_MODULE      := fcam_iface
LOCAL_ARM_MODE    := arm
TARGET_ARCH_ABI   := armeabi-v7a

ifeq ($(NDK_DEBUG),1)
  LOCAL_CFLAGS      += -DDEBUG
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON  := true
endif
endif

# Set up build configuration
LOCAL_CFLAGS 		+= -DFCAM_PLATFORM_ANDROID
LOCAL_SRC_FILES     := $(wildcard *.cpp)
LOCAL_SRC_FILES     += $(wildcard *.c)
LOCAL_STATIC_LIBRARIES += opencv_contrib opencv_calib3d opencv_objdetect opencv_features2d opencv_video opencv_imgproc opencv_highgui opencv_ml opencv_legacy opencv_flann opencv_core
LOCAL_SHARED_LIBRARIES += fcamhal fcamlib imagestack opencv
LOCAL_LDLIBS           += -llog -lGLESv2 -lEGL
include $(BUILD_SHARED_LIBRARY)

## Include the master makefile (load FCam, ImageStack, libjpeg, libpng)
$(call import-module,fcam)

## Include the OpenCV makefile (load OpenCV)
$(call __ndk_info,Including OpenCV library)
$(call import-module,fcam/external/opencv)
