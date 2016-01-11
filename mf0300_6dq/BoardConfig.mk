#
# Product-specific compile-time definitions.
#

include device/fsl/imx6/soc/imx6dq.mk
include device/fsl/mf0300_6dq/build_id.mk
include device/fsl/imx6/BoardConfigCommon.mk
include device/fsl-proprietary/gpu-viv/fsl-gpu.mk
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

BOARD_KERNEL_CMDLINE := console=ttymxc3,115200 init=/init rootfstype=ext4 video=mxcfb0:dev=ldb,bpp=32 video=mxcfb1:dev=hdmi,if=RGB24,bpp=32 vmalloc=400M androidboot.console=ttymxc3 consoleblank=0 androidboot.hardware=freescale androidboot.selinux=disabled cma=384M 
# BOARD_KERNEL_CMDLINE := console=ttymxc3,115200 init=/init vmalloc=400M androidboot.console=ttymxc3 consoleblank=0 androidboot.hardware=freescale cma=384M

TARGET_BOARD_DTS_CONFIG := imx6q-mf0300-xga.dtb
TARGET_BOOTLOADER_CONFIG := mx6qmf0300_xga_defconfig

BOARD_SEPOLICY_DIRS := \
       device/fsl/imx6/sepolicy

