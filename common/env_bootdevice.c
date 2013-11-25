/*
 * (C) Copyright 2010-2011 Freescale Semiconductor, Inc.
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
#include <nand.h>
#include <search.h>
#include <errno.h>
#include <asm/arch/sys_proto.h>


#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

#ifndef CONFIG_ENV_RANGE
#define CONFIG_ENV_RANGE	CONFIG_ENV_NAND_SIZE
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


#if !defined(CONFIG_ENV_NAND_OFFSET)
#define CONFIG_ENV_NAND_OFFSET 0
#endif


DECLARE_GLOBAL_DATA_PTR;

static int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset;

	offset = CONFIG_ENV_OFFSET;

	if (offset < 0)
		offset += mmc->capacity;

	*env_addr = offset;

	return 0;
}

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

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num) {
		if (mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV,
				    CONFIG_SYS_MMC_ENV_PART)) {
			puts("MMC partition switch failed\n");
			return -1;
		}
	}
#endif

	return 0;
}

static void fini_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num)
		mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV,
				mmc->part_num);
#endif
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

static int sata_write_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_write(sata_curr_device, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int mmc_write_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt		= ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = mmc->block_dev.block_write(CONFIG_SYS_MMC_ENV_DEV, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
int nand_writeenv(size_t offset, u_char *buf)
{
	size_t end = offset + CONFIG_ENV_RANGE;
	size_t amount_saved = 0;
	size_t blocksize, len;
	u_char *char_ptr;

	blocksize = nand_info[0].erasesize;
	len = min(blocksize, CONFIG_ENV_NAND_SIZE);
	
	while (amount_saved < CONFIG_ENV_NAND_SIZE && offset < end) {
		if (nand_block_isbad(&nand_info[0], offset)) {
			offset += blocksize;
		} else {
			char_ptr = &buf[amount_saved];
			if (nand_write(&nand_info[0], offset, &len, char_ptr))
				return 1;

			offset += blocksize;
			amount_saved += len;
		}
	}
	if (amount_saved != CONFIG_ENV_NAND_SIZE)
		return 1;

	return 0;
}

struct env_location {
	const char *name;
	const nand_erase_options_t erase_opts;
};

static int nand_erase_and_write_env(const struct env_location *location,
		u_char *env_new)
{
	int ret = 0;

	printf("Erasing %s...\n", location->name);
	if (nand_erase_opts(&nand_info[0], &location->erase_opts))
		return 1;

	printf("Writing to %s... ", location->name);
	ret = nand_writeenv(location->erase_opts.offset, env_new);
	puts(ret ? "FAILED!\n" : "OK\n");

	return ret;
}

int sata_saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ssize_t	len;
	char	*res;
	struct block_dev_desc_t *sata = NULL;

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
	res = (char *)&env_new->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}

	env_new->crc = crc32(0, &env_new->data[0], ENV_SIZE);	

	sata = sata_get_dev(sata_curr_device);

	printf("Writing to SATA(%d)... ", sata_curr_device);
	if (sata_write_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, (u_char *)env_new)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");

	return 0;
}

int mmc_saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ssize_t	len;
	char	*res;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	u32	offset;
	int	ret, copy = 0;

	if (init_mmc_for_env(mmc))
		return 1;

	res = (char *)&env_new->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		ret = 1;
		goto fini;
	}

	env_new->crc = crc32(0, &env_new->data[0], ENV_SIZE);

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sMMC(%d)... ", copy ? "redundant " : "",
	       CONFIG_SYS_MMC_ENV_DEV);
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

int nand_saveenv(void)
{
	int	ret = 0;
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ssize_t	len;
	char	*res;
	int	env_idx = 0;
	static const struct env_location location[] = {
		{
			.name = "NAND",
			.erase_opts = {
				.length = CONFIG_ENV_RANGE,
				.offset = CONFIG_ENV_NAND_OFFSET,
			},
		},
	};


	if (CONFIG_ENV_RANGE < CONFIG_ENV_NAND_SIZE)
		return 1;

	res = (char *)&env_new->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new->crc   = crc32(0, env_new->data, ENV_SIZE);

	ret = nand_erase_and_write_env(&location[env_idx], (u_char *)env_new);

	return ret;
}

static int sata_read_env(block_dev_desc_t *sata, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = sata->block_read(sata_curr_device, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static inline int mmc_read_env(struct mmc *mmc, unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt		= ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int nand_readenv(size_t offset, u_char *buf)
{
	size_t end = offset + CONFIG_ENV_RANGE;
	size_t amount_loaded = 0;
	size_t blocksize, len;
	u_char *char_ptr;

	blocksize = nand_info[0].erasesize;
	if (!blocksize)
		return 1;

	len = min(blocksize, CONFIG_ENV_NAND_SIZE);

	while (amount_loaded < CONFIG_ENV_NAND_SIZE && offset < end) {
		if (nand_block_isbad(&nand_info[0], offset)) {
			offset += blocksize;
		} else {
			char_ptr = &buf[amount_loaded];
			if (nand_read_skip_bad(&nand_info[0], offset,
					       &len, NULL,
					       nand_info[0].size, char_ptr))
				return 1;

			offset += blocksize;
			amount_loaded += len;
		}
	}

	if (amount_loaded != CONFIG_ENV_NAND_SIZE)
		return 1;

	return 0;
}

static void sata_env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct block_dev_desc_t *sata = NULL;
	int ret = 0;

	if (sata_curr_device == -1) {
		if (sata_initialize()) {
			ret = 1;
			goto err;
		}
		sata_curr_device = CONFIG_SATA_ENV_DEV;
	}

	if (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Unknown SATA(%d) device for environment!\n",
			sata_curr_device);
		ret = 1;
		goto err;
	}
	sata = sata_get_dev(sata_curr_device);

	if (sata_read_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf)){
		printf( "\r\n--->Read environment failed \r\n");
		ret = 1;
		goto err;
	}

	gd->env_valid = 1;
	env_import(buf, 1);
	ret = 0;
err:
	if (ret)
		set_default_env(NULL);
#endif
}

void mmc_env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	u32 offset;
	int ret;

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

/*
 * The legacy NAND code saved the environment in the first NAND
 * device i.e., nand_dev_desc + 0. This is also the behaviour using
 * the new NAND code.
 */
void nand_env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_NAND_SIZE);


	ret = nand_readenv(CONFIG_ENV_NAND_OFFSET, (u_char *)buf);
	if (ret) {
		set_default_env("!readenv() failed");
		return;
	}

	env_import(buf, 1);
#endif /* ! ENV_IS_EMBEDDED */
}

void env_relocate_spec(void)
{
	switch (get_boot_device()) {
	case MX6_SD0_BOOT:
	case MX6_SD1_BOOT:
	case MX6_MMC_BOOT:
		printf( "Environment is in MMC.... \r\n");
		mmc_env_relocate_spec();
		setenv("bootmode", "MMC");
		break;
	case MX6_SATA_BOOT:
		printf( "Environment is in SATA.... \r\n");
		sata_env_relocate_spec();
		setenv("bootmode", "SATA");
		break;
	case MX6_NAND_BOOT:
		printf( "Environment is in NAND.... \r\n");
		nand_env_relocate_spec();
		setenv("bootmode", "NAND");
		break;
	default:
		break;
	}	
}

int saveenv(void)
{
	switch (get_boot_device()) {
	case MX6_SD0_BOOT:
	case MX6_SD1_BOOT:
	case MX6_MMC_BOOT:
		printf( "Save environment to MMC.... \r\n");
		mmc_saveenv();
		return 0;
		break;
	case MX6_SATA_BOOT:
		printf( "Save environment to SATA.... \r\n");
		sata_saveenv();
		return 0;
		break;
	case MX6_NAND_BOOT:
		printf( "Save environment to NAND.... \r\n");
		nand_saveenv();
		setenv("bootmode", "NAND");
		break;
	default:
		return 0;
		break;
	}	
}


