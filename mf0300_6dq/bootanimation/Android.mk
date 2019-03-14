LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := bootanimation.zip
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/media

BOOTANIMATION_ZIP := $(local-generated-sources-dir)/bootanimation.zip
BOOTANIMATION_SRC := $(LOCAL_PATH)

$(BOOTANIMATION_ZIP):
	zip_out=$$(realpath --relative-to=$(BOOTANIMATION_SRC) $(BOOTANIMATION_ZIP)) && \
	cd $(BOOTANIMATION_SRC) && zip -0qry -i \*.txt \*.png \*.wav @ $$zip_out *.txt part*

LOCAL_PREBUILT_MODULE_FILE := $(BOOTANIMATION_ZIP)
include $(BUILD_PREBUILT)
