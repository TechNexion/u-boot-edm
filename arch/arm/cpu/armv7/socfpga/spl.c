/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <version.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

/*
 * Board initialization after bss clearance
 */
void spl_board_init(void)
{
#ifndef CONFIG_SOCFPGA_VIRTUAL_TARGET
	/* configure the pin muxing through system manager */
	sysmgr_pinmux_init();
#endif /* CONFIG_SOCFPGA_VIRTUAL_TARGET */

	/* de-assert reset for peripherals and bridges based on handoff */
	reset_deassert_peripherals_handoff();

	/* enable console uart printing */
	preloader_console_init();
}
