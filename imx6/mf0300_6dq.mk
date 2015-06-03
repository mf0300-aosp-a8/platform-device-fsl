# This is a FSL Android Reference Design platform based on i.MX6Q ARD board
# It will inherit from FSL core product which in turn inherit from Google generic

$(call inherit-product, device/fsl/imx6/imx6.mk)
$(call inherit-product-if-exists,vendor/google/products/gms.mk)

ifneq ($(wildcard device/fsl/mf0300_6dq/fstab_nand.freescale),)
$(shell touch device/fsl/mf0300_6dq/fstab_nand.freescale)
endif

ifneq ($(wildcard device/fsl/mf0300_6dq/fstab.freescale),)
$(shell touch device/fsl/mf0300_6dq/fstab.freescale)
endif

# Overrides
PRODUCT_NAME := mf0300_6dq
PRODUCT_DEVICE := mf0300_6dq

PRODUCT_COPY_FILES += \
	device/fsl/imx6/eeti/eGTouchD:system/bin/eGTouchD \
	device/fsl/imx6/eeti/eGTouchA.ini:system/etc/eGTouchA.ini \
	device/fsl/imx6/eeti/eGalaxTouch_VirtualDevice.idc:system/usr/idc/eGalaxTouch_VirtualDevice.idc \
	device/fsl/imx6/elo/elo_mt_android:system/bin/elo_mt_android \
	device/fsl/imx6/elo/EloMultiTouchScreen.idc:system/usr/idc/EloMultiTouchScreen.idc \
	device/fsl/mf0300_6dq/required_hardware.xml:system/etc/permissions/required_hardware.xml \
	device/fsl/mf0300_6dq/init.rc:root/init.freescale.rc \
	device/fsl/mf0300_6dq/audio_policy.conf:system/etc/audio_policy.conf \
	device/fsl/mf0300_6dq/audio_effects.conf:system/vendor/etc/audio_effects.conf

PRODUCT_COPY_FILES +=	\
	device/fsl/common/input/eGalax_Touch_Screen.idc:system/usr/idc/ilitek_i2c.idc \
	external/linux-firmware-imx/firmware/vpu/vpu_fw_imx6d.bin:system/lib/firmware/vpu/vpu_fw_imx6d.bin 	\
	external/linux-firmware-imx/firmware/vpu/vpu_fw_imx6q.bin:system/lib/firmware/vpu/vpu_fw_imx6q.bin

# GPU files

# Azurewave WiFi/BT files
PRODUCT_COPY_FILES +=   \
	device/fic-extend/3rd-party/azw-broadcom/firmware/fw_bcmdhd_apsta.bin:system/etc/firmware/fw_bcmdhd_apsta.bin \
      	device/fic-extend/3rd-party/azw-broadcom/firmware/fw_bcmdhd.bin:system/etc/firmware/fw_bcmdhd.bin \
      	device/fic-extend/3rd-party/azw-broadcom/firmware/fw_bcmdhd_p2p.bin:system/etc/firmware/fw_bcmdhd_p2p.bin \
      	device/fic-extend/3rd-party/azw-broadcom/firmware/BCM4330B1_002.001.003.0967.0973.hcd:system/etc/firmware/BCM4330B1_002.001.003.0967.0973.hcd \
      	device/fic-extend/3rd-party/azw-broadcom/nvram/bcmdhd.cal:system/etc/wifi/bcmdhd.cal \
	device/fic-extend/3rd-party/azw-broadcom/nvram/nvram.txt:system/etc/firmware/nvram.txt \
	hardware/broadcom/wlan/bcmdhd/config/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
	hardware/broadcom/wlan/bcmdhd/config/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_AAPT_CONFIG += xlarge large tvdpi hdpi

PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
	frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
	frameworks/native/data/etc/android.hardware.faketouch.xml:system/etc/permissions/android.hardware.faketouch.xml \
	frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
	frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

# for PDK build, include only when the dir exists
# too early to use $(TARGET_BUILD_PDK)
ifneq ($(wildcard packages/wallpapers/LivePicker),)
PRODUCT_COPY_FILES += \
	packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml
endif


PRODUCT_PACKAGES += sernd libficextjni checkfiles

