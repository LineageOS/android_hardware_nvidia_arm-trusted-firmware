/*
 * Copyright (c) 2020-2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA234_PRIVATE
#define TEGRA234_PRIVATE

#define CARVEOUT_OEM_COUNT		U(73)

struct tegra234_carveout_info {
	uint64_t base;
	uint64_t size;
	union {
		struct {
			uint64_t ecc_protected:1;
			uint64_t reserved:63;
		};
		uint64_t flags;
	};
};

typedef struct t234_bl31_plat_params {
	uint64_t tzdram_size;
	uint64_t tzdram_base;
	uint64_t boot_profiler_shmem_base;
	uint64_t sc7entry_fw_size;
	uint64_t sc7entry_fw_base;
	uint32_t uart_port_id;;
	uint8_t  enable_ccplex_lock_step;
	uint64_t trng_pool_base;
	uint32_t trng_pool_entries;
	uint32_t reserved_1;
	union {
		uint64_t data;
		struct {
			uint64_t disable_cpu_l2ecc:1;
			uint64_t enable_sc7_dram_auth:1;
			uint64_t reserved_2:62;
		};
	};
	uint64_t dram_page_retirement_address;
	struct tegra234_carveout_info carveout_info[CARVEOUT_OEM_COUNT];
} t234_bl31_plat_params_t;

void tegra234_enable_errata_2821985_wa(void);
void tegra234_report_errata(void);
void tegra234_ras_init_common(void);
void tegra234_ras_init_my_cluster(void);
int tegra234_ras_inject_fault(uint64_t base, uint64_t pfgcdn, uint64_t pfgctl);

#endif /* TEGRA234_PRIVATE */
