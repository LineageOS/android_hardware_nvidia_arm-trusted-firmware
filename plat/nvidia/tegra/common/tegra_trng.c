/*
 * Copyright (c) 2024-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/smccc.h>

#include <nvrng.h>
#include <plat/common/platform.h>
#include <tegra_def.h>
#include <tegra_platform.h>

/* Global variables */
static bool nvrng_initialized;
uuid_t plat_trng_uuid;

DEFINE_SVC_UUID2(_plat_trng_uuid,
	0xa60040f2, 0xe8d9, 0x4f3e, 0x81, 0x2f,
	0x23, 0xa3, 0xc6, 0x95, 0x09, 0xbc
);

/*
 * @param out the address of the variable to write entropy bits to
 * @return true if entropy bits are successfully written, false if not
 */
bool plat_get_entropy(uint64_t *out)
{
	/* Sanity check before proceeding */
	if (!nvrng_initialized || (out == NULL)) {
		return false;
	}

	tegra_nvrng_get_entropy(out);

	return true;
}

/**
 * This function is called only once during the boot up
 */
void plat_entropy_setup(void)
{
	/* initialize nvrng HW */
	uint8_t ret = tegra_nvrng_init();

	if (ret == 0) {
		plat_trng_uuid = _plat_trng_uuid;
		nvrng_initialized = true;
		INFO("TRNG entropy setup completed\n");
	} else {
		WARN("TRNG entropy setup failure\n");
	}
}
