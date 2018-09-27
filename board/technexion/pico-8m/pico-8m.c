/*
 * Copyright 2018 TechNexion Ltd.
 * Copyright 2017 NXP
 * 
 * Author: Richard Hu <richard.hu@technexion.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <asm/imx-common/video.h>
#include <asm/arch/video_common.h>
#include <spl.h>
#include <usb.h>
#include <dwc3-uboot.h>

DECLARE_GLOBAL_DATA_PTR;

#define QSPI_PAD_CTRL	(PAD_CTL_DSE2 | PAD_CTL_HYS)

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static int ddr_size;

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	/* TODO */
	return 0;
}
#endif

int dram_init(void)
{
	/*************************************************
	ToDo: It's a dirty workaround to store the
	information of DDR size into start address of TCM.
	It'd be better to detect DDR size from DDR controller.
	**************************************************/
	ddr_size = readl(M4_BOOTROM_BASE_ADDR);

	if (ddr_size == 0x4) {
		/* rom_pointer[1] contains the size of TEE occupies */
		if (rom_pointer[1])
			gd->ram_size = PHYS_SDRAM_SIZE_3GB - rom_pointer[1];
		else
			gd->ram_size = PHYS_SDRAM_SIZE_3GB;
	}
	else if (ddr_size == 0x3) {
		/* rom_pointer[1] contains the size of TEE occupies */
		if (rom_pointer[1])
			gd->ram_size = PHYS_SDRAM_SIZE_3GB - rom_pointer[1];
		else
			gd->ram_size = PHYS_SDRAM_SIZE_3GB;
	}
	else if (ddr_size == 0x2) {
		if (rom_pointer[1])
			gd->ram_size = PHYS_SDRAM_SIZE_2GB - rom_pointer[1];
		else
			gd->ram_size = PHYS_SDRAM_SIZE_2GB;
	}
	else if (ddr_size == 0x1) {
		if (rom_pointer[1])
			gd->ram_size = PHYS_SDRAM_SIZE_1GB - rom_pointer[1];
		else
			gd->ram_size = PHYS_SDRAM_SIZE_1GB;
	}
	else
		puts("Unknown DDR type!!!\n");

	return 0;
}

/* Get the top of usable RAM */
ulong board_get_usable_ram_top(ulong total_size)
{
        if(gd->ram_top > 0x100000000)
            gd->ram_top = 0x100000000;

        return gd->ram_top;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
#define FEC_RST_PAD IMX_GPIO_NR(1, 9)
static iomux_v3_cfg_t const fec1_rst_pads[] = {
	IMX8MQ_PAD_GPIO1_IO09__GPIO1_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define FEC_PWR_PAD IMX_GPIO_NR(1, 0)
static iomux_v3_cfg_t const fec1_pwr_pads[] = {
	IMX8MQ_PAD_GPIO1_IO00__GPIO1_IO0 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define WL_REG_ON_PAD IMX_GPIO_NR(3, 24)
static iomux_v3_cfg_t const wl_reg_on_pads[] = {
	IMX8MQ_PAD_SAI5_RXD3__GPIO3_IO24 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define BT_ON_PAD IMX_GPIO_NR(3, 21)
static iomux_v3_cfg_t const bt_on_pads[] = {
	IMX8MQ_PAD_SAI5_RXD0__GPIO3_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_rst_pads, ARRAY_SIZE(fec1_rst_pads));
	imx_iomux_v3_setup_multiple_pads(fec1_pwr_pads, ARRAY_SIZE(fec1_pwr_pads));

	gpio_request(IMX_GPIO_NR(1, 0), "fec1_pwr");
	gpio_direction_output(IMX_GPIO_NR(1, 0), 1);
	udelay(500);

	gpio_request(IMX_GPIO_NR(1, 9), "fec1_rst");
	gpio_direction_output(IMX_GPIO_NR(1, 9), 0);
	udelay(500);
	gpio_direction_output(IMX_GPIO_NR(1, 9), 1);
}

static int setup_fec(void)
{
	setup_iomux_fec();

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(IOMUXC_GPR1,
			BIT(13) | BIT(17), 0);
	return set_clk_enet(ENET_125MHz);
}


int board_phy_config(struct phy_device *phydev)
{
	/* enable rgmii rxc skew and phy mode select to RGMII copper */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.power_down_scale = 2,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
int board_usb_init(int index, enum usb_init_type init)
{
	imx8m_usb_power(index, true);

	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_nxp_usb_phy_init(&dwc3_device_data);
		return dwc3_uboot_init(&dwc3_device_data);
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_uboot_exit(index);
	}

	imx8m_usb_power(index, false);

	return 0;
}
#endif

#define DDR_DET_1		IMX_GPIO_NR(3, 11)
#define DDR_DET_2		IMX_GPIO_NR(3, 12)
#define DDR_DET_3		IMX_GPIO_NR(3, 13)

static iomux_v3_cfg_t const ver_det_pads[] = {
	IMX8MQ_PAD_NAND_DATA01__GPIO3_IO7 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA02__GPIO3_IO8 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA03__GPIO3_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA04__GPIO3_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA05__GPIO3_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA06__GPIO3_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA07__GPIO3_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

void setup_iomux_ver_det(void)
{
	imx_iomux_v3_setup_multiple_pads(ver_det_pads, ARRAY_SIZE(ver_det_pads));

	gpio_request(DDR_DET_1, "ddr_det_1");
	gpio_direction_input(DDR_DET_1);
	gpio_request(DDR_DET_2, "ddr_det_2");
	gpio_direction_input(DDR_DET_2);
	gpio_request(DDR_DET_3, "ddr_det_3");
	gpio_direction_input(DDR_DET_3);
}

void setup_wifi(void)
{
	imx_iomux_v3_setup_multiple_pads(wl_reg_on_pads, ARRAY_SIZE(wl_reg_on_pads));
	imx_iomux_v3_setup_multiple_pads(bt_on_pads, ARRAY_SIZE(bt_on_pads));

	gpio_request(WL_REG_ON_PAD, "wl_reg_on");
	gpio_direction_output(WL_REG_ON_PAD, 0);
	gpio_set_value(WL_REG_ON_PAD, 0);

	gpio_request(BT_ON_PAD, "bt_on");
	gpio_direction_output(BT_ON_PAD, 0);
	gpio_set_value(BT_ON_PAD, 0);
}


int board_init(void)
{
	setup_wifi();

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

static int check_mmc_autodetect(void)
{
	char *autodetect_str = getenv("mmcautodetect");

	if ((autodetect_str != NULL) &&
		(strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

/* This should be defined for each board */
__weak int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	if (!check_mmc_autodetect())
		return;

	setenv_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	setenv("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	setenv("board_name", "PICO-8M");
	setenv("board_rev", "iMX8MQ");

	setup_iomux_ver_det();
	/***********************************************
	DDR_DET_1    DDR_DET_2   DDR_DET_3
	0            0            1       4G LPDDR4
	1            1            1       3G LPDDR4
	1            1            0       2G LPDDR4
	1            0            1       1G LPDDR4
	************************************************/
	if (!gpio_get_value(DDR_DET_1) && !gpio_get_value(DDR_DET_2) && gpio_get_value(DDR_DET_3)) {
		/* LPDDR4 4GB */
		setenv("cma_size", "1G");
	}
	else if (gpio_get_value(DDR_DET_1) && gpio_get_value(DDR_DET_2) && gpio_get_value(DDR_DET_3)) {
		/* LPDDR4 3GB */
		setenv("cma_size", "1G");
	}
	else if (gpio_get_value(DDR_DET_1) && gpio_get_value(DDR_DET_2) && !gpio_get_value(DDR_DET_3)) {
		/* LPDDR4 2GB */
		setenv("cma_size", "1G");
	}
	else if (gpio_get_value(DDR_DET_1) && !gpio_get_value(DDR_DET_2) && gpio_get_value(DDR_DET_3)) {
		/* LPDDR4 1GB */
		setenv("cma_size", "768M");
	}
	else
		puts("Unknown DDR type!!!\n");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /*TODO*/
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/

#if defined(CONFIG_VIDEO_IMXDCSS)

struct display_info_t const displays[] = {{
	.bus	= 0, /* Unused */
	.addr	= 0, /* Unused */
	.pixfmt	= GDF_32BIT_X888RGB,
	.detect	= NULL,
	.enable	= NULL,
#ifndef CONFIG_VIDEO_IMXDCSS_1080P
	.mode	= {
		.name           = "HDMI", /* 720P60 */
		.refresh        = 60,
		.xres           = 1280,
		.yres           = 720,
		.pixclock       = 13468, /* 74250  kHz */
		.left_margin    = 110,
		.right_margin   = 220,
		.upper_margin   = 5,
		.lower_margin   = 20,
		.hsync_len      = 40,
		.vsync_len      = 5,
		.sync           = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode          = FB_VMODE_NONINTERLACED
	}
#else
	.mode	= {
		.name           = "HDMI", /* 1080P60 */
		.refresh        = 60,
		.xres           = 1920,
		.yres           = 1080,
		.pixclock       = 6734, /* 148500 kHz */
		.left_margin    = 148,
		.right_margin   = 88,
		.upper_margin   = 36,
		.lower_margin   = 4,
		.hsync_len      = 44,
		.vsync_len      = 5,
		.sync           = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode          = FB_VMODE_NONINTERLACED
	}
#endif
} };
size_t display_count = ARRAY_SIZE(displays);

#endif /* CONFIG_VIDEO_IMXDCSS */

