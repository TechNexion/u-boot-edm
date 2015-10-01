/*
 * Copyright (C) 2013 TechNexion Ltd.
 *
 * Author: Richard Hu <linuxfae@technexion.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#ifdef CONFIG_SPL
#include <spl.h>
#endif
#include "mx6_pins.h"
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <ipu_pixfmt.h>
#include <mmc.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/fb.h>
#include <nand.h>
#include <part.h>

DECLARE_GLOBAL_DATA_PTR;

#define MX6QDL_SET_PAD(p, q) \
        if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)) \
                imx_iomux_v3_setup_pad(MX6Q_##p | q); \
        else \
                imx_iomux_v3_setup_pad(MX6DL_##p | q);

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define USDHC1_CD_GPIO		IMX_GPIO_NR(1, 2)
#define USDHC3_CD_GPIO		IMX_GPIO_NR(3, 9)
#define ETH_PHY_RESET		IMX_GPIO_NR(3, 29)


/*
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */

#define DRIVE_STRENGTH_IMX6SOLO 0x28U
#define DRIVE_STRENGTH_IMX6DL 0x28U
#define DRIVE_STRENGTH_IMX6D 0x28U

static enum boot_device boot_dev;
enum boot_device get_boot_device(void);

static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;
	uint bt_mem_mmc = (soc_sbmr & 0x00001000) >> 12;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = MX6_ONE_NAND_BOOT;
		else
			boot_dev = MX6_WEIM_NOR_BOOT;
		break;
	case 0x2:
			boot_dev = MX6_SATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = MX6_I2C_BOOT;
		else
			boot_dev = MX6_SPI_NOR_BOOT;
		break;
	case 0x4:
	case 0x5:
		if (bt_mem_mmc)
			boot_dev = MX6_SD0_BOOT;
		else
			boot_dev = MX6_SD1_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MX6_MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = MX6_NAND_BOOT;
		break;
	default:
		boot_dev = MX6_UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void) {
	return boot_dev;
}


int dram_init(void)
{
	uint cpurev, imxtype;
	u32 sdram_size;

	cpurev = get_cpu_rev();
	imxtype = (cpurev & 0xFF000) >> 12;
			
	switch (imxtype){
	case MXC_CPU_MX6SOLO:
		sdram_size = 512 * 1024 * 1024;
		break;
	case MXC_CPU_MX6Q:
		sdram_size = 2u * 1024 * 1024 * 1024;
		break;
	case MXC_CPU_MX6D:
		sdram_size = 1u * 1024 * 1024 * 1024;
		break;
	case MXC_CPU_MX6DL:
	default:
		sdram_size = 1u * 1024 * 1024 * 1024;;
		break;	
	}
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, sdram_size);

	return 0;
}

static void setup_iomux_usdhc1(void)
{
	MX6QDL_SET_PAD(PAD_SD1_CLK__USDHC1_CLK, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD1_CMD__USDHC1_CMD, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD1_DAT0__USDHC1_DAT0, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD1_DAT1__USDHC1_DAT1, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD1_DAT2__USDHC1_DAT2, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD1_DAT3__USDHC1_DAT3, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	// Carrier MicroSD Card Detect
	MX6QDL_SET_PAD(PAD_GPIO_2__GPIO_1_2, MUX_PAD_CTRL(NO_PAD_CTRL));
}

static void setup_iomux_usdhc3(void)
{
	MX6QDL_SET_PAD(PAD_SD3_CLK__USDHC3_CLK , MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD3_CMD__USDHC3_CMD , MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD3_DAT0__USDHC3_DAT0, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD3_DAT1__USDHC3_DAT1, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD3_DAT2__USDHC3_DAT2, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD3_DAT3__USDHC3_DAT3, MUX_PAD_CTRL(USDHC_PAD_CTRL));
	// SOM MicroSD Card Detect
	MX6QDL_SET_PAD(PAD_EIM_DA9__GPIO_3_9, MUX_PAD_CTRL(NO_PAD_CTRL));
}

static void setup_iomux_fec(void)
{
	MX6QDL_SET_PAD(PAD_ENET_MDIO__ENET_MDIO, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_ENET_MDC__ENET_MDC, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TXC__ENET_RGMII_TXC, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TD0__ENET_RGMII_TD0, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TD1__ENET_RGMII_TD1, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TD2__ENET_RGMII_TD2, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TD3__ENET_RGMII_TD3, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_TX_CTL__RGMII_TX_CTL, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_ENET_REF_CLK__ENET_TX_CLK, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RXC__ENET_RGMII_RXC, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RD0__ENET_RGMII_RD0, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RD1__ENET_RGMII_RD1, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RD2__ENET_RGMII_RD2, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RD3__ENET_RGMII_RD3, MUX_PAD_CTRL(ENET_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_RGMII_RX_CTL__RGMII_RX_CTL, MUX_PAD_CTRL(ENET_PAD_CTRL));
	// AR8031 PHY Reset
	MX6QDL_SET_PAD(PAD_EIM_D29__GPIO_3_29, MUX_PAD_CTRL(NO_PAD_CTRL));
}

static void setup_iomux_nfc(void)
{
	MX6QDL_SET_PAD(PAD_NANDF_CLE__RAWNAND_CLE, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_ALE__RAWNAND_ALE, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_WP_B__RAWNAND_RESETN, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_RB0__RAWNAND_READY0, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_CS0__RAWNAND_CE0N, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_CS1__RAWNAND_CE1N, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_CS2__RAWNAND_CE2N, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_CS3__RAWNAND_CE3N, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD4_CMD__RAWNAND_RDN	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD4_CLK__RAWNAND_WRN	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D0__RAWNAND_D0	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D1__RAWNAND_D1	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D2__RAWNAND_D2	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D3__RAWNAND_D3	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D4__RAWNAND_D4	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D5__RAWNAND_D5	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D6__RAWNAND_D6	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_NANDF_D7__RAWNAND_D7	, MUX_PAD_CTRL(NO_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_SD4_DAT0__RAWNAND_DQS, MUX_PAD_CTRL(NO_PAD_CTRL));
};

static void setup_iomux_uart(void)
{
	MX6QDL_SET_PAD(PAD_CSI0_DAT10__UART1_TXD, MUX_PAD_CTRL(UART_PAD_CTRL));
	MX6QDL_SET_PAD(PAD_CSI0_DAT11__UART1_RXD, MUX_PAD_CTRL(UART_PAD_CTRL));
}

static void setup_iomux_enet(void)
{
	setup_iomux_fec();
	/* Reset AR8031 PHY */
	gpio_direction_output(ETH_PHY_RESET, 0);
	udelay(500);
	gpio_set_value(ETH_PHY_RESET, 1);
}

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC3_BASE_ADDR},
	{USDHC1_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;
	/*
	 * Following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    SOM MicroSD
	 * mmc1                    Carrier board MicroSD
	 */
	switch (get_boot_device()) {
		case MX6_SD1_BOOT:
			usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			usdhc_cfg[0].max_bus_width = 4;
			usdhc_cfg[1].esdhc_base = USDHC3_BASE_ADDR;
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			usdhc_cfg[1].max_bus_width = 4;
		break;
		case MX6_SD0_BOOT:
		default:
			usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			usdhc_cfg[0].max_bus_width = 4;
			usdhc_cfg[1].esdhc_base = USDHC1_BASE_ADDR;
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			usdhc_cfg[1].max_bus_width = 4;
		break;
	}

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
			setup_iomux_usdhc3();
			gpio_direction_input(USDHC3_CD_GPIO);
			break;
		case 1:
			setup_iomux_usdhc1();
			gpio_direction_input(USDHC1_CD_GPIO);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       index + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}
	return status;
}

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	setup_iomux_nfc();

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}

static int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void enable_hdmi(void)
{
	struct hdmi_regs *hdmi	= (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
	u8 reg;
	reg = readb(&hdmi->phy_conf0);
	reg |= HDMI_PHY_CONF0_PDZ_MASK;
	writeb(reg, &hdmi->phy_conf0);

	udelay(3000);
	reg |= HDMI_PHY_CONF0_ENTMDS_MASK;
	writeb(reg, &hdmi->phy_conf0);
	udelay(3000);
	reg |= HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
	writeb(reg, &hdmi->phy_conf0);
	writeb(HDMI_MC_PHYRSTZ_ASSERT, &hdmi->mc_phyrstz);
}

static struct fb_videomode const hdmi = {
	.name           = "HDMI",
	.refresh        = 60,
	.xres           = 1280,
	.yres           = 720,
	.pixclock       = 13468,
	.left_margin    = 220,
	.right_margin   = 110,
	.upper_margin   = 20,
	.lower_margin   = 5,
	.hsync_len      = 40,
	.vsync_len      = 5,
	.sync           = FB_SYNC_EXT,
	.vmode          = FB_VMODE_NONINTERLACED
};

int board_video_skip(void)
{
	int ret;

	ret = ipuv3_fb_init(&hdmi, 0, IPU_PIX_FMT_RGB24);

	if (ret)
		printf("HDMI cannot be configured: %d\n", ret);

	enable_hdmi();

	return ret;
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct hdmi_regs *hdmi	= (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
	int reg;

	/* Turn on IPU clock */
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_IPU1_IPU_DI0_OFFSET;
	writel(reg, &mxc_ccm->CCGR3);

	/* Turn on HDMI PHY clock */
	reg = readl(&mxc_ccm->CCGR2);
	reg |=  MXC_CCM_CCGR2_HDMI_TX_IAHBCLK_MASK
		| MXC_CCM_CCGR2_HDMI_TX_ISFRCLK_MASK;
	writel(reg, &mxc_ccm->CCGR2);

	/* clear HDMI PHY reset */
	writeb(HDMI_MC_PHYRSTZ_DEASSERT, &hdmi->mc_phyrstz);

	reg = readl(&mxc_ccm->chsccdr);
	reg &= ~(MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK
		| MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK
		| MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET)
	      | (CHSCCDR_PODF_DIVIDE_BY_3
		<< MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET)
	      | (CHSCCDR_IPU_PRE_CLK_540M_PFD
		<< MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);
}
#endif /* CONFIG_VIDEO_IPUV3 */
#ifdef CONFIG_SPLASH_SCREEN
extern block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];
extern int sata_curr_device;
int splash_screen_prepare(void)
{
        ulong addr;
        char *s;
        __maybe_unused  int n, err;
        __maybe_unused  ulong offset = 0, size = 0, count = 0;
	__maybe_unused  struct mmc *mmc = NULL;
	__maybe_unused  struct block_dev_desc_t *sata = NULL;

	printf("%s\n",__FUNCTION__);
        s = getenv("splashimage");
        if (s != NULL) {

                addr = simple_strtoul(s, NULL, 16);
                switch (get_boot_device()) {
                        case MX6_SD0_BOOT:
                        case MX6_SD1_BOOT:
			case MX6_MMC_BOOT:
				mmc = find_mmc_device(0);

				if (!mmc){
					printf ("splash:Error Reading MMC.\n");
				}

				mmc_init(mmc);

				if((s = getenv("splashimage_file_name")) != NULL) {
					err = fat_register_device(&mmc->block_dev,
			  			CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION);
					if (err) {
						printf("spl: fat register err - %d\n", err);
					}			
					err = file_fat_read(s, (ulong *)addr, 0);
					if (err <= 0) {
						printf("splash:error reading %s from MMC FAT\n", s);
						printf("splash:reading from MMC RAW\n");
						if((s = getenv("splashimage_mmc_init_block")) != NULL)
							offset = simple_strtoul (s, NULL, 16);
						if (!offset){
							printf ("splashimage_mmc_init_block is illegal! \n");
						}
						if((s = getenv("splashimage_mmc_blkcnt")) != NULL)
							size = simple_strtoul (s, NULL, 16);
						if (!size){
							printf ("splashimage_mmc_blkcnt is illegal! \n");
						}
						n = mmc->block_dev.block_read(0, offset, size, (ulong *)addr);
						if (n == 0)
							printf("splash: mmc blk read err \n");
					}
					else
						printf("splash:loading %s from MMC FAT...\n", s);	
				}	
				break;
			case MX6_SATA_BOOT:
				if (sata_curr_device == -1) {
					if (sata_initialize())
						return 1;
					sata_curr_device = 0;
				}

				if((s = getenv("splashimage_file_name")) != NULL) {
					err = fat_register_device(&sata_dev_desc[sata_curr_device],
			  			CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION);
					if (err) {
						printf("spl: fat register err - %d\n", err);
					}			
					err = file_fat_read(s, (ulong *)addr, 0);
					if (err <= 0) {
						printf("splash:error reading %s from SATA FAT\n", s);
						printf("splash:reading from SATA RAW\n");
						if((s = getenv("splashimage_mmc_init_block")) != NULL)
							offset = simple_strtoul (s, NULL, 16);
						if (!offset){
							printf ("splashimage_mmc_init_block is illegal! \n");
						}
						if((s = getenv("splashimage_mmc_blkcnt")) != NULL)
							size = simple_strtoul (s, NULL, 16);
						if (!size){
							printf ("splashimage_mmc_blkcnt is illegal! \n");
						}
						count = sata_read(sata_curr_device, offset, size, (ulong *)addr);
						/* flush cache after read */
						flush_cache(addr, size * sata_dev_desc[sata_curr_device].blksz);
						printf("splash:SATA %ld blocks read: %s\n", count, (count==size) ? "OK" : "ERROR");
						if (count != size) {
							printf("splash: SATA blk read err \n");
						}
					}
					else
						printf("splash:loading %s from SATA FAT...\n", s);	
				}	
				break;
			default:
                        	break;
                }
                return 0;
        }
        return -1;
}
#endif

int board_eth_init(bd_t *bis)
{
	int ret;

	setup_iomux_enet();

	ret = cpu_eth_init(bis);
	if (ret)
		printf("FEC MXC: %s:failed\n", __func__);

	return 0;
}

int board_early_init_f(void)
{
	setup_iomux_uart();
#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
#ifdef CONFIG_NO_CONSOLE
	return 0;
#else 
	return 1;
#endif
}

#ifdef CONFIG_CMD_SATA

int setup_sata(void)
{
	struct iomuxc_base_regs *const iomuxc_regs
		= (struct iomuxc_base_regs *) IOMUXC_BASE_ADDR;
	int ret = enable_sata_clock();
	if (ret)
		return ret;

	clrsetbits_le32(&iomuxc_regs->gpr[13],
			IOMUXC_GPR13_SATA_MASK,
			IOMUXC_GPR13_SATA_PHY_8_RXEQ_3P0DB
			|IOMUXC_GPR13_SATA_PHY_7_SATA2M
			|IOMUXC_GPR13_SATA_SPEED_3G
			|(3<<IOMUXC_GPR13_SATA_PHY_6_SHIFT)
			|IOMUXC_GPR13_SATA_SATA_PHY_5_SS_DISABLED
			|IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_9_16
			|IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P00_DB
			|IOMUXC_GPR13_SATA_PHY_2_TX_1P104V
			|IOMUXC_GPR13_SATA_PHY_1_SLOW);

	return 0;
}
#endif



#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	  MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	return 0;
}

int board_init(void)
{
	setup_boot_device();
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_gpmi_nand();

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: edm_cf_imx6\n");

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
static void spl_dram_init_mx6solo_512mb(void);
static void spl_dram_init_mx6dl_1g(void);
static void spl_dram_init_mx6q_2g(void);
static void spl_dram_init(void);

void board_init_f(ulong dummy)
{	
	/* Set the stack pointer. */
	asm volatile("mov sp, %0\n" : : "r"(CONFIG_SPL_STACK));

	spl_dram_init();	
	
	arch_cpu_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Set global data pointer. */
	gd = &gdata;

	board_early_init_f();	

	timer_init();
	preloader_console_init();

	board_init_r(NULL, 0);
}

static void spl_dram_init_mx6solo_512mb(void)
{
	//Copy from DCD in flash_header.S
	//CONFIG_MX6SOLO_DDR3
	/* DDR3 initialization based on the MX6Solo Auto Reference Design (ARD) */
	/* DDR IO TYPE */
	writel(0x000c0000, IOMUXC_BASE_ADDR + 0x774);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x754);
	/* CLOCK */
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4ac);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4b0);
	/* ADDRESS */
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x464);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x490);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x74c);
	/* CONTROLE */
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x494);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x4a0);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4b4);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4b8);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x76c);
	/* DATA STROBE */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x750);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4bc);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4c0);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4c4);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x4c8);
	/* DATA */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x760);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x764);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x770);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x778);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x77c);

	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x470);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x474);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x478);
	writel(DRIVE_STRENGTH_IMX6SOLO, IOMUXC_BASE_ADDR + 0x47c);

	/* ZQ */
	writel(0xA1390003, MMDC_P0_BASE_ADDR + 0x800);

	/* Write leveling */
	writel(0x0053005A, MMDC_P0_BASE_ADDR + 0x80c);
	writel(0x00430048, MMDC_P0_BASE_ADDR + 0x810);
	
	writel(0x024C0244, MMDC_P0_BASE_ADDR + 0x83c);
	writel(0x0234023C, MMDC_P0_BASE_ADDR + 0x840);
	writel(0x3E484A44, MMDC_P0_BASE_ADDR + 0x848);
	writel(0x3A38302E, MMDC_P0_BASE_ADDR + 0x850);
	/* Read data bit delay */
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x828);

	/* Complete calibration by forced measurement */
	writel(0x00000800, MMDC_P0_BASE_ADDR + 0x8b8);
	writel(0x0002002D, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00333040, MMDC_P0_BASE_ADDR + 0x008);
	writel(0x3F435313, MMDC_P0_BASE_ADDR + 0x00c);
	writel(0xB66E8B63, MMDC_P0_BASE_ADDR + 0x010);
	writel(0x01FF00DB, MMDC_P0_BASE_ADDR + 0x014);

	writel(0x00011740, MMDC_P0_BASE_ADDR + 0x018);
	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x000026D2, MMDC_P0_BASE_ADDR + 0x02c);
	writel(0x00431023, MMDC_P0_BASE_ADDR + 0x030);
	writel(0x00000017, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x83190000, MMDC_P0_BASE_ADDR + 0x000);

	writel(0x02008032, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00008033, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00048031, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x05208030, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x04008040, MMDC_P0_BASE_ADDR + 0x01c);

	writel(0x00007800, MMDC_P0_BASE_ADDR + 0x020);
	writel(0x00022227, MMDC_P0_BASE_ADDR + 0x818);
	writel(0x0002556D, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00011006, MMDC_P0_BASE_ADDR + 0x404);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x01c);
}

static void spl_dram_init_mx6dl_1g(void)
{
	//Copy from DCD in flash_header.S
	//CONFIG_MX6DL_DDR3
	/* DDR3 initialization based on the MX6Solo Auto Reference Design (ARD) */
	/* DDR IO TYPE */
	writel(0x000c0000, IOMUXC_BASE_ADDR + 0x774);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x754);	
	/* Clock */
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4ac);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4b0);
	/* Address */
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x464);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x490);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x74c);
	/* Control */
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x494);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x4a0);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4b4);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4b8);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x76c);
	/* Data Strobe */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x750);
	
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4bc);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4c0);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4c4);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4c8);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4cc);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4d0);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4d4);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x4d8);
	/* DATA */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x760);
	
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x764);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x770);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x778);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x77c);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x780);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x784);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x78c);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x748);

	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x470);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x474);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x478);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x47c);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x480);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x484);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x488);
	writel(DRIVE_STRENGTH_IMX6DL, IOMUXC_BASE_ADDR + 0x48c);
	
	/* Calibrations */
	/* ZQ */
	writel(0xA1390003, MMDC_P0_BASE_ADDR + 0x800);

	/* write leveling */
	writel(0x003E0047, MMDC_P0_BASE_ADDR + 0x80c);
	writel(0x00300036, MMDC_P0_BASE_ADDR + 0x810);
	writel(0x0018001B, MMDC_P1_BASE_ADDR + 0x80c);
	writel(0x0010002C, MMDC_P1_BASE_ADDR + 0x810);
	/* DQS gating, read delay, write delay calibration values
	based on calibration compare of 0x00ffff00 */
	writel(0x02380234, MMDC_P0_BASE_ADDR + 0x83c);
	writel(0x02300230, MMDC_P0_BASE_ADDR + 0x840);
	writel(0x0218021C, MMDC_P1_BASE_ADDR + 0x83C);
	writel(0x0210020C, MMDC_P1_BASE_ADDR + 0x840);

	writel(0x40464846, MMDC_P0_BASE_ADDR + 0x848);
	writel(0x40444640, MMDC_P1_BASE_ADDR + 0x848);
	writel(0x3A302E30, MMDC_P0_BASE_ADDR + 0x850);
	writel(0x3630302C, MMDC_P1_BASE_ADDR + 0x850);
	/* read data bit delay */
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x828);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x828);
	/* Complete calibration by forced measurment */
	writel(0x00000800, MMDC_P0_BASE_ADDR + 0x8b8);
	writel(0x00000800, MMDC_P1_BASE_ADDR + 0x8b8);
	/* MMDC init:
	 in DDR3, 64-bit mode, only MMDC0 is initiated: */
	writel(0x0002002D, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00333040, MMDC_P0_BASE_ADDR + 0x008);
	
	writel(0x3F435313, MMDC_P0_BASE_ADDR + 0x00c);
	writel(0xB66E8B63, MMDC_P0_BASE_ADDR + 0x010);

	writel(0x01FF00DB, MMDC_P0_BASE_ADDR + 0x014);
	writel(0x00011740, MMDC_P0_BASE_ADDR + 0x018);
	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x000026D2, MMDC_P0_BASE_ADDR + 0x02c);
	writel(0x00431023, MMDC_P0_BASE_ADDR + 0x030);
	writel(0x00000027, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x831A0000, MMDC_P0_BASE_ADDR + 0x000);

	/* MR */
	writel(0x02008032, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00008033, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00048031, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x05208030, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x04008040, MMDC_P0_BASE_ADDR + 0x01c);

	/* final DDR setup */
	writel(0x00007800, MMDC_P0_BASE_ADDR + 0x020);
	writel(0x00022227, MMDC_P0_BASE_ADDR + 0x818);
	writel(0x00022227, MMDC_P1_BASE_ADDR + 0x818);
	writel(0x0002556D, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00011006, MMDC_P0_BASE_ADDR + 0x404);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x01c);
}

static void spl_dram_init_mx6d_1g(void)
{
	 /* i.MX6D */
	/* DDR IO TYPE */
	writel(0x000C0000, IOMUXC_BASE_ADDR + 0x798);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x758);
	/* Clock */
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x588);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x594);
	/* Address */
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x56c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x578);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x74c);
	/* Control */
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x57c);

	writel(0x00000000, IOMUXC_BASE_ADDR + 0x58c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x59c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5a0);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x78c);
	/* Data Strobe */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x750);

	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5a8);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5b0);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x524);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x51c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x518);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x50c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5b8);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5c0);
	/* Data */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x774);

	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x784);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x788);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x794);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x79c);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x7a0);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x7a4);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x7a8);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x748);

	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5ac);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5b4);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x528);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x520);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x514);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x510);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5bc);
	writel(DRIVE_STRENGTH_IMX6D, IOMUXC_BASE_ADDR + 0x5c4);

	/* Calibrations */
	/* ZQ */
	writel(0xa1390003, MMDC_P0_BASE_ADDR + 0x800);
	/* write leveling */
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x80c);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x810);
	writel(0x00000000, MMDC_P1_BASE_ADDR + 0x80c);
	writel(0x00000000, MMDC_P1_BASE_ADDR + 0x810);
	/* DQS gating, read delay, write delay calibration values
	 based on calibration compare of 0x00ffff00  */
	writel(0x03280334, MMDC_P0_BASE_ADDR + 0x83c);
	writel(0x03280320, MMDC_P0_BASE_ADDR + 0x840);
	writel(0x0330033C, MMDC_P1_BASE_ADDR + 0x83c);
	writel(0x033C0278, MMDC_P1_BASE_ADDR + 0x840);

	writel(0x423A3A3E, MMDC_P0_BASE_ADDR + 0x848);
	writel(0x3C3E3842, MMDC_P1_BASE_ADDR + 0x848);

	writel(0x36384240, MMDC_P0_BASE_ADDR + 0x850);
	writel(0x4A384440, MMDC_P1_BASE_ADDR + 0x850);

	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x828);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x828);

	writel(0x00000800, MMDC_P0_BASE_ADDR + 0x8b8);
	writel(0x00000800, MMDC_P1_BASE_ADDR + 0x8b8);
	/* MMDC init:
	  in DDR3, 64-bit mode, only MMDC0 is initiated: */
	writel(0x00020036, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x09444040, MMDC_P0_BASE_ADDR + 0x008);
	writel(0x54597975, MMDC_P0_BASE_ADDR + 0x00c);
	writel(0xFF538F64, MMDC_P0_BASE_ADDR + 0x010);
	writel(0x01FF00DB, MMDC_P0_BASE_ADDR + 0x014);

	writel(0x00011740, MMDC_P0_BASE_ADDR + 0x018);
	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x000026D2, MMDC_P0_BASE_ADDR + 0x02c);
	writel(0x00591023, MMDC_P0_BASE_ADDR + 0x030);

	/* 1G */
	writel(0x00000027, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x831A0000, MMDC_P0_BASE_ADDR + 0x000);

	writel(0x02088032, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00008033, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00048031, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x09408030, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x04008040, MMDC_P0_BASE_ADDR + 0x01c);

	writel(0x00007800, MMDC_P0_BASE_ADDR + 0x020);

	writel(0x00022227, MMDC_P0_BASE_ADDR + 0x818);
	writel(0x00022227, MMDC_P1_BASE_ADDR + 0x818);

	writel(0x00025576, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00011006, MMDC_P0_BASE_ADDR + 0x404);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x01c);
}

static void spl_dram_init_mx6q_2g(void)
{
	 /* i.MX6Q */
	/* DDR IO TYPE */
	writel(0x000C0000, IOMUXC_BASE_ADDR + 0x798);
	writel(0x00000000, IOMUXC_BASE_ADDR + 0x758);
	/* Clock */
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x588);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x594);
	/* Address */
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x56c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x578);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x74c);
	/* Control */
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x57c);

	writel(0x00000000, IOMUXC_BASE_ADDR + 0x58c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x59c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5a0);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x78c);
	/* Data Strobe */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x750);

	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5a8);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5b0);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x524);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x51c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x518);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x50c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5b8);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5c0);
	/* Data */
	writel(0x00020000, IOMUXC_BASE_ADDR + 0x774);

	writel(0x00000030, IOMUXC_BASE_ADDR + 0x784);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x788);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x794);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x79c);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x7a0);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x7a4);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x7a8);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x748);

	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5ac);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5b4);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x528);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x520);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x514);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x510);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5bc);
	writel(0x00000030, IOMUXC_BASE_ADDR + 0x5c4);

	/* Calibrations */
	/* ZQ */
	writel(0xa1390003, MMDC_P0_BASE_ADDR + 0x800);
	/* write leveling */
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x80c);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x810);
	writel(0x00000000, MMDC_P1_BASE_ADDR + 0x80c);
	writel(0x00000000, MMDC_P1_BASE_ADDR + 0x810);
	/* DQS gating, read delay, write delay calibration values
	 based on calibration compare of 0x00ffff00  */
	writel(0x032C0340, MMDC_P0_BASE_ADDR + 0x83c);
	writel(0x03300324, MMDC_P0_BASE_ADDR + 0x840);
	writel(0x032C0338, MMDC_P1_BASE_ADDR + 0x83c);
	writel(0x03300274, MMDC_P1_BASE_ADDR + 0x840);
	
	writel(0x423A383E, MMDC_P0_BASE_ADDR + 0x848);
	writel(0x3638323E, MMDC_P1_BASE_ADDR + 0x848);

	writel(0x363C4640, MMDC_P0_BASE_ADDR + 0x850);
	writel(0x4034423C, MMDC_P1_BASE_ADDR + 0x850);

	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x828);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x828);

	writel(0x00000800, MMDC_P0_BASE_ADDR + 0x8b8);
	writel(0x00000800, MMDC_P1_BASE_ADDR + 0x8b8);
	/* MMDC init:
	  in DDR3, 64-bit mode, only MMDC0 is initiated: */
	writel(0x00020036, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x09444040, MMDC_P0_BASE_ADDR + 0x008);
	writel(0x898E7955, MMDC_P0_BASE_ADDR + 0x00c);
	writel(0xFF328F64, MMDC_P0_BASE_ADDR + 0x010);
	writel(0x01FF00DB, MMDC_P0_BASE_ADDR + 0x014);
	writel(0x00011740, MMDC_P0_BASE_ADDR + 0x018);

	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x000026D2, MMDC_P0_BASE_ADDR + 0x02c);
	writel(0x008E1023, MMDC_P0_BASE_ADDR + 0x030);
	
	/* 2G */
	writel(0x00000047, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x841A0000, MMDC_P0_BASE_ADDR + 0x000);
	
	/* 1G 
	writel(0x00000027, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x831A0000, MMDC_P0_BASE_ADDR + 0x000);*/

	writel(0x02088032, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00008033, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00048031, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x09408030, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x04008040, MMDC_P0_BASE_ADDR + 0x01c);

	writel(0x00007800, MMDC_P0_BASE_ADDR + 0x020);
	
	writel(0x00022227, MMDC_P0_BASE_ADDR + 0x818);
	writel(0x00022227, MMDC_P1_BASE_ADDR + 0x818);

	writel(0x00025576, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00011006, MMDC_P0_BASE_ADDR + 0x404);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x01c);
}

static void spl_dram_init(void)
{	
	u32 cpurev, imxtype;
	
	cpurev = get_cpu_rev();
	imxtype = (cpurev & 0xFF000) >> 12;

	get_imx_type(imxtype);	

	switch (imxtype){
	case MXC_CPU_MX6SOLO:
		spl_dram_init_mx6solo_512mb();
		break;
	case MXC_CPU_MX6Q:
		spl_dram_init_mx6q_2g();
		break;
	case MXC_CPU_MX6D:
		spl_dram_init_mx6d_1g();
		break;
	case MXC_CPU_MX6DL:
	default:
		spl_dram_init_mx6dl_1g();
		break;	
	}
}


void spl_board_init(void)
{
	setup_gpmi_nand();
	setup_boot_device();
}

u32 spl_boot_device(void)
{	
	puts("Boot Device: ");
	switch (get_boot_device()) {
	case MX6_SD0_BOOT:
		printf("SD0\n");
		return BOOT_DEVICE_MMC1;
	case MX6_SD1_BOOT:
		printf("SD1\n");
		return BOOT_DEVICE_MMC2;
	case MX6_MMC_BOOT:
		printf("MMC\n");
		return BOOT_DEVICE_MMC2;
	case MX6_NAND_BOOT:
		printf("NAND\n");
		return BOOT_DEVICE_NAND;
	case MX6_SATA_BOOT:
		printf("SATA\n");
		return BOOT_DEVICE_SATA;
	case MX6_UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		return BOOT_DEVICE_NONE;
	}
}

u32 spl_boot_mode(void)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		return MMCSD_MODE;
		break;
	case BOOT_DEVICE_SATA:
		return SATA_MODE;
		break;
	//case BOOT_DEVICE_NAND:
	//	return 0;
	//	break;
	default:
		puts("spl: ERROR:  unsupported device\n");
		hang();
	}
}


void reset_cpu(ulong addr)
{
	__REG16(WDOG1_BASE_ADDR) = 4;
}
#endif
