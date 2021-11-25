ifeq ($(ETHERNET_LED_IRQ), true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= enetmond.cpp
LOCAL_SHARED_LIBRARIES := libcutils libstlport libhardware_legacy libnetutils
LOCAL_MODULE := enetmond
include $(BUILD_EXECUTABLE)

endif
