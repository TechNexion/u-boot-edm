/*
 * Copyright 2018 Wandboard.org
 * Copyright 2017 NXP
 * 
 * Author: Richard Hu <richard.hu@technexion.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/arch/ddr_memory_map.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/imx-common/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include "ddr/ddr.h"

DECLARE_GLOBAL_DATA_PTR;

static iomux_v3_cfg_t const ver_det_pads[] = {
	IMX8MQ_PAD_NAND_DATA01__GPIO3_IO7 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA02__GPIO3_IO8 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA03__GPIO3_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA04__GPIO3_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA05__GPIO3_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA06__GPIO3_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA07__GPIO3_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define DDR_DET_1		IMX_GPIO_NR(3, 11)
#define DDR_DET_2		IMX_GPIO_NR(3, 12)
#define DDR_DET_3		IMX_GPIO_NR(3, 13)

static void setup_iomux_ver_det(void)
{
	imx_iomux_v3_setup_multiple_pads(ver_det_pads, ARRAY_SIZE(ver_det_pads));

	gpio_request(DDR_DET_1, "ddr_det_1");
	gpio_direction_input(DDR_DET_1);
	gpio_request(DDR_DET_2, "ddr_det_2");
	gpio_direction_input(DDR_DET_2);
	gpio_request(DDR_DET_3, "ddr_det_3");
	gpio_direction_input(DDR_DET_3);
}
/***********************************************
DDR_DET_1    DDR_DET_2   DDR_DET_3
   1            1            1       3G LPDDR4
   1            1            0       2G LPDDR4
   1            0            1       1G LPDDR4
************************************************/
void spl_dram_init(void)
{
	setup_iomux_ver_det();

	/*************************************************
	ToDo: It's a dirty workaround to store the
	information of DDR size into start address of TCM.
	U-boot would extract this information in dram_init().
	**************************************************/
	if (gpio_get_value(DDR_DET_1) && gpio_get_value(DDR_DET_2) && gpio_get_value(DDR_DET_3)) {
		puts("dram_init: LPDDR4 3GB\n");
		ddr_init_3gb();
		writel(0x3, M4_BOOTROM_BASE_ADDR);
	}
	else if (gpio_get_value(DDR_DET_1) && gpio_get_value(DDR_DET_2) && !gpio_get_value(DDR_DET_3)) {
		puts("dram_init: LPDDR4: 2GB\n");
		ddr_init_2gb();
		writel(0x2, M4_BOOTROM_BASE_ADDR);
	}
	else if (gpio_get_value(DDR_DET_1) && !gpio_get_value(DDR_DET_2) && gpio_get_value(DDR_DET_3)) {
		puts("dram_init: LPDDR4: 1GB\n");
		ddr_init_1gb();
		writel(0x1, M4_BOOTROM_BASE_ADDR);
	}
	else
		puts("Unknown DDR type!!!\n");
}

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 12)
#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)
#define USDHC2_PWR_GPIO IMX_GPIO_NR(2, 19)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		return ret;
	}

	return 1;
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_DSE1)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IMX8MQ_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IMX8MQ_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0x16 */
	IMX8MQ_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_CD_B__GPIO2_IO12 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MQ_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC2_BASE_ADDR, 0, 4},
};

int board_mmc_init(bd_t *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(USDHC1_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(
				usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
			gpio_direction_output(USDHC1_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC1_PWR_GPIO, 1);
			break;
		case 1:
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(USDHC2_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(
				usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			gpio_request(USDHC2_PWR_GPIO, "usdhc2_reset");
			gpio_direction_output(USDHC2_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_PWR_GPIO, 1);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

void spl_board_init(void)
{
	enable_tzc380();

	/* Adjust pmic voltage to 1.0V for 800M */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	//power_init_board();

	/* DDR initialization */
	spl_dram_init();

	/* Serial download mode */
	if (is_usb_boot()) {
		puts("Back to ROM, SDP\n");
		restore_boot_params();
	}
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}
