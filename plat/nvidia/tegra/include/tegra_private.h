/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PRIVATE_H
#define TEGRA_PRIVATE_H

#include <platform_def.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <tegra_gic.h>

/*******************************************************************************
 * Implementation defined ACTLR_EL1 bit definitions
 ******************************************************************************/
#define ACTLR_EL1_PMSTATE_MASK		(ULL(0xF) << 0)

/*******************************************************************************
 * Implementation defined ACTLR_EL2 bit definitions
 ******************************************************************************/
#define ACTLR_EL2_PMSTATE_MASK		(ULL(0xF) << 0)

/*******************************************************************************
 * Remote chip address offset
 ******************************************************************************/
#ifndef TEGRA_REMOTE_CHIP_MEM_OFFSET
#define TEGRA_REMOTE_CHIP_MEM_OFFSET(n)	U(0)
#endif

/*******************************************************************************
 * Struct for parameters received from BL2
 ******************************************************************************/
typedef struct bl31_plat_params_struct {
	/* TZ memory size */
	uint64_t tzdram_size;
	/* TZ memory base */
	uint64_t tzdram_base;
	/* UART port ID */
	int32_t uart_id;
	/* L2 ECC parity protection disable flag */
	int32_t l2_ecc_parity_prot_dis;
	/* SHMEM base address for storing the boot logs */
	uint64_t boot_profiler_shmem_base;
	/* System Suspend Entry Firmware size */
	uint64_t sc7entry_fw_size;
	/* System Suspend Entry Firmware base address */
	uint64_t sc7entry_fw_base;
	/* Enable dual execution */
	uint8_t enable_ccplex_lock_step;
	/** Padding to align the next field at 4B boundary */
	uint8_t reserved_1[3];
	/* MPID for SatMc reserved core */
	uint32_t reserved_core_mpid;
	/** TRNG pool base address in the TZRAM */
	uint64_t trng_pool_base;
	/** Number of TRNG entries in the pool */
	uint32_t trng_pool_entries;
	/* Size of the RAS_FW Communication buffer in bytes */
	uint32_t th500_ras_fw_comm_size;
	/* Base address of the RAS_FW Communication buffer */
	uint64_t th500_ras_fw_comm_base;
	/* Logging level for the console prints */
	uint32_t log_level;
	/* CPU performance knobs */
	uint32_t cpu_perf_knobs;
	/* Size of the size of the GPT memory in bytes */
	uint64_t gpt_memory_size;
	/* RAS CE error threshold vale */
	uint64_t cpu_ras_ce_threshold;
} bl31_plat_params_t;

/*******************************************************************************
 * Helper function to access l2ctlr_el1 register on Cortex-A57 CPUs
 ******************************************************************************/
DEFINE_RENAME_SYSREG_RW_FUNCS(l2ctlr_el1, CORTEX_A57_L2CTLR_EL1)

/*******************************************************************************
 * Struct describing parameters passed to bl31
 ******************************************************************************/
struct tegra_bl31_params {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
       entry_point_info_t *rmm_ep_info;
       image_info_t *rmm_image_info;
};

/*******************************************************************************
* To suppress Coverity MISRA C-2012 Rule 2.2 violations
*******************************************************************************/
#define UNUSED_FUNC_NOP()	asm("nop")

/*******************************************************************************
* Invalid MPID value for SatMc reserved core
*******************************************************************************/
#define INVALID_RSVD_MPID_VAL		U(0x7F0000)

/* Declarations for plat_psci_handlers.c */
int32_t tegra_soc_validate_power_state(uint32_t power_state,
		psci_power_state_t *req_state);

/* Declarations for plat_setup.c */
const mmap_region_t *plat_get_mmio_map(void);
void plat_enable_console(int32_t id);
void plat_gic_setup(void);
struct tegra_bl31_params *plat_get_bl31_params(void);
bl31_plat_params_t *plat_get_bl31_plat_params(void);
void plat_early_platform_setup(void);
void plat_late_platform_setup(void);
int32_t plat_parse_bl31_plat_params(bl31_plat_params_t *plat_bl31_params,
					void *params_from_bl2);
void plat_relocate_bl32_image(const image_info_t *bl32_img_info);
bool plat_supports_system_suspend(void);
void plat_runtime_setup(void);
int plat_bl31_manifest_parse(const void *fdt, bl31_plat_params_t *params);

/* Declarations for plat_secondary.c */
bool plat_is_secondary_present(u_register_t mpidr);
void plat_secondary_setup(void);
int32_t plat_lock_cpu_vectors(void);

/* Declarations for tegra_fiq_glue.c */
void tegra_fiq_handler_setup(void);

/* Declarations for tegra_helpers.S */
bool plat_is_my_cpu_primary(void);

/* Declarations for tegra_security.c */
void tegra_security_setup(void);
void tegra_security_setup_videomem(uintptr_t base, uint64_t size);

/* Declarations for tegra_pm.c */
void tegra_pm_system_suspend_entry(void);
void tegra_pm_system_suspend_exit(void);
int32_t tegra_system_suspended(void);
int32_t tegra_soc_cpu_standby(plat_local_state_t cpu_state);
void tegra_soc_cpu_standby_exit(plat_local_state_t cpu_state);
int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_on(u_register_t mpidr);
int32_t tegra_soc_pwr_domain_off_early(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_off(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state);
int32_t tegra_soc_prepare_system_reset(void);
__dead2 void tegra_soc_prepare_system_off(void);
plat_local_state_t tegra_soc_get_target_pwr_state(uint32_t lvl,
					     const plat_local_state_t *states,
					     uint32_t ncpu);
int32_t tegra_soc_validate_ns_entrypoint(uintptr_t entrypoint);
#ifdef ENABLE_FEAT_RAS
void tegra_ras_dump_core_records(void);
#endif

/* Declarations for tegraXXX_pm.c */
int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl);
int tegra_prepare_cpu_on_finish(unsigned long mpidr);

/* Declarations for tegra_bl31_setup.c */
bl31_plat_params_t *bl31_get_plat_params(void);
int32_t bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes);

/* Declarations for tegra_delay_timer.c */
void tegra_delay_timer_init(void);

void tegra_secure_entrypoint(void);

/* Declarations for tegra_sip_calls.c */
uintptr_t tegra_sip_handler(uint32_t smc_fid,
			    u_register_t x1,
			    u_register_t x2,
			    u_register_t x3,
			    u_register_t x4,
			    void *cookie,
			    void *handle,
			    u_register_t flags);
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags);

/* platform specific interrupt handler */
bool plat_fiq_handler(uint32_t irq_num);

#if ENABLE_FEAT_RAS && FFH_SUPPORT
void tegra194_ras_enable(void);
void tegra194_ras_corrected_err_clear(uint64_t *cookie);
#endif

#endif /* TEGRA_PRIVATE_H */
