/*
 * Copyright (c) 2013 Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

extern void zynq_slcr_lock(void);
extern void zynq_slcr_unlock(void);
extern void zynq_slcr_cpu_reset(void);
extern void zynq_slcr_gem_clk_setup(u32 gem_id, u32 rclk, u32 clk);
extern void zynq_slcr_devcfg_disable(void);
extern void zynq_slcr_devcfg_enable(void);
extern u32 zynq_slcr_get_idcode(void);
extern void zynq_ddrc_init(void);

/* Driver extern functions */
extern int zynq_sdhci_init(u32 regbase);

#endif /* _SYS_PROTO_H_ */
