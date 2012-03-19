LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE    := arm

ifeq ($(NDK_DEBUG),1)
  LOCAL_CFLAGS += -DDEBUG
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON  := true
  ARCH_ARM_HAVE_NEON := true
endif
endif

LOCAL_SRC_FILES :=\
	png.c \
	pngerror.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c 

LOCAL_CFLAGS += -O3

LOCAL_LDLIBS := -lz

LOCAL_EXPORT_LDLIBS := -lz

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_MODULE    := libpng
LOCAL_MODULE_FILENAME := libpng

include $(BUILD_STATIC_LIBRARY)
