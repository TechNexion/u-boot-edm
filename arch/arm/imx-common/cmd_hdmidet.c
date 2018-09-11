/*
 * Copyright (C) 2012 Boundary Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/imx-regs.h>
#if defined(CONFIG_VIDEO_IMXDCSS)
#include <asm/arch/video_common.h>
#include <imx8_hdmi.h>
#else
#include <asm/arch/mxc_hdmi.h>
#endif
#include <asm/io.h>

static int do_hdmidet(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_VIDEO_IMXDCSS)
	return imx8_hdmi_hpd_status() & 1 ? 0 : 1;
#else
	struct hdmi_regs *hdmi = (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
	return (readb(&hdmi->phy_stat0) & HDMI_DVI_STAT) ? 0 : 1;
#endif
}

U_BOOT_CMD(hdmidet, 1, 1, do_hdmidet,
	"detect HDMI monitor",
	""
);
