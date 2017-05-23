/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived work from spl_usb.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <sata.h>
#include <scsi.h>
#include <errno.h>
#include <fat.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

static int sata_curr_device = -1;
block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];

static unsigned long sata_bread(block_dev_desc_t *block_dev, lbaint_t start,
				lbaint_t blkcnt, void *dst)
{
	return sata_read(block_dev->dev, start, blkcnt, dst);
}

static unsigned long sata_bwrite(block_dev_desc_t *block_dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	return sata_write(block_dev->dev, start, blkcnt, buffer);
}

int spl_sata_load_image(void)
{
	int err;
	block_dev_desc_t *stor_dev;

	memset(&sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE], 0, sizeof(struct block_dev_desc));
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].if_type = IF_TYPE_SATA;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].dev = 0;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].part_type = PART_TYPE_UNKNOWN;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].type = DEV_TYPE_HARDDISK;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].lba = 0;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].blksz = 512;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].log2blksz = LOG2(sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].blksz);
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].block_read = sata_bread;
	sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].block_write = sata_bwrite;

	err = init_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: sata init failed: err - %d\n", err);
#endif
		return err;
	} else {
		err = scan_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
		if (!err && (sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].lba > 0) &&
			(sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE].blksz > 0))
			init_part(&sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE]);

		stor_dev = &sata_dev_desc[CONFIG_SPL_SATA_BOOT_DEVICE];
		if (!stor_dev)
			return -ENODEV;
	}

	sata_curr_device = CONFIG_SPL_SATA_BOOT_DEVICE;

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || spl_load_image_fat_os(stor_dev,
									CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	err = spl_load_image_fat(stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	if (err) {
		puts("Error loading sata device\n");
		return err;
	}

	return 0;
}
