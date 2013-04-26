LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LIBS := -lpthread

LOCAL_CFLAGS := -Wall

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

LOCAL_SRC_FILES := \
			test_main.c notifier.c

LOCAL_MODULE := test_main

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

