/*
 * Copyright (C) 2015 Technexion Ltd.
 *
 * Author: Richard Hu <richard.hu@technexion.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)

/*
 * driving strength
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */

#define IMX6SDL_DRIVE_STRENGTH 0x28U

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	.dram_sdclk_0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_cas = IMX6SDL_DRIVE_STRENGTH,
	.dram_ras = IMX6SDL_DRIVE_STRENGTH,
	.dram_reset = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6SDL_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ctlds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b1ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b2ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b3ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b4ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b5ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b6ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b7ds = IMX6SDL_DRIVE_STRENGTH,
};

/* H5T04G63AFR-PB for i.mx6Solo/DL operating DDR at 400MHz */
static struct mx6_ddr3_cfg h5t04g63afr_800mhz = {
	.mem_speed = 800,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1500,
	.trcmin = 5250,
	.trasmin = 3750,
};

/* H5TQ2G63FFR-H9 for i.mx6Solo/DL operating DDR at 400MHz */
static struct mx6_ddr3_cfg h5tq2g63ffr_800mhz = {
	.mem_speed = 800,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1500,
	.trcmin = 5250,
	.trasmin = 3750,
};

/*
 * calibration parameters for pisosom
 */

/* calibration parameters for i.mx6DL with h5t04g63afr_800mhz */
static struct mx6_mmdc_calibration mx6dl_1g_mmdc_calib = {
	.p0_mpwldectrl0 = 0x003D0044,
	.p0_mpwldectrl1 = 0x0036003A,
	.p0_mpdgctrl0 = 0x0240023C,
	.p0_mpdgctrl1 = 0x0224022C,
	.p0_mprddlctl = 0x42464848,
	.p0_mpwrdlctl = 0x3A383636,
};

static struct mx6_mmdc_calibration mx6s_512m_mmdc_calib = {
	.p0_mpwldectrl0 = 0x0033003C,
	.p0_mpwldectrl1 = 0x002E0032,
	.p0_mpdgctrl0 = 0x02300224,
	.p0_mpdgctrl1 = 0x02140218,
	.p0_mprddlctl = 0x40444A48,
	.p0_mpwrdlctl = 0x38362E2E,
};

/* DDR bus width is 32bit */
static struct mx6_ddr_sysinfo ddr_sysinfo_32bit = {
	.dsize		= 1,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static void spl_dram_init(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_MX6SOLO:
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&ddr_sysinfo_32bit, &mx6s_512m_mmdc_calib, &h5tq2g63ffr_800mhz);
		break;
	case MXC_CPU_MX6DL:
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&ddr_sysinfo_32bit, &mx6dl_1g_mmdc_calib, &h5t04g63afr_800mhz);
		break;
	}
	udelay(100);
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}
#endif