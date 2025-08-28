/*
 * Copyright (c) 2019-2024, ARM Limited. All rights reserved.
 * Copyright (c) 2021-2024, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78_ae.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78_ae must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78_AE_BHB_LOOP_COUNT, cortex_a78_ae
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a78_ae, ERRATUM(1941500), ERRATA_A78_AE_1941500
	sysreg_bit_set CORTEX_A78_AE_CPUECTLR_EL1, CORTEX_A78_AE_CPUECTLR_EL1_BIT_8
workaround_reset_end cortex_a78_ae, ERRATUM(1941500)

check_erratum_ls cortex_a78_ae, ERRATUM(1941500), CPU_REV(0, 1)

workaround_reset_start cortex_a78_ae, ERRATUM(1951502), ERRATA_A78_AE_1951502
	msr	S3_6_c15_c8_0, xzr
	ldr	x0, =0x10E3900002
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #1
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #2
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0
workaround_reset_end cortex_a78_ae, ERRATUM(1951502)

check_erratum_ls cortex_a78_ae, ERRATUM(1951502), CPU_REV(0, 1)

workaround_reset_start cortex_a78_ae, ERRATUM(2376748), ERRATA_A78_AE_2376748
	/* -------------------------------------------------------
	 * Set CPUACTLR2_EL1[0] to 1 to force PLDW/PFRM ST to
	 * behave like PLD/PRFM LD and not cause invalidations to
	 * other PE caches. There might be a small performance
	 * degradation to this workaround for certain workloads
	 * that share data.
	 * -------------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A78_AE_ACTLR2_EL1, CORTEX_A78_AE_ACTLR2_EL1_BIT_0
workaround_reset_end cortex_a78_ae, ERRATUM(2376748)

check_erratum_ls cortex_a78_ae, ERRATUM(2376748), CPU_REV(0, 2)

workaround_reset_start cortex_a78_ae, ERRATUM(2395408), ERRATA_A78_AE_2395408
	/* --------------------------------------------------------
	 * Disable folding of demand requests into older prefetches
	 * with L2 miss requests outstanding by setting the
	 * CPUACTLR2_EL1[40] to 1.
	 * --------------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A78_AE_ACTLR2_EL1, CORTEX_A78_AE_ACTLR2_EL1_BIT_40
workaround_reset_end cortex_a78_ae, ERRATUM(2395408)

check_erratum_ls cortex_a78_ae, ERRATUM(2395408), CPU_REV(0, 1)

workaround_reset_start cortex_a78_ae, ERRATUM(2466780), ERRATA_A78_AE_2466780
	/* flush the mop cache */
	mov_imm	x0, 0x3
	msr	CPUPSELR_EL3, x0
	mov_imm	x0, 0xEE010F10
	msr	CPUPOR_EL3, x0
	mov_imm	x0, 0xFF1F0FFE
	msr	CPUPMR_EL3, x0
	mov_imm	x0, 0x100000004003FF
	msr	CPUPCR_EL3,x0
	isb
workaround_reset_end cortex_a78_ae, ERRATUM(2466780)

check_erratum_range cortex_a78_ae, ERRATUM(2466780), CPU_REV(0, 0), CPU_REV(0, 2)

workaround_reset_start cortex_a78_ae, ERRATUM(2743229), ERRATA_A78_AE_2743229
	/* Set bit [56:55]=b'01 in CPUACTLR5 */
	sysreg_bit_set CORTEX_A78_AE_ACTLR5_EL1, CORTEX_A78_AE_ACTLR5_EL1_BIT_55
	sysreg_bit_clear CORTEX_A78_AE_ACTLR5_EL1, CORTEX_A78_AE_ACTLR5_EL1_BIT_56
workaround_reset_end cortex_a78_ae, ERRATUM(2743229)

check_erratum_range cortex_a78_ae, ERRATUM(2743229), CPU_REV(0, 0), CPU_REV(0, 2)

workaround_runtime_start cortex_a78_ae, ERRATUM(2743093), ERRATA_A78_AE_2743093
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a78_ae, ERRATUM(2743093)

check_erratum_ls cortex_a78_ae, ERRATUM(2743093), CPU_REV(0, 2)

workaround_reset_start cortex_a78_ae, ERRATUM(2855379), ERRATA_A78_AE_2855379
	/* set CPUACTLR5_EL1[39] */
	sysreg_bit_set CORTEX_A78_AE_ACTLR5_EL1, CORTEX_A78_AE_ACTLR5_EL1_BIT_39
workaround_reset_end cortex_a78_ae, ERRATUM(2855379)

check_erratum_ls cortex_a78_ae, ERRATUM(2855379), CPU_REV(0, 1)

workaround_reset_start cortex_a78_ae, ERRATUM(3604886), ERRATA_A78_AE_3604886
	sysreg_bit_set CORTEX_A78_AE_CPUACTLR3_EL1, CORTEX_A78_AE_CPUACTLR3_EL1_BIT_58
workaround_reset_end cortex_a78_ae, ERRATUM(3604886)

check_erratum_range cortex_a78_ae, ERRATUM(3604886), CPU_REV(0, 0), CPU_REV(0, 3)

workaround_reset_start cortex_a78_ae, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A78AE generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a78_ae
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a78_ae, CVE(2022, 23960)

check_erratum_chosen cortex_a78_ae, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_a78ae, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set CORTEX_A78_CPUECTLR_EL1, CORTEX_A78_CPUECTLR_EL1_BIT_46
workaround_reset_end cortex_a78ae, CVE(2024, 5660)

check_erratum_chosen cortex_a78ae, CVE(2024, 5660), WORKAROUND_CVE_2024_5660

cpu_reset_func_start cortex_a78_ae
#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_clear actlr_el3, CORTEX_A78_ACTLR_TAM_BIT

	/* Make sure accesses from non-secure EL0/EL1 are not trapped to EL2 */
	sysreg_bit_clear actlr_el2, CORTEX_A78_ACTLR_TAM_BIT

	/* Enable group0 counters */
	mov	x0, #CORTEX_A78_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET0_EL0, x0
#endif
cpu_reset_func_end cortex_a78_ae

	/* -------------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * -------------------------------------------------------
	 */
func cortex_a78_ae_core_pwr_dwn
	/* -------------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * -------------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A78_CPUPWRCTLR_EL1, CORTEX_A78_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT

	apply_erratum cortex_a78_ae, ERRATUM(2743093), ERRATA_A78_AE_2743093

	isb
	ret
endfunc cortex_a78_ae_core_pwr_dwn

	/* -------------------------------------------------------
	 * This function provides cortex_a78_ae specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * -------------------------------------------------------
	 */
.section .rodata.cortex_a78_ae_regs, "aS"
cortex_a78_ae_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78_ae_cpu_reg_dump
	adr	x6, cortex_a78_ae_regs
	mrs	x8, CORTEX_A78_CPUECTLR_EL1
	ret
endfunc cortex_a78_ae_cpu_reg_dump

declare_cpu_ops cortex_a78_ae, CORTEX_A78_AE_MIDR, \
	cortex_a78_ae_reset_func, \
	cortex_a78_ae_core_pwr_dwn
