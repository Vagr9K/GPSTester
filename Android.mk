LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	gps_test.cpp

LOCAL_C_INCLUDES += $(ANDROID_SRCDIR)/hardware/libhardware/include
LOCAL_C_INCLUDES += $(ANDROID_SRCDIR)/system/core/include

LOCAL_LDFLAGS := -L $(SYSDIR)/system/lib
LOCAL_LDLIBS := -lhardware -lcutils

LOCAL_MODULE:= gps-test

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -O0

include $(BUILD_EXECUTABLE)
