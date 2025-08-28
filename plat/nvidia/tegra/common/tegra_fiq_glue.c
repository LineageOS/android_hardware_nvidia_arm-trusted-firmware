/*
 * Copyright (c) 2016-2024, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <bl31/ehf.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <denver.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#if ENABLE_WDT_LEGACY_FIQ_HANDLING
#include <flowctrl.h>
#endif
#include <tegra_def.h>
#include <tegra_private.h>

/* Legacy FIQ used by earlier Tegra platforms */
#define LEGACY_FIQ_PPI_WDT		28U

/*******************************************************************************
 * Static variables
 ******************************************************************************/
static uint32_t fiq_handler_active;

#pragma weak plat_fiq_handler
bool plat_fiq_handler(unsigned int id)
{
	return false;
}

/*******************************************************************************
 * Handler for FIQ interrupts
 ******************************************************************************/
static int tegra_fiq_interrupt_handler(unsigned int id, unsigned int flags,
		void *handle, void *cookie)
{
	(void)flags;
	(void)handle;
	(void)cookie;

	if (plat_fiq_handler(id)) {
		return 0U;
	}

#if ENABLE_WDT_LEGACY_FIQ_HANDLING
	/*
	 * Tegra platforms that use LEGACY_FIQ as the watchdog timer FIQ
	 * need to issue an IPI to other CPUs, to allow them to handle
	 * the "system hung" scenario. This interrupt is passed to the GICD
	 * via the Flow Controller. So, once we receive this interrupt,
	 * disable the routing so that we can mark it as "complete" in the
	 * GIC later.
	 */
	if (id == LEGACY_FIQ_PPI_WDT) {
		tegra_fc_disable_fiq_to_ccplex_routing();
	}
#endif

	/*
	 * Mark this interrupt as complete to avoid a FIQ storm.
	 */
	plat_ic_end_of_interrupt(id);

	return 0;
}

/*******************************************************************************
 * Setup handler for FIQ interrupts
 ******************************************************************************/
void tegra_fiq_handler_setup(void)
{
	/* return if already registered */
	if (fiq_handler_active == 0U) {
		/*
		 * Register an interrupt handler for FIQ interrupts generated for
		 * NS interrupt sources
		 */
		ehf_register_priority_handler(PLAT_TEGRA_WDT_PRIO, tegra_fiq_interrupt_handler);

		/* handler is now active */
		fiq_handler_active = 1;
	}
}
