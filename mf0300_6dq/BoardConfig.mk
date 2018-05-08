#
# Product-specific compile-time definitions.
#

include device/fsl/imx6/soc/imx6dq.mk
include device/fsl/mf0300_6dq/build_id.mk
include device/fsl/imx6/BoardConfigCommon.mk
include device/fsl-proprietary/gpu-viv/fsl-gpu.mk
include external/boringssl/flavor.mk
# default target for EXT4
BUILD_TARGET_FS ?= ext4
include device/fsl/imx6/imx6_target_fs.mk

TARGET_RECOVERY_FSTAB = device/fsl/mf0300_6dq/fstab.freescale
# build for ext4
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/fstab.freescale:root/fstab.freescale

DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay

TARGET_BOOTLOADER_BOARD_NAME := MF0300

BOARD_SOC_CLASS := IMX6
BOARD_SOC_TYPE := IMX6DQ
PRODUCT_MODEL := MF0300-MX6DQ

USE_ION_ALLOCATOR := false
USE_GPU_ALLOCATOR := true

WITH_DEXPREOPT := true

USE_OPENGL_RENDERER := true
TARGET_CPU_SMP := true
DM_VERITY_RUNTIME_CONFIG := true

TARGET_RELEASETOOLS_EXTENSIONS := device/fsl/imx6

USE_ION_ALLOCATOR := false
USE_GPU_ALLOCATOR := true

TARGET_BOARD_DTS_CONFIG := \
	imx6q-mf0300-wxga:imx6q-mf0300-wxga.dtb \
	imx6q-mf0300-xga:imx6q-mf0300-xga.dtb

TARGET_BOOTLOADER_CONFIG := \
	imx6q-mf0300-wxga:mx6qmf0300_wxga_defconfig \
	imx6q-mf0300-xga:mx6qmf0300_xga_defconfig \
	imx6q-mf0300-mfg:mx6qmf0300_mfg_defconfig \
	imx6q-mf0300-sdcardflasher:mx6qmf0300_sdcardflasher_defconfig

BOARD_SEPOLICY_DIRS := \
       device/fsl/imx6/sepolicy

# mod 4.4
# Wifi
#
BOARD_WLAN_VENDOR := BROADCOM
#for broadcom
ifeq ($(BOARD_WLAN_VENDOR),BROADCOM)
	WIFI_DRIVER_FW_PATH_STA          := "/system/etc/firmware/fw_bcmdhd.bin"
	WIFI_DRIVER_FW_PATH_AP           := "/system/etc/firmware/fw_bcmdhd_apsta.bin"
	WIFI_DRIVER_FW_PATH_P2P          := "/system/etc/firmware/fw_bcmdhd_p2p.bin"
	WIFI_DRIVER_FW_PATH_PARAM        := "/sys/module/bcmdhd/parameters/firmware_path"
	BOARD_WLAN_DEVICE                := UNITE
	BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
	BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_bcmdhd
	WIFI_DRIVER_MODULE_NAME          := "bcmdhd"
	WIFI_DRIVER_MODULE_ARG           := "iface_name=wlan firmware_path=/system/etc/firmware/fw_bcmdhd.bin nvram_path=/system/etc/firmware/nvram.txt"
endif

WPA_SUPPLICANT_VERSION      := VER_0_8_UNITE
HOSTAPD_VERSION             := VER_0_8_X
BOARD_HAVE_BLUETOOTH_BCM    := true

BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER        := NL80211
WIFI_TEST_INTERFACE         := "sta"

BOARD_MODEM_VENDOR          := AMAZON

USE_ATHR_GPS_HARDWARE       := false
USE_QEMU_GPS_HARDWARE       := false

# force copy dhcpcd config
PRODUCT_COPY_FILES +=	\
	hardware/broadcom/wlan/bcmdhd/config/android_dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf

# atheros 3k BT
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/fsl/mf0300_6dq/bluetooth
# mod 4.4 end
