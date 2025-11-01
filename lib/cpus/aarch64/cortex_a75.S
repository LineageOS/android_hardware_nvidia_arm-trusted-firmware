/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cpu_macros.S>

func cortex_a75_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A75_764081
	mov	x0, x18
	bl	errata_a75_764081_wa
#endif

#if ERRATA_A75_790748
	mov	x0, x18
	bl	errata_a75_790748_wa
#endif

#if IMAGE_BL31 && (WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960)
	cpu_check_csv2	x0, 1f
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	msr	vbar_el3, x0
	isb
	/* Skip installing vector table again for CVE_2022_23960 */
        b       2f
1:
#if WORKAROUND_CVE_2022_23960
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	msr	vbar_el3, x0
	isb
#endif
2:
#endif /* IMAGE_BL31 &&  (WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960) */

#if WORKAROUND_CVE_2018_3639
	mrs	x0, CORTEX_A75_ACTLR_EL1
	orr	x0, x0, #CORTEX_A75_ACTLR_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A75_ACTLR_EL1, x0
	isb
#endif

	ret
endfunc cortex_a75_reset_func

func check_errata_cve_2017_5715
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #ID_AA64PFR0_CSV2_SHIFT, #ID_AA64PFR0_CSV2_LENGTH
	/*
	 * If the field equals to 1 then branch targets trained in one
	 * context cannot affect speculative execution in a different context.
	 */
	cmp	x0, #1
	beq	1f

#if WORKAROUND_CVE_2017_5715
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
1:
	mov	x0, #ERRATA_NOT_APPLIES
	ret
endfunc check_errata_cve_2017_5715

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2018_3639

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960
	cpu_check_csv2	x0, 1f
	mov	x0, #ERRATA_APPLIES
	ret
1:
# if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
# else
	mov	x0, #ERRATA_MISSING
# endif /* WORKAROUND_CVE_2022_23960 */
	ret
#endif /* WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960 */
	mov	x0, #ERRATA_MISSING
	ret
endfunc check_errata_cve_2022_23960

func check_smccc_arch_workaround_3
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_smccc_arch_workaround_3

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a75_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A75_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A75_CORE_PWRDN_EN_MASK
	msr	CORTEX_A75_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a75_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A75. Must follow AAPCS.
 */
func cortex_a75_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2017_5715, cortex_a75, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, cortex_a75, cve_2018_3639
	report_errata ERRATA_DSU_798953, cortex_a75, dsu_798953
	report_errata ERRATA_DSU_936184, cortex_a75, dsu_936184
	report_errata WORKAROUND_CVE_2022_23960, cortex_a75, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a75_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a75 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a75_regs, "aS"
cortex_a75_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a75_cpu_reg_dump
	adr	x6, cortex_a75_regs
	mrs	x8, CORTEX_A75_CPUECTLR_EL1
	ret
endfunc cortex_a75_cpu_reg_dump

declare_cpu_ops_wa cortex_a75, CORTEX_A75_MIDR, \
	cortex_a75_reset_func, \
	check_errata_cve_2017_5715, \
	CPU_NO_EXTRA2_FUNC, \
	check_smccc_arch_workaround_3, \
	cortex_a75_core_pwr_dwn
