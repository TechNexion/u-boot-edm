/*
 * Copyright (C) 2013 TechNexion Ltd.
 *
 * Author: Richard Hu <linuxfae@technexion.com>
 *
 * Configuration settings for the TechNexion EDM-CF-IMX6 SOM
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>
#include <asm/sizes.h>

#define CONFIG_MX6
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define MACH_TYPE_EDM_CF_IMX6		4257
#define CONFIG_MACH_TYPE		MACH_TYPE_EDM_CF_IMX6

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* SPL */
#ifdef CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define	CONFIG_SPL_LDSCRIPT	"arch/arm/cpu/armv7/mx6/u-boot-spl.lds"
#define CONFIG_SPL_TEXT_BASE          0x00908000 
#define CONFIG_SPL_MAX_SIZE          (68 * 1024)
#define CONFIG_SPL_START_S_PATH     "arch/arm/cpu/armv7"
#define CONFIG_SPL_STACK	0x0091FFB8

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SPL_GPIO_SUPPORT


#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION     1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME     "boot/u-boot.img"
#define CONFIG_SPL_LIBDISK_SUPPORT

#define CONFIG_SPL_SATA_SUPPORT

#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	138 /* offset 69KB */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	800 /* 400 KB */
#define CONFIG_SYS_MONITOR_LEN	(CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS/2*1024)

#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SPL_BSS_START_ADDR	0x18200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000	/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START     0x18300000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x3200000  /* 50 MB */
#define CONFIG_SYS_TEXT_BASE          0x17800000
#endif



/* NAND SPL */

#define CONFIG_SPL_DMA_SUPPORT

#define CONFIG_SPL_NAND_SUPPORT
#ifdef CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SYS_NAND_U_BOOT_OFFS 0x200000 /* offset 2MB */
/*  offset 64KB */ 
#define CONFIG_SYS_NAND_PAGE_SIZE     2048
#define CONFIG_SYS_NAND_OOBSIZE          64
#define CONFIG_SYS_NAND_BLOCK_SIZE (64*2048)
#define CONFIG_SYS_NAND_PAGE_COUNT     64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS     0
#endif


/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_SETEXPR

#define CONFIG_BOOTDELAY		3

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 500 * SZ_1M)
#define CONFIG_LOADADDR			0x12000000
#define CONFIG_SYS_TEXT_BASE		0x17800000

/* MMC Configuration */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_SATA

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif

/* Ethernet Configuration */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		1
#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS

/* Framebuffer */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_IPUV3
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (1 << 20)
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_IPUV3_CLK 260000000
#define CONFIG_IMX_HDMI


/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT	       "=> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		384

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	       32
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING

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

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

/* Enable NAND support */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_CMD_TIME


/* NAND stuff */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8
#endif /* CONFIG_CMD_NAND */


#ifdef CONFIG_ENV_IS_IN_BOOT_DEVICE
#define CONFIG_ENV_NAND_OFFSET		0x280000 /*2M + 512k*/
#define CONFIG_ENV_NAND_SECT_SIZE		(128 * 1024) /*128k*/
#define CONFIG_ENV_NAND_SIZE			CONFIG_ENV_NAND_SECT_SIZE
#define CONFIG_ENV_SIZE			(8 << 10)
#define CONFIG_ENV_OFFSET		(512 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SATA_ENV_DEV     	0
#endif

#define MTDIDS_DEFAULT		"nand0=gpmi-nand"
#define MTDPARTS_DEFAULT	"mtdparts=gpmi-nand:2m(spl-uboot),512k(uboot)," \
				"128k(env),512k(logo),4m(kernel),-(rootfs)"

#define CONFIG_EXTRA_ENV_SETTINGS					\
		"mtdparts=" MTDPARTS_DEFAULT "\0" \
		"videomode=video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24\0"	\
		"bootargs_base=setenv bootargs console=ttymxc0,115200\0"	\
		"bootargs_mmc=setenv bootargs ${bootargs} ${rootdevice} ${videomode}\0"	\
		"bootargs_nand=setenv bootargs ${bootargs} ubi.mtd=5 root=ubi0:rootfs rootfstype=ubifs rootwait rw ${mtdparts} ${videomode}\0"	\
		"bootargs_sata=setenv bootargs ${bootargs} ${rootdevice} ${videomode}\0"	\
		"mmc_rootdevice=root=/dev/mmcblk0p1 rootwait rw\0"	\
		"fat_rootdevice=root=/dev/mmcblk0p2 rootwait rw\0"	\
		"sata_rootdevice=root=/dev/sda1 rootwait rw\0"	\
		"rootdevice=\0"	\
		"android_hdmi_bootargs=console=ttymxc0,115200 "		\
			"root=/dev/mmcblk0p1 rootwait rw init=/init "		\
			"video=mxcfb0:dev=hdmi,1280x720M@60,if=RGB24,bpp=32 " \
			"video=mxcfb1:off video=mxcfb2:off fbmem=28M "		\
			"vmalloc=400M androidboot.console=ttymxc0 "		\
			"androidboot.hardware=fairy\0"			\
		"android_lvds_bootargs=console=ttymxc0,115200 "		\
			"root=/dev/mmcblk0p1 rootwait rw init=/init "		\
			"video=mxcfb0:dev=ldb,LDB-WSVGA,if=RGB24 " \
			"video=mxcfb1:off video=mxcfb2:off fbmem=28M "		\
			"vmalloc=400M androidboot.console=ttymxc0 "		\
			"androidboot.hardware=fairy\0"			\
		"android_lcd_bootargs=console=ttymxc0,115200 "		\
			"root=/dev/mmcblk0p1 rootwait rw init=/init "		\
			"video=mxcfb0:dev=lcd,SEIKO-WVGA,if=RGB666,bpp=32 " \
			"video=mxcfb1:off video=mxcfb2:off fbmem=28M "		\
			"vmalloc=400M androidboot.console=ttymxc0 "		\
			"androidboot.hardware=fairy\0"			\
		"mmcdev=0\0" \
		"satadev=0\0" \
		"bootcmd_nand=run bootargs_base bootargs_nand; "     \
			"nand read ${loadaddr} 0x320000 0x400000;bootm\0 " \
		"bootcmd_mmc=run bootargs_base bootargs_mmc; "	\
			"mmc dev ${mmcdev}; mmc read ${loadaddr} 0x800 0x2200;bootm\0" \
		"bootcmd_sata=run bootargs_base bootargs_sata; "	\
			"sata read ${loadaddr} 0x800 0x2200;bootm\0" \
		"bootenv=boot/uEnv.txt\0" \
		"loadbootenv_mmc=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
		"loadbootenv_sata=fatload sata ${satadev} ${loadaddr} ${bootenv}\0" \
		"importbootenv=echo Importing environment...; " \
		"env import -t $loadaddr $filesize\0" \
		"loaduimage_mmc=fatload mmc ${mmcdev} ${loadaddr} boot/uImage\0" \
		"loaduimage_sata=fatload sata ${satadev} ${loadaddr} boot/uImage\0" \
		"splashimage=0x10800000\0"				\
		"splashimage_mmc_init_block=0x410\0"			\
		"splashimage_mmc_blkcnt=0x3F0\0"			\
		"splashimage_nand_offset=0x2A0000\0"			\
		"splashimage_nand_size=0x80000\0"			\
		"splashimage_file_name=boot/out.bmp.gz\0"		\
		"splashpos=m,m\0"

#ifdef CONFIG_ENV_IS_IN_BOOT_DEVICE
#define CONFIG_BOOTCOMMAND \
	"if test ${bootmode} = MMC; then " \
		"setenv loadbootenv ${loadbootenv_mmc};" \
		"setenv loaduimage ${loaduimage_mmc};" \
		"setenv bootcmd ${bootcmd_mmc};" \
		"setenv rootdevice ${mmc_rootdevice};" \
		"setenv bootargs_bootdev ${bootargs_mmc};" \
		"setenv fatbootdev ${fat_rootdevice} rootwait rw;" \
	"fi;" \
	"if test ${bootmode} = SATA; then " \
		"setenv loadbootenv ${loadbootenv_sata};" \
		"setenv loaduimage ${loaduimage_sata};" \
		"setenv bootcmd ${bootcmd_sata};" \
		"setenv rootdevice ${sata_rootdevice};" \
		"setenv bootargs_bootdev ${bootargs_sata};" \
		"setenv fatbootdev ${fat_rootdevice} rootwait rw;" \
	"fi;" \
	"if test ${bootmode} = NAND; then " \
		"run bootcmd_nand;" \
	"fi;" \
	"if run loadbootenv; then " \
		"echo Loaded environment from ${bootenv};" \
		"run importbootenv;" \
	"fi;" \
	"if test -n $uenvcmd; then " \
		"echo Running uenvcmd ...;" \
		"run uenvcmd;" \
	"fi;" \
	"if run loaduimage; then " \
		"setenv rootdevice ${fatbootdev};" \
		"run bootargs_base bootargs_bootdev;" \
		"bootm;" \
	"fi;" \
	"echo Reading uImage from RAW...;" \
	"run bootcmd;"
#endif
#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#endif			       /* __CONFIG_H * */
