LOCAL_PATH := $(my-dir)

##########################################################

include $(CLEAR_VARS)

LOCAL_MODULE := log-kernel
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).sh
LOCAL_MODULE_CLASS :=  EXECUTABLES

include $(BUILD_PREBUILT)

##########################################################

include $(CLEAR_VARS)

LOCAL_MODULE := log-logcat
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).sh
LOCAL_MODULE_CLASS :=  EXECUTABLES

include $(BUILD_PREBUILT)

##########################################################

include $(CLEAR_VARS)

LOCAL_MODULE := log-tools
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).sh
LOCAL_MODULE_CLASS :=  EXECUTABLES

include $(BUILD_PREBUILT)

##########################################################

include $(CLEAR_VARS)

LOCAL_MODULE := log-temp
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).sh
LOCAL_MODULE_CLASS :=  EXECUTABLES

include $(BUILD_PREBUILT)

##########################################################

