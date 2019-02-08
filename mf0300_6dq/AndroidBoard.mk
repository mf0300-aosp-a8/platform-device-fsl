LOCAL_PATH := $(call my-dir)

$(call add-radio-file,e2fsprogs/e2fsck)
$(call add-radio-file,e2fsprogs/e2image)
$(call add-radio-file,e2fsprogs/resize2fs)

include $(FSL_PROPRIETARY_PATH)/fsl-proprietary/media-profile/media-profile.mk
