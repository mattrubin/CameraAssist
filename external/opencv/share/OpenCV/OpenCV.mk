# In order to compile your application under cygwin
# you need to define NDK_USE_CYGPATH=1 before calling ndk-build
USER_LOCAL_PATH:=$(LOCAL_PATH)
OPENCV_THIS_DIR:=$(patsubst $(LOCAL_PATH)\\%,%,$(patsubst $(LOCAL_PATH)/%,%,$(call my-dir)))
OPENCV_LIBS_DIR:=$(OPENCV_THIS_DIR)/../..

OPENCV_BASEDIR:=
OPENCV_LOCAL_C_INCLUDES:="$(LOCAL_PATH)/$(OPENCV_THIS_DIR)/../../include/opencv" "$(LOCAL_PATH)/$(OPENCV_THIS_DIR)/../../include"

OPENCV_LIB_TYPE := STATIC
OPENCV_LIB_SUFFIX := a

# Define modules for 3rd party libraries
# Note that libjpeg and libpng will be included by the FCamera makefile.
OPENCV_EXTRA_COMPONENTS := libtiff libjasper zlib
include $(CLEAR_VARS)
LOCAL_MODULE := libtiff
LOCAL_LDLIBS := -lz
LOCAL_SRC_FILES := $(OPENCV_THIS_DIR)/3rdparty/libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := libjasper
LOCAL_LDLIBS := -lz
LOCAL_SRC_FILES := $(OPENCV_THIS_DIR)/3rdparty/libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := zlib
LOCAL_LDLIBS := -lz
LOCAL_SRC_FILES := $(OPENCV_THIS_DIR)/3rdparty/libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)

# Define OpenCV modules
OPENCV_MODULES := contrib calib3d objdetect features2d video imgproc highgui ml legacy flann core
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_contrib
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_calib3d
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_objdetect
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_features2d
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_video
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_imgproc
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_highgui
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_ml
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_legacy
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_flann
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv_core
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
LOCAL_STATIC_LIBARARIES := libjpeg libpng libjasper libtiff zlib
LOCAL_EXPORT_STATIC_LIBARARIES := libjpeg libpng libjasper libtiff zlib
include $(PREBUILT_STATIC_LIBRARY)

ifneq ($(OPENCV_BASEDIR),)
	OPENCV_LOCAL_C_INCLUDES += $(foreach mod, $(OPENCV_MODULES), $(OPENCV_BASEDIR)/modules/$(mod)/include)
endif

OPENCV_LOCAL_LIBRARIES	:= $(OPENCV_MODULES)
OPENCV_LOCAL_LIBRARIES	+= $(OPENCV_EXTRA_COMPONENTS)
OPENCV_LOCAL_CFLAGS	:= -fPIC -DANDROID -fsigned-char

LOCAL_C_INCLUDES	:= $(OPENCV_LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES	:= $(OPENCV_LOCAL_C_INCLUDES)

LOCAL_STATIC_LIBRARIES   	:= $(OPENCV_LOCAL_LIBRARIES)
LOCAL_EXPORT_STATIC_LIBRARIES   := $(OPENCV_LOCAL_LIBRARIES)

LOCAL_CFLAGS		:= $(OPENCV_LOCAL_CFLAGS)
LOCAL_LDLIBS		:= -lz
LOCAL_EXPORT_LDLIBS	:= -lz

LOCAL_MODULE := opencv
LOCAL_MODULE_FILENAME := libOpenCV
LOCAL_SRC_FILES :=

include $(BUILD_SHARED_LIBRARY)
