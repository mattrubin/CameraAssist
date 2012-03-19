LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE    := arm
TARGET_ARCH_ABI   := armeabi-v7a

ifeq ($(NDK_DEBUG),1)
  LOCAL_CFLAGS      += -DDEBUG
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON  := true
endif
endif

LOCAL_SRC_FILES     := \
	Alignment.cpp Arithmetic.cpp Calculus.cpp Color.cpp Complex.cpp \
	Control.cpp Convolve.cpp Deconvolution.cpp DFT.cpp Display.cpp \
	DisplayWindow.cpp Exception.cpp File.cpp FileCSV.cpp FileEXR.cpp \
	FileFLO.cpp FileHDR.cpp FileJPG.cpp FilePBA.cpp FilePNG.cpp \
	FilePPM.cpp FileTGA.cpp FileTIFF.cpp FileTMP.cpp FileWAV.cpp \
	Filter.cpp GaussTransform.cpp Geometry.cpp HDR.cpp Image.cpp \
	KernelEstimation.cpp LAHBPCG.cpp LaplacianFilter.cpp LightField.cpp \
	LocalLaplacian.cpp main.cpp Network.cpp NetworkOps.cpp Operation.cpp \
	OpticalFlow.cpp Paint.cpp Panorama.cpp Parser.cpp \
	PatchMatch.cpp Plugin.cpp Prediction.cpp Projection.cpp Stack.cpp \
	Statistics.cpp Wavelet.cpp WLS.cpp

LOCAL_MODULE      := imagestack
LOCAL_MODULE_FILENAME := libImageStack

LOCAL_STATIC_LIBRARIES := libjpeg libpng
LOCAL_EXPORT_STATIC_LIBRARIES += libjpeg libpng

LOCAL_SHARED_LIBRARIES := 
LOCAL_EXPORT_SHARED_LIBRARIES += 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include/ImageStack
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include

LOCAL_CFLAGS += -O3 -fno-strict-aliasing -fomit-frame-pointer -fexpensive-optimizations -funroll-loops -fprefetch-loop-arrays -DNO_SDL -DNO_FFTW -DNO_OPENEXR -DNO_TIFF

# This is taken care of by the master Android.mk.
# $(call import-module, fcam/external/libpng)
# $(call import-module, fcam/external/libjpeg)

include $(BUILD_SHARED_LIBRARY)
