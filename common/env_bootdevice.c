/*
 * (C) Copyright 2010-2014 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <sata.h>
#include <mmc.h>
#include <search.h>
#include <errno.h>
#include <asm/imx-common/boot_mode.h>


#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

char *env_name_spec = "BOOT_DEVICE";

#ifdef ENV_IS_EMBEDDED
env_t *env_ptr = &environment;
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

extern int sata_curr_device;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

DECLARE_GLOBAL_DATA_PTR;

__weak int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset;

	offset = CONFIG_ENV_OFFSET;

	if (offset < 0)
		offset += mmc->capacity;

	*env_addr = offset;

	return 0;
}

__weak int mmc_get_env_devno(void)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_SYS_MMC_ENV_PART
__weak uint mmc_get_env_part(struct mmc *mmc)
{
	return CONFIG_SYS_MMC_ENV_PART;
}

static int mmc_set_env_part(struct mmc *mmc)
{
	uint part = mmc_get_env_part(mmc);
	int dev = mmc_get_env_devno();
	int ret = 0;

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	if (part != mmc->part_num) {
		ret = mmc_switch_part(dev, part);
		if (ret)
			puts("MMC partition switch failed\n");
	}

	return ret;
}
#else
static inline int mmc_set_env_part(struct mmc *mmc) {return 0; };
#endif

static int init_mmc_for_env(struct mmc *mmc)
{
	if (!mmc) {
		puts("No MMC card found\n");
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	return mmc_set_env_part(mmc);
}

static void fini_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	int dev = mmc_get_env_devno();

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif
	if (mmc_get_env_part(mmc) != mmc->part_num)
		mmc_switch_part(dev, mmc->part_num);
#endif
}

#ifdef CONFIG_CMD_SAVEENV
static inline int mmc_write_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt		= ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = mmc->block_dev.block_write(mmc_get_env_devno(), blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int mmc_saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int mmc_env_devno = mmc_get_env_devno();
	struct mmc *mmc = find_mmc_device(mmc_env_devno);
	u32	offset;
	int	ret, copy = 0;

	if (init_mmc_for_env(mmc))
		return 1;

	ret = env_export(env_new);
	if (ret)
		goto fini;

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sMMC(%d)... ", copy ? "redundant " : "",
	       mmc_env_devno);
	if (mmc_write_env(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	puts("done\n");
	ret = 0;

fini:
	fini_mmc_for_env(mmc);
	return ret;
}
#endif

static inline int mmc_read_env(struct mmc *mmc, unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	int dev = mmc_get_env_devno();

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	blk_start	= ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt		= ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(dev, blk_start, blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

void mmc_env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc;
	u32 offset;
	int ret;
	int dev = mmc_get_env_devno();

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	mmc = find_mmc_device(dev);

	if (init_mmc_for_env(mmc)) {
		ret = 1;
		goto err;
	}

	if (mmc_get_env_addr(mmc, 0, &offset)) {
		ret = 1;
		goto fini;
	}

	if (mmc_read_env(mmc, CONFIG_ENV_SIZE, offset, buf)) {
		ret = 1;
		goto fini;
	}

	env_import(buf, 1);
	ret = 0;

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		set_default_env(NULL);
#endif
}
//SATA env-------------------------------------------------------------------------

extern int sata_curr_device;

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

#ifdef CONFIG_CMD_SAVEENV

inline int sata_write_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_write(sata_curr_device, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int sata_saveenv(void)
{
	block_dev_desc_t *sata = NULL;
	env_t	env_new;
	ssize_t	len;
	char *res;

	if (sata_curr_device == -1) {
		if (sata_initialize())
			return 1;
		sata_curr_device = CONFIG_SATA_ENV_DEV;
	}

	if (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Unknown SATA(%d) device for environment!\n",
			sata_curr_device);
		return 1;
	}

	sata = sata_get_dev(sata_curr_device);

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	printf("Writing to SATA(%d)...", sata_curr_device);
	if (sata_write_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, &env_new)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

inline int sata_read_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_read(sata_curr_device, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

void sata_env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	block_dev_desc_t *sata = NULL;
	char buf[CONFIG_ENV_SIZE];
	int ret;

	if (sata_curr_device == -1) {
		if (sata_initialize())
			return;
		sata_curr_device = CONFIG_SATA_ENV_DEV;
	}

	if (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Unknown SATA(%d) device for environment!\n",
			sata_curr_device);
		return;
	}
	sata = sata_get_dev(sata_curr_device);

	if (sata_read_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf))
		return use_default();

	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = 1;

#endif
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts("*** Warning - bad CRC or MMC, using default environment\n\n");
	set_default_env(NULL);
}
#endif
//End of SATA env-------------------------------------------------------------------------

void env_relocate_spec(void)
{
	switch (get_boot_device()) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
	case SD4_BOOT:
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
	case MMC4_BOOT:
		printf( "Environment is in MMC.... \r\n");
		mmc_env_relocate_spec();
		break;
	case SATA_BOOT:
		printf( "Environment is in SATA.... \r\n");
		sata_env_relocate_spec();
		break;
	default:
		break;
	}	
}

int saveenv(void)
{
	switch (get_boot_device()) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
	case SD4_BOOT:
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
	case MMC4_BOOT:
		printf( "Save environment to MMC.... \r\n");
		mmc_saveenv();
		return 0;
		break;
	case SATA_BOOT:
		printf( "Save environment to SATA.... \r\n");
		sata_saveenv();
		return 0;
		break;
	default:
		return 0;
		break;
	}	
}


