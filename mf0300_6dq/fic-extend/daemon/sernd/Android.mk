LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= sernd.cpp

ifeq ($(BOARD_FOR_FEC),true)
    LOCAL_CPPFLAGS += -DMAC_AND_SERIAL_CHECK
endif

# LOCAL_CFLAGS := -I$(LOCAL_PATH)/../rdv_interface -I$(TOPDIR)external/astl/include
# LOCAL_SHARED_LIBRARIES := libcutils libstlport 
LOCAL_SHARED_LIBRARIES := libcutils libstlport libhardware_legacy libnetutils
LOCAL_MODULE := sernd
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= test.cpp
LOCAL_C_INCLUDES := $(KERNEL_HEADERS) $(TOPDIR)external/astl/include
LOCAL_CFLAGS := -I$(LOCAL_PATH)/../rdv_interface -I$(TOPDIR)external/astl/include
LOCAL_SHARED_LIBRARIES := libcutils
#LOCAL_STATIC_LIBRARIES := libastl
LOCAL_MODULE := test_sern
include $(BUILD_EXECUTABLE)
