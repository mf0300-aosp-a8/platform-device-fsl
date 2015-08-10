#
# Product-specific compile-time definitions.
#

include device/fsl/imx6/soc/imx6dq.mk
include device/fsl/mf0300_6dq/build_id.mk
include device/fsl/imx6/BoardConfigCommon.mk
include device/fsl-proprietary/gpu-viv/fsl-gpu.mk
#BOARD_FOR_FEC := true
##################################################################3
PLATFORM_SKULL ?= std

ifeq ($(PLATFORM_SKULL),std)

PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/init.rc:root/init.freescale.rc 
DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay


endif

# FEC 1366x768 HT vendor 
ifeq ($(PLATFORM_SKULL),fec_1)
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/init.fec_1.rc:root/init.freescale.rc 
DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay_fec


endif

# FEC 1366x768 and 1024x768 XGA
ifeq ($(PLATFORM_SKULL),fec_2)
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/init.fec_2.rc:root/init.freescale.rc 
DEVICE_PACKAGE_OVERLAYS := device/fsl/mf0300_6dq/overlay


endif

BOARD_FOR_FEC := true
#################################################################################3

# mf0300_6dq default target for EXT4
BUILD_TARGET_FS ?= ext4
include device/fsl/imx6/imx6_target_fs.mk

ifeq ($(BUILD_TARGET_FS),ubifs)
TARGET_RECOVERY_FSTAB = device/fsl/mf0300_6dq/fstab_nand.freescale
# build ubifs for nand devices
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/fstab_nand.freescale:root/fstab.freescale
else
ifeq ($(BUILD_TARGET_FS),sd)
TARGET_RECOVERY_FSTAB = device/fsl/mf0300_6dq/fstab_sd.freescale
# build for sd card
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/fstab_sd.freescale:root/fstab.freescale
else
TARGET_RECOVERY_FSTAB = device/fsl/mf0300_6dq/fstab.freescale
# build for ext4
PRODUCT_COPY_FILES +=	\
	device/fsl/mf0300_6dq/fstab.freescale:root/fstab.freescale
endif
endif # BUILD_TARGET_FS


TARGET_BOOTLOADER_BOARD_NAME := MF0300
PRODUCT_MODEL := MF0300-MX6DQ

#
# Wifi
#
BOARD_WLAN_VENDOR 			 := BROADCOM
# for atheros vendor
ifeq ($(BOARD_WLAN_VENDOR),ATHEROS)
	BOARD_WLAN_DEVICE			 := ar6003
	BOARD_HAS_ATH_WLAN 			 := true
	BOARD_WLAN_ATHEROS_SDK			 := system/wlan/atheros/compat-wireless
	WPA_SUPPLICANT_VERSION			 := VER_0_9_ATHEROS
	HOSTAPD_VERSION				 := VER_0_9_ATHEROS
	WIFI_DRIVER_MODULE_PATH          	 := "/system/lib/modules/ath6kl_sdio.ko"
	WIFI_DRIVER_MODULE_NAME          	 := "ath6kl_sdio"
	WIFI_DRIVER_MODULE_ARG           	 := "suspend_mode=3 ath6kl_p2p=1"
	WIFI_DRIVER_P2P_MODULE_ARG       	 := "suspend_mode=3 ath6kl_p2p=1 debug_mask=0x2413"
	WIFI_SDIO_IF_DRIVER_MODULE_PATH  	 := "/system/lib/modules/cfg80211.ko"
	WIFI_SDIO_IF_DRIVER_MODULE_NAME  	 := "cfg80211"
	WIFI_SDIO_IF_DRIVER_MODULE_ARG   	 := ""
	WIFI_COMPAT_MODULE_PATH			 := "/system/lib/modules/compat.ko"
	WIFI_COMPAT_MODULE_NAME			 := "compat"
	WIFI_COMPAT_MODULE_ARG			 := ""
endif
#for intel vendor
ifeq ($(BOARD_WLAN_VENDOR),INTEL)
	BOARD_HOSTAPD_PRIVATE_LIB		 ?= private_lib_driver_cmd
	BOARD_WPA_SUPPLICANT_PRIVATE_LIB 	 ?= private_lib_driver_cmd
	WPA_SUPPLICANT_VERSION			 := VER_0_7_X_INTEL
	HOSTAPD_VERSION				 := VER_0_7_X_INTEL
	WIFI_DRIVER_MODULE_PATH          	 := "/system/lib/modules/iwlagn.ko"
	WIFI_DRIVER_MODULE_NAME          	 := "iwlagn"
	WIFI_DRIVER_MODULE_PATH			 ?= auto
endif
#for broadcom
ifeq ($(BOARD_WLAN_VENDOR),BROADCOM)
	WIFI_DRIVER_FW_PATH_STA			 := "/system/etc/firmware/fw_bcmdhd.bin"
	WIFI_DRIVER_FW_PATH_AP			 := "/system/etc/firmware/fw_bcmdhd_apsta.bin"
	WIFI_DRIVER_FW_PATH_P2P			 := "/system/etc/firmware/fw_bcmdhd_p2p.bin"
	WIFI_DRIVER_FW_PATH_PARAM		 := "/sys/module/bcmdhd/parameters/firmware_path"
	BOARD_WLAN_DEVICE			 := bcmdhd
	BOARD_WPA_SUPPLICANT_PRIVATE_LIB	 := lib_driver_cmd_bcmdhd
	BOARD_HOSTAPD_PRIVATE_LIB		 := lib_driver_cmd_bcmdhd

	WIFI_DRIVER_MODULE_NAME 		 := "bcmdhd"
	WIFI_DRIVER_MODULE_ARG			 := "iface_name=wlan firmware_path=/system/etc/firmware/fw_bcmdhd.bin nvram_path=/system/etc/firmware/nvram.txt"

endif

WPA_SUPPLICANT_VERSION			 := VER_0_8_X
HOSTAPD_VERSION				 := VER_0_8_X
BOARD_HAVE_BLUETOOTH_BCM		 := true

BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_HOSTAPD_DRIVER             	 := NL80211
WIFI_TEST_INTERFACE			 := "sta"

BOARD_MODEM_VENDOR := AMAZON

USE_ATHR_GPS_HARDWARE := false
USE_QEMU_GPS_HARDWARE := false

#for accelerator sensor, need to define sensor type here
BOARD_HAS_SENSOR := false
SENSOR_MMA8451 := false
CONFIG_FIC_SENSOR_WITHOUT_GYRO := false
CONFIG_FIC_ROOT := true

# for recovery service
TARGET_SELECT_KEY := 28

# we don't support sparse image.
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true

# uncomment below lins if use NAND
#TARGET_USERIMAGES_USE_UBIFS = true


ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
UBI_ROOT_INI := device/fsl/mf0300_6dq/ubi/ubinize.ini
TARGET_MKUBIFS_ARGS := -m 4096 -e 516096 -c 4096 -x none
TARGET_UBIRAW_ARGS := -m 4096 -p 512KiB $(UBI_ROOT_INI)
endif

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
ifeq ($(TARGET_USERIMAGES_USE_EXT4),true)
$(error "TARGET_USERIMAGES_USE_UBIFS and TARGET_USERIMAGES_USE_EXT4 config open in same time, please only choose one target file system image")
endif
endif

BOARD_KERNEL_CMDLINE := console=ttymxc3,115200 init=/init video=mxcfb0:dev=ldb,bpp=32 video=mxcfb1:dev=hdmi,if=RGB24,bpp=32 video=mxcfb2:off video=mxcfb3:off vmalloc=400M androidboot.console=ttymxc3 consoleblank=0 androidboot.hardware=freescale cma=384M

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
#UBI boot command line.
# Note: this NAND partition table must align with MFGTool's config.
BOARD_KERNEL_CMDLINE +=  mtdparts=gpmi-nand:16m(bootloader),16m(bootimg),128m(recovery),-(root) gpmi_debug_init ubi.mtd=3
endif

# atheros 3k BT
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/fsl/mf0300_6dq/bluetooth

USE_ION_ALLOCATOR := false
USE_GPU_ALLOCATOR := true

# camera hal v2
IMX_CAMERA_HAL_V2 := true

# define frame buffer count
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3

#define consumer IR HAL support
IMX6_CONSUMER_IR_HAL := false

TARGET_KERNEL_DEFCONF := imx_v7_mf0300_android_defconfig
TARGET_BOOTLOADER_CONFIG := imx6q:mx6qmf0300android_config imx6q-xga:mx6qmf0300android-xga_config
TARGET_BOARD_DTS_CONFIG := imx6q-mf0300:imx6q-mf0300.dtb imx6q-mf0300-xga:imx6q-mf0300-xga.dtb 

BOARD_SEPOLICY_DIRS := \
       device/fsl/mf0300_6dq/sepolicy

BOARD_SEPOLICY_UNION := \
       app.te \
       file_contexts \
       fs_use \
       untrusted_app.te \
       genfs_contexts


