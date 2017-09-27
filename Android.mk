LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libOpenCL
LOCAL_SRC_FILES := opencl_stubs.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/opencl_stubs_include
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -fPIE

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.cc pmu_utils.cc erode.cc erode_cl.cc erode_gl.cc erode_rs.cc erode3x3.rs
ifeq ($(TARGET_ARCH), $(filter $(TARGET_ARCH), arm arm64))
LOCAL_ARM_NEON := true
LOCAL_SRC_FILES += erode_neon.cc
endif
LOCAL_MODULE := erode_test
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/opencl_stubs_include
LOCAL_CPPFLAGS := -std=gnu++0x -Wall -fPIE -fopenmp -DWITH_OPENGLES -DWITH_OPENMP
LOCAL_CPP_FEATURES := exceptions
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -fPIE -pie -lGLESv2 -lEGL -fopenmp
LOCAL_SHARED_LIBRARIES := libOpenCL

# For Renderscript
LOCAL_STATIC_LIBRARIES += RScpp_static

include $(BUILD_EXECUTABLE)
$(call import-module,android/renderscript)
