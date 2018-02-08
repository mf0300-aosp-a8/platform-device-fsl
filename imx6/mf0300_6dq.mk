# This is a FSL Android Reference Design platform based on i.MX6Q ARD board
# It will inherit from FSL core product which in turn inherit from Google generic

$(call inherit-product, device/fsl/imx6/imx6.mk)
$(call inherit-product-if-exists,vendor/google/products/gms.mk)

ifneq ($(wildcard device/fsl/mf0300_6dq/fstab.freescale),)
$(shell touch device/fsl/mf0300_6dq/fstab.freescale)
endif

# Overrides
PRODUCT_NAME := mf0300_6dq
PRODUCT_DEVICE := mf0300_6dq

PRODUCT_COPY_FILES += \
	device/fsl/mf0300_6dq/init.rc:root/init.freescale.rc \
        device/fsl/mf0300_6dq/init.i.MX6Q.rc:root/init.freescale.i.MX6Q.rc \
        device/fsl/mf0300_6dq/init.i.MX6DL.rc:root/init.freescale.i.MX6DL.rc \
	device/fsl/mf0300_6dq/init.i.MX6QP.rc:root/init.freescale.i.MX6QP.rc \
	device/fsl/mf0300_6dq/audio_policy.conf:system/etc/audio_policy.conf \
	device/fsl/mf0300_6dq/audio_effects.conf:system/vendor/etc/audio_effects.conf

PRODUCT_COPY_FILES +=	\
	external/linux-firmware-imx/firmware/vpu/vpu_fw_imx6d.bin:system/lib/firmware/vpu/vpu_fw_imx6d.bin 	\
	external/linux-firmware-imx/firmware/vpu/vpu_fw_imx6q.bin:system/lib/firmware/vpu/vpu_fw_imx6q.bin
# setup dm-verity configs.
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/mmcblk3p5
$(call inherit-product, build/target/product/verity.mk)

# GPU files

# mod 4.4
# AMPAK BROADCOM BCM4330 firmware and configuration files.
PRODUCT_COPY_FILES += \
	device/fsl/mf0300_6dq/broadcom/fw_bcm4330b2_ag.bin:system/etc/firmware/fw_bcmdhd.bin \
	device/fsl/mf0300_6dq/broadcom/fw_bcm4330b2_ag_p2p.bin:system/etc/firmware/fw_bcmdhd_p2p.bin \
	device/fsl/mf0300_6dq/broadcom/fw_bcm4330b2_ag_apsta.bin:system/etc/firmware/fw_bcmdhd_apsta.bin \
	device/fsl/mf0300_6dq/broadcom/nvram_gb86302i.txt:system/etc/firmware/nvram.txt \
	device/fsl/mf0300_6dq/broadcom/bcm4330b2.hcd:system/etc/firmware/bcm4330b2.hcd \
	hardware/broadcom/wlan/bcmdhd/config/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
	hardware/broadcom/wlan/bcmdhd/config/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf
# mod 4.4 end
# touch service for Echo POS WXGA (1366x768)
PRODUCT_COPY_FILES += \
	device/fsl/imx6/eeti/eGTouchD:system/bin/egtouchd \
	device/fsl/imx6/eeti/eGTouchA.ini:data/eGTouchA.ini \
	device/fsl/imx6/eeti/eGalaxTouch_VirtualDevice.idc:system/usr/idc/eGalaxTouch_VirtualDevice.idc

DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_AAPT_CONFIG += xlarge large tvdpi hdpi xhdpi

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
	frameworks/native/data/etc/android.hardware.faketouch.xml:system/etc/permissions/android.hardware.faketouch.xml \
	frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
	frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
	device/fsl/mf0300_6dq/required_hardware.xml:system/etc/permissions/required_hardware.xml

PRODUCT_PACKAGES += \
	eSystemInfo \
	SpeedTest \
	GPUTest \
	sernd \
	libficextjni \
	test_sern \
	uart-test \
	checkfiles \
	log-kernel \
	log-logcat \
	log-tools \
	log-temp \
	eGalaxCalibrator

#PRODUCT_PACKAGES += \
#	AnTuTu \
#	EchoPro \
#	AudioRoute \
#	eHardwareInfo \
#	eHardwareInfoV2 \
#	eSystemInfo
#	eGalaxCalibrator-1024
