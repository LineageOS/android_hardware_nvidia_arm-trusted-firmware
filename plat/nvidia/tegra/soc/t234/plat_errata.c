/*
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/cpus/errata.h>
#include <lib/mmio.h>
#include <tegra234_private.h>
#include <tegra_def.h>

/* -------------------------------------------------------
 * Errata Workaround for Cortex A78AE Erratum 2821985.
 * This applies to revisions applies to revisions <= r0p2
 * The workaround is to set bits SCR_EL3.API, SCR_EL3.APK
 * when DBGEN=1 and SPIDEN=0.
 * -------------------------------------------------------
 */

void tegra234_enable_errata_2821985_wa(void)
{
	uint32_t pmc_misc_val;
	uint64_t val;

	/* This applies to revisions <= r0p2. */
	val = cpu_get_rev_var();
	if (val > CPU_REV(0, 2)) {
		return;
	}

	/* check if DBGEN=1 and SPIDEN=0, apply errata 2821985 */
	pmc_misc_val = mmio_read_32(TEGRA_PMC_MISC_BASE + TEGRA_PMC_MISC_DEBUG_AUTHENTICATION_0);
	pmc_misc_val &= (TEGRA_PMC_MISC_DBG_AUTH_SPIDEN_BIT | TEGRA_PMC_MISC_DBG_AUTH_DBGEN_BIT);

	if (pmc_misc_val == TEGRA_PMC_MISC_DBG_AUTH_DBGEN_BIT) {
		/* Set SCR_EL3.API = 1, SCR_EL3.APK = 1 */
		val = read_scr();
		val |= (SCR_APK_BIT | SCR_API_BIT);
		write_scr(val);
	}
}

void tegra234_report_errata()
{
	uint64_t val;

	val = cpu_get_rev_var();

#if ERRATA_A78_AE_2821985
	/* report erratas apllied for rev <=2 */
        if (val <= CPU_REV(0, 2)) {
		 INFO("BL31: cortex_a78_ae: CPU workaround for 2821985 was applied\n");
	}
#endif

}
