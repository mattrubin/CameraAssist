# This simply refers to ./share/OpenCV/OpenCV.mk.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
OPENCV_MK_PATH := $(LOCAL_PATH)/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)

