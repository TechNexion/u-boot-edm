/*
 * Copyright (C) 2015 Technexion Ltd.
 *
 * Configuration settings for the Technexion PICO-IMX6UL-EMMC board.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */
#ifndef __PICO_IMX6UL_CONFIG_H
#define __PICO_IMX6UL_CONFIG_H


#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/imx-common/gpio.h>

/* SPL support */
#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SERIAL_TAG
#define CONFIG_CMD_BOOTA

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART_BASE		MX6UL_UART6_BASE_ADDR

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR

#define CONFIG_SYS_FSL_USDHC_NUM	1

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */
#endif

#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define CONFIG_FEC_ENET_DEV		1

/* USB Configs */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#define CONFIG_USBD_HS

#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_USB_GADGET_VBUS_DRAW	2

#define CONFIG_SYS_DFU_DATA_BUF_SIZE	SZ_16M
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* Network support */
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET2_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x1
#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1         /* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2         /* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED            100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08

#define CONFIG_LOADADDR			0x80800000
#define CONFIG_SYS_TEXT_BASE		0x87800000
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* Framebuffer */
#define CONFIG_VIDEO_MXS
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"som=autodetect\0" \
	"image=zImage\0" \
	"ip_dyn=no\0" \
	"console=ttymxc5\0" \
	"splashpos=m,m\0" \
	"baseboard=pi\0" \
	"default_baseboard=pi\0" \
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"detectmem=" \
		"if test ${memdet} = 512MB; then " \
			"setenv memsize cma=128M; " \
		"else " \
			"setenv memsize cma=96M; " \
		"fi\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"${memsize} "\
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"setfdt=setenv fdtfile ${som}_${baseboard}.dtb\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run detectmem; " \
		"run mmcargs; " \
		"echo baseboard is ${baseboard}; " \
		"run setfdt; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"echo WARN: Cannot load the DT; " \
					"echo fall back to load the default DT; " \
					"setenv baseboard ${default_baseboard}; " \
					"run setfdt; " \
					"run loadfdt; " \
					"bootz ${loadaddr} - ${fdt_addr}; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t -r $loadaddr $filesize\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
	"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"netboot=echo Booting from net ...; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"run loadbootenv; " \
		"run importbootenv; " \
		"run setfdt; " \
		"run netargs; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdtfile}; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootenv; then " \
			   "echo Loaded environment from ${bootenv};" \
			   "run importbootenv;" \
		   "fi;" \
		   "if test -n $uenvcmd; then " \
			   "echo Running uenvcmd ...;" \
			   "run uenvcmd;" \
		   "fi;" \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "else run netboot; " \
			   "fi; " \
		   "fi; " \
	   "else run netboot; fi"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP

/* #define CONFIG_SYS_HUSH_PARSER */
#define CONFIG_AUTO_COMPLETE

/* Print Buffer Size */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 256 * SZ_1M)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_CMDLINE_EDITING

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_SIZE			SZ_8K
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(8 * SZ_64K)

#define CONFIG_SYS_MMC_ENV_DEV		0   /* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART		0	/* user area */
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* USDHC2 */

#define CONFIG_CMD_BMODE

#endif /* __PICO_IMX6UL_CONFIG_H */
