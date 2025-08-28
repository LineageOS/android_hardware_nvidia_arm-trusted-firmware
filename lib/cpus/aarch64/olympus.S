/*
 * Copyright (c) 2024-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <olympus.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Olympus must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Olympus supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table OLYMPUS_BHB_LOOP_COUNT, olympus
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start olympus, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Olympus generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_olympus

#endif /* IMAGE_BL31 */
workaround_reset_end olympus, CVE(2022,23960)

check_erratum_chosen olympus, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start olympus, CVE(2024,7881), WORKAROUND_CVE_2024_7881
	sysreg_bit_set OLYMPUS_CPUACTLR6_EL1, OLYMPUS_CPUACTLR6_EL1_BIT_41
workaround_reset_end olympus, CVE(2024,7881)

check_erratum_chosen olympus, CVE(2024,7881), WORKAROUND_CVE_2024_7881

workaround_reset_start olympus, ERRATUM(5167426), ERRATA_OLYMPUS_5167426
	/* Perform sysreg programming to enable L3 Prefetching */
	sysreg_bit_set OLYMPUS_NV_CPUACTLR2_EL1, OLYMPUS_NV_CPUACTLR2_BIT_57
	sysreg_bit_set OLYMPUS_NV_CPUACTLR2_EL1, OLYMPUS_NV_CPUACTLR2_BIT_56
	sysreg_bit_set OLYMPUS_NV_CPUACTLR2_EL1, OLYMPUS_NV_CPUACTLR2_BIT_55
	sysreg_bit_set OLYMPUS_NV_CPUACTLR2_EL1, OLYMPUS_NV_CPUACTLR2_BIT_18
	sysreg_bit_set OLYMPUS_NV_CPUACTLR2_EL1, OLYMPUS_NV_CPUACTLR2_BIT_0

	sysreg_bit_set OLYMPUS_NV_CPUACTLR6_EL1, OLYMPUS_NV_CPUACTLR6_BIT_63
	sysreg_bit_set OLYMPUS_NV_CPUACTLR6_EL1, OLYMPUS_NV_CPUACTLR6_BIT_62
	sysreg_bit_set OLYMPUS_NV_CPUACTLR6_EL1, OLYMPUS_NV_CPUACTLR6_BIT_61

	sysreg_bit_set OLYMPUS_NV_CPUACTLR7_EL1, OLYMPUS_NV_CPUACTLR7_BIT_18

	sysreg_bit_set OLYMPUS_NV_CPUACTLR22_EL1, OLYMPUS_NV_CPUACTLR22_BIT_45
	sysreg_bit_set OLYMPUS_NV_CPUACTLR22_EL1, OLYMPUS_NV_CPUACTLR22_BIT_46
	sysreg_bit_set OLYMPUS_NV_CPUACTLR22_EL1, OLYMPUS_NV_CPUACTLR22_BIT_47
workaround_reset_end olympus, ERRATUM(5167426)

check_erratum_chosen olympus, ERRATUM(5167426), ERRATA_OLYMPUS_5167426

workaround_reset_start olympus, ERRATUM(5258219), ERRATA_OLYMPUS_5258219
	sysreg_bit_set OLYMPUS_CPUECTLR_EL1, OLYMPUS_CPUECTLR_EL1_BIT_40
	sysreg_bit_set OLYMPUS_CPUECTLR_EL1, OLYMPUS_CPUECTLR_EL1_BIT_41
	sysreg_bit_set OLYMPUS_CPUECTLR_EL1, OLYMPUS_CPUECTLR_EL1_BIT_38
	sysreg_bit_set OLYMPUS_CPUECTLR_EL1, OLYMPUS_CPUECTLR_EL1_BIT_37
	sysreg_bit_set OLYMPUS_CPUECTLR_EL1, OLYMPUS_CPUECTLR_EL1_BIT_36
workaround_reset_end olympus, ERRATUM(5258219)

check_erratum_chosen olympus, ERRATUM(5258219), ERRATA_OLYMPUS_5258219

cpu_reset_func_start olympus
	/* Disable speculative loads */
	msr	SSBS, xzr

	/* Enable ED in ERXCTLR_EL1 */
	sysreg_bit_set ERXCTLR_EL1, ERXCTLR_ED_BIT

	/* Clear CORE_PWRDN_EN bit from IMP_CPUPWRCTLR_EL1 */
	sysreg_bit_clear OLYMPUS_CPUPWRCTLR_EL1, \
		OLYMPUS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	/* all SPR programming is complete unblock secondary PE */
	sysreg_bit_clear OLYMPUS_NV_CPUACTLR28_EL1, \
		OLYMPUS_NV_CPUACTLR28_EL1_BLOCK_SECONDARY_PE_BIT
cpu_reset_func_end olympus

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func olympus_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set OLYMPUS_CPUPWRCTLR_EL1, \
		OLYMPUS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc olympus_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Olympus specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.olympus_regs, "aS"
olympus_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func olympus_cpu_reg_dump
	adr	x6, olympus_regs
	mrs	x8, OLYMPUS_CPUECTLR_EL1
	ret
endfunc olympus_cpu_reg_dump

declare_cpu_ops olympus, OLYMPUS_MIDR, \
	olympus_reset_func, \
	olympus_core_pwr_dwn
