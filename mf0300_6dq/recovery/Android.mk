LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
    bootable/recovery \
    bootable/recovery/bootloader_message/include \
    bootable/recovery/otafault \
    bootable/recovery/updater/include \
    system/core/base/include

LOCAL_SRC_FILES := recovery_updater.cpp \
    utils/gpt_utils.c \
    utils/loop_utils.c

LOCAL_MODULE := librecovery_updater_mf0300_6dq

include $(BUILD_STATIC_LIBRARY)
