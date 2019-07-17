/*
 * Copyright (C) 2018 TechNexion Ltd.
 *
 * Author: Tapani Utriainen <tapani@technexion.com>
 *         Richard Hu <richard.hu@technexion.com>
 *         Alvin Chen <alvin.chen@technexion.com>
 *         Po Cheng <po.cheng@technexion.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PICO_IMX7D_CONFIG_H
#define __PICO_IMX7D_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <asm/mach-imx/gpio.h>
#include "mx7_common.h"

#include "imx7_spl.h"

/* SPL OS boot options - Falcon Mode */
#ifdef CONFIG_SPL_OS_BOOT
/* NOTE: we all work in sectors, i.e. a sector = 512 Bytes */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME	"fdtargs"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME	"uImage"
#define CONFIG_SYS_SPL_ARGS_ADDR	0x83000000 /* (CONFIG_SYS_SDRAM_BASE + (128 << 19)) i.e. 0x80000000 + 0x4000000 = 84000000 */
#define CONFIG_CMD_SPL_WRITE_SIZE		(128 * SZ_1K)

/* #define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x500000  i.e. nand: sector 5242880, at 2,560 MB location */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR  2931712  /* CONFIG_ENV_OFFSET @ 64 * SZ_64K, CONFIG_ENV_SIZE = 0x1000 = 4K, 1st emmc partition starts at 16384 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS 133      /* 133 * 512 = 68KB, e.g. (CONFIG_CMD_SPL_WRITE_SIZE / 512) */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR        3000000  /* emmc: sector 3000000, at 1,536 MB location */
#endif

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

#define CONFIG_MXC_UART_BASE		UART5_IPS_BASE_ADDR

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* FUSE command to burn fuse */
#ifdef CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP
#endif

/* Network */
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_ENET_DEV 0

/* ENET1 */
#if (CONFIG_FEC_ENET_DEV == 0)
#define IMX_FEC_BASE			ENET_IPS_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x0
#elif (CONFIG_FEC_ENET_DEV == 1)
#define IMX_FEC_BASE			ENET2_IPS_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x1
#endif
#define CONFIG_FEC_MXC_MDIO_BASE	ENET_IPS_BASE_ADDR

/* MMC Config */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */

/* Command definition */

#define CONFIG_LOADADDR			0x80800000
#define CONFIG_SYS_TEXT_BASE		0x87800000

#define CONFIG_SYS_AUXCORE_BOOTDATA 0x60000000 /* Set to QSPI1 A flash at default */

#define SYS_AUXCORE_BOOTDATA_DDR	0x80000000
#define SYS_AUXCORE_BOOTDATA_TCM	0x007f8000

#ifdef CONFIG_IMX_BOOTAUX
#define UPDATE_M4_ENV \
	"m4image=m4_tcm.bin\0" \
	"m4_addr="__stringify(SYS_AUXCORE_BOOTDATA_TCM)"\0"
#else
#define UPDATE_M4_ENV ""
#endif

#define CONFIG_SYS_MMC_IMG_LOAD_PART	1
#ifdef CONFIG_SYS_BOOT_NAND
#define MFG_NAND_PARTITION "mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),-(rootfs) "
#else
#define MFG_NAND_PARTITION ""
#endif

#define CONFIG_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF " \
		"g_mass_storage.iSerialNumber=\"\" " \
		MFG_NAND_PARTITION "\0" \
		"clk_ignore_unused\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffff\0" \
	"bootcmd_mfg=run mfgtool_args;bootz ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \

#if defined(CONFIG_SYS_BOOT_NAND)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc2\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_high=0xffffffff\0"	  \
	"bootargs=console=ttymxc2,115200 ubi.mtd=3 "  \
		"root=ubi0:rootfs rootfstype=ubifs " \
		MFG_NAND_PARTITION "\0" \
	"bootcmd=nand read ${loadaddr} 0x4000000 0x800000;" \
		"nand read ${fdt_addr} 0x5000000 0x100000;" \
		"bootz ${loadaddr} - ${fdt_addr}\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	UPDATE_M4_ENV \
	"console=ttymxc4,"__stringify(CONFIG_BAUDRATE)"\0" \
	"splashpos=m,m\0" \
	"ip_dyn=yes\0" \
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_addr_R=0x83000000\0" \
	"ramdisk_addr_r=0x83000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"initrd_high=0xffffffff\0" \
	"initrd_addr=0x83800000\0" \
	"bootenv=uEnv.txt\0" \
	"fit_addr=0x87880000\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"=> "

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x20000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3
#define CONFIG_SYS_I2C_MXC_I2C4
#define CONFIG_SYS_I2C_SPEED		100000

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08

#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_MXS
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#endif

#ifdef CONFIG_SYS_BOOT_QSPI
#define CONFIG_SYS_USE_QSPI
#define CONFIG_ENV_IS_IN_SPI_FLASH
#elif defined CONFIG_SYS_BOOT_NAND
#define CONFIG_SYS_USE_NAND
#define CONFIG_ENV_IS_IN_NAND
#endif

/* NAND stuff */
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS

#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8
#endif

#ifdef CONFIG_SYS_USE_QSPI
#define CONFIG_FSL_QSPI    /* enable the QUADSPI driver */
#define CONFIG_QSPI_BASE		QSPI1_IPS_BASE_ADDR
#define CONFIG_QSPI_MEMMAP_BASE		QSPI0_ARB_BASE_ADDR

#define CONFIG_CMD_SF
#define	CONFIG_SPI_FLASH
#define	CONFIG_SPI_FLASH_MACRONIX
#define	CONFIG_SPI_FLASH_BAR
#define	CONFIG_SF_DEFAULT_BUS		0
#define	CONFIG_SF_DEFAULT_CS		0
#define	CONFIG_SF_DEFAULT_SPEED		40000000
#define	CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_ENV_OFFSET		(8 * SZ_64K) /* at 512KB (sector 1024) */
#define CONFIG_ENV_SIZE 		0x3000 /* 12,288 bytes (24 sectors) */
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SIZE 		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#elif defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_ENV_OFFSET		(3 << 18)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#endif

#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_SYS_FSL_USDHC_NUM	1
#else
#define CONFIG_SYS_FSL_USDHC_NUM	2
#endif
#define CONFIG_SYS_MMC_ENV_DEV		0			/* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART		0			/* user area */
#define CONFIG_MMCROOT			"/dev/mmcblk2p2"	/* USDHC1 */

/*
 * FDT options
 */
#define CONFIG_LMB

/*
 * SPLASH SCREEN Configs
 */
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_MXC_EPDC)
/*
 * Framebuffer and LCD
 */
#define	CONFIG_CFB_CONSOLE
#define CONFIG_CMD_BMP
#define CONFIG_LCD
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#undef LCD_TEST_PATTERN
/* #define CONFIG_SPLASH_IS_IN_MMC			1 */
#define LCD_BPP					LCD_MONOCHROME
/* #define CONFIG_SPLASH_SCREEN_ALIGN		1 */

#define CONFIG_WAVEFORM_BUF_SIZE		0x400000
#endif

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC			(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS			0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#if defined(CONFIG_MXC_EPDC) && defined(CONFIG_SYS_USE_QSPI)
#error "EPDC Pins conflicts QSPI, Either EPDC or QSPI can be enabled!"
#endif

#define CONFIG_OF_SYSTEM_SETUP

#if defined(CONFIG_ANDROID_SUPPORT)
#include "pico-imx7d_android.h"
#endif

#endif	/* __PICO_IMX7D_CONFIG_H */
