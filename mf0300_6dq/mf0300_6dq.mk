# This is a FSL Android Reference Design platform based on i.MX6Q ARD board
# It will inherit from FSL core product which in turn inherit from Google generic

-include device/fsl/common/imx_path/ImxPathConfig.mk
$(call inherit-product, device/fsl/mf0300_6dq/imx6_mf0300.mk)
$(call inherit-product-if-exists,vendor/google/products/gms.mk)

# Overrides
PRODUCT_NAME := mf0300_6dq
PRODUCT_DEVICE := mf0300_6dq

PRODUCT_COPY_FILES += \
	device/fsl/mf0300_6dq/init.rc:root/init.freescale.rc \
	device/fsl/mf0300_6dq/init.imx6q.rc:root/init.freescale.imx6q.rc \
	device/fsl/mf0300_6dq/init.imx6dl.rc:root/init.freescale.imx6dl.rc \
	device/fsl/mf0300_6dq/init.imx6qp.rc:root/init.freescale.imx6qp.rc \

# Default target partition table is gpt
# else - mbr
BUILD_TARGET_PARTITION_TABLE ?= gpt

PRODUCT_COPY_FILES += \
    device/fsl/mf0300_6dq/fstab.freescale.$(BUILD_TARGET_PARTITION_TABLE):root/fstab.freescale \
    device/fsl/mf0300_6dq/ueventd.freescale.$(BUILD_TARGET_PARTITION_TABLE).rc:root/ueventd.freescale.rc \
    device/fsl/mf0300_6dq/init.freescale.sd.$(BUILD_TARGET_PARTITION_TABLE).rc:root/init.freescale.sd.rc \
    device/fsl/mf0300_6dq/init.freescale.emmc.$(BUILD_TARGET_PARTITION_TABLE).rc:root/init.freescale.emmc.rc

TARGET_RECOVERY_FSTAB = device/fsl/mf0300_6dq/fstab.freescale.$(BUILD_TARGET_PARTITION_TABLE)

ifeq ($(BUILD_TARGET_PARTITION_TABLE),gpt)
    # Default gpt
    BOARD_BPT_INPUT_FILES = device/fsl/mf0300_6dq/partition/device-partitions-14.6GB.bpt
    PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/by-name/system
    PRODUCT_PROPERTY_OVERRIDES += ro.frp.pst=/dev/block/by-name/presistdata
else
    BOARD_BPT_INPUT_FILES =
    PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/mmcblk3p5
    PRODUCT_PROPERTY_OVERRIDES += ro.frp.pst=/dev/block/mmcblk3p11
endif
# Force no additional partition table strategies
ADDITION_BPT_PARTITION =

# Audio
USE_XML_AUDIO_POLICY_CONF := 1
PRODUCT_COPY_FILES += \
	device/fsl/mf0300_6dq/audio_effects.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.conf \
	device/fsl/mf0300_6dq/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
	frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
	frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
	frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
	frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
	frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \

PRODUCT_COPY_FILES +=	\
	$(LINUX_FIRMWARE_IMX_PATH)/linux-firmware-imx/firmware/vpu/vpu_fw_imx6d.bin:system/lib/firmware/vpu/vpu_fw_imx6d.bin 	\
	$(LINUX_FIRMWARE_IMX_PATH)/linux-firmware-imx/firmware/vpu/vpu_fw_imx6q.bin:system/lib/firmware/vpu/vpu_fw_imx6q.bin
# setup dm-verity configs.
 $(call inherit-product, build/target/product/verity.mk)


# GPU files

DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_AAPT_CONFIG += xlarge large tvdpi hdpi xhdpi

PRODUCT_COPY_FILES += \
	device/fsl/mf0300_6dq/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.faketouch.xml:system/etc/permissions/android.hardware.faketouch.xml \
	frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
	frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml

PRODUCT_COPY_FILES += \
    $(FSL_PROPRIETARY_PATH)/fsl-proprietary/gpu-viv/lib/egl/egl.cfg:system/lib/egl/egl.cfg

# HWC2 HAL
PRODUCT_PACKAGES += \
    android.hardware.graphics.composer@2.1-impl

# Gralloc HAL
PRODUCT_PACKAGES += \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-impl \
    android.hardware.audio@2.0-service \
    android.hardware.audio.effect@2.0-impl \
    android.hardware.power@1.0-impl \
    android.hardware.power@1.0-service \
    android.hardware.light@2.0-impl \
    android.hardware.light@2.0-service

# Usb HAL
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service

# Bluetooth HAL
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl \
    android.hardware.bluetooth@1.0-service

# WiFi HAL
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    wifilogd \
    wificond

# Keymaster HAL
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl

PRODUCT_PACKAGES += \
    libEGL_VIVANTE \
    libGLESv1_CM_VIVANTE \
    libGLESv2_VIVANTE \
    libGAL \
    libGLSLC \
    libVSC \
    libg2d \
    libgpuhelper

PRODUCT_PACKAGES += \
    Launcher3

# Factory serial daemon
PRODUCT_PACKAGES += \
	sernd

# TeamViewer
PRODUCT_PACKAGES += \
	TeamViewerQS

PRODUCT_PROPERTY_OVERRIDES += \
    ro.internel.storage_size=/sys/block/bootdev_size
