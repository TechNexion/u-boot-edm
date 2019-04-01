/*
 * Copyright (C) 2012-2016 Freescale Semiconductor, Inc.
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __MX6_COMMON_H
#define __MX6_COMMON_H

#if (defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL))
#define CONFIG_SC_TIMER_CLK 8000000 /* 8Mhz */
#define COUNTER_FREQUENCY CONFIG_SC_TIMER_CLK
#else
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	L2_PL310_BASE
#endif

#define CONFIG_MP
#endif
#define CONFIG_BOARD_POSTCLK_INIT
#define CONFIG_MXC_GPT_HCLK

#define CONFIG_SYS_BOOTM_LEN		0xA000000

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#ifndef CONFIG_MX6
#define CONFIG_MX6
#endif

#define CONFIG_SYS_FSL_CLK

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Boot options */
#if defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL) || \
	defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
#define CONFIG_LOADADDR		0x80800000
#else
#define CONFIG_LOADADDR		0x12000000
#endif
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX       1
#define CONFIG_BAUDRATE         115200

/* Filesystems and image support */
/* #define CONFIG_SUPPORT_RAW_INITRD -- redefined */

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE	512
#define CONFIG_SYS_MAXARGS	32
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#ifndef CONFIG_SYS_DCACHE_OFF
#endif

/* MMC */
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SUPPORT_EMMC_BOOT

/* Fuses */
#define CONFIG_MXC_OCOTP

/* Secure boot (HAB) support */
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CSF_SIZE			0x4000
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_DRIVERS_MISC_SUPPORT
#endif
#endif

#ifdef CONFIG_IMX_OPTEE
#define TEE_ENV "tee=yes\0"
#else
#define TEE_ENV "tee=no\0"
#endif
#endif
