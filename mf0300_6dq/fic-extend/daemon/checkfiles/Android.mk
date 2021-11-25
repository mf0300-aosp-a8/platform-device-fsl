LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= checkfiles.cpp
# LOCAL_CFLAGS := -I$(LOCAL_PATH)/../rdv_interface -I$(TOPDIR)external/astl/include
LOCAL_SHARED_LIBRARIES := libcutils libstlport 
LOCAL_MODULE := checkfiles
include $(BUILD_EXECUTABLE)

# include $(CLEAR_VARS)
# LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
# LOCAL_MODULE_TAGS := optional
# LOCAL_SRC_FILES:= test.cpp
# LOCAL_C_INCLUDES := $(KERNEL_HEADERS) $(TOPDIR)external/astl/include
# # LOCAL_CFLAGS := -I$(LOCAL_PATH)/../rdv_interface -I$(TOPDIR)external/astl/include
# LOCAL_SHARED_LIBRARIES := libcutils 
# LOCAL_STATIC_LIBRARIES := libastl
# LOCAL_MODULE := test_sern
# include $(BUILD_EXECUTABLE)
