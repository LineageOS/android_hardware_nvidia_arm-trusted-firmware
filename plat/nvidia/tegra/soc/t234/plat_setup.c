/*
 * Copyright (c) 2021-2025, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl31/bl31.h>
#include <bpmp_ipc.h>
#include <common/bl_common.h>
#include <drivers/console.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <cortex_a57.h>
#include <common/debug.h>
#include <denver.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic600ae_fmu.h>
#include <bl31/interrupt_mgmt.h>
#include <mce.h>
#include <mce_private.h>
#include <memctrl.h>
#include <plat/common/platform.h>
#include <profiler.h>
#include <spe.h>
#include <tegra234_private.h>
#include <tegra_def.h>
#include <tegra_mc_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/* ID for spe-console */
#define TEGRA_CONSOLE_SPE_ID		0xFE
/* MCE aperture start address */
#define TEGRA_MCE_ARI0_BASE		TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_0_OFFSET

/*******************************************************************************
 * The Tegra power domain tree has a single system level power domain i.e. a
 * single root node. The first entry in the power domain descriptor specifies
 * the number of power domains at the highest power level.
 *******************************************************************************
 */
static const uint8_t tegra_power_domain_tree_desc[] = {
	/* No of root nodes */
	1,
	/* No of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* No of CPU cores - cluster0 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster1 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster2 */
	PLATFORM_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const uint8_t *plat_get_power_domain_tree_desc(void)
{
	return tegra_power_domain_tree_desc;
}

/*
 * Table of regions to map using the MMU.
 */
static const mmap_region_t tegra_mmap[] = {
	MAP_REGION_FLAT(TEGRA_MISC_BASE, 0x4000U, /* 16KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_TSA_BASE, 0x20000U, /* 128KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GPCDMA_BASE, 0x10000U, /* 64KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_STREAMID_BASE, 0x8000U, /* 32KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_BASE, 0x8000U, /* 32KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#if !ENABLE_CONSOLE_SPE
	MAP_REGION_FLAT(TEGRA_UARTA_BASE, 0x20000U, /* 128KB - UART A, B*/
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTD_BASE, 0x30000U, /* 192KB - UART D, E, F */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
	MAP_REGION_FLAT(TEGRA_SE0_BASE, 0x1000U,  /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PKA1_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_FUSE_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RNG1_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_HSP_DBELL_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#if ERRATA_A78_AE_2821985
	MAP_REGION_FLAT(TEGRA_PMC_MISC_BASE, 0x10000U, /* 64KB */
                        (uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
#if ENABLE_CONSOLE_SPE
	MAP_REGION_FLAT(TEGRA_CONSOLE_SPE_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
#if !ENABLE_CONSOLE_SPE
	MAP_REGION_FLAT(TEGRA_UARTC_BASE, 0x20000U, /* 128KB - UART C, G */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
	MAP_REGION_FLAT(TEGRA_SCRATCH_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_AON_FABRIC_FIREWALL_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_TMRUS_BASE, TEGRA_TMRUS_SIZE, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MCE_ARI0_BASE, 0xc0000U, /* 64KB*12 */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICD_BASE, 0x10000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICR_BASE, 0x200000U, /* 2MB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#if GICV3_SUPPORT_GIC600AE_FMU
	MAP_REGION_FLAT(TEGRA_GICFMU_BASE, 0x10000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
	MAP_REGION_FLAT(TEGRA_CAR_RESET_BASE, 0x10000U, /* 64KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_CCPMU_RAS_BASE, 0x1D000U, /* 116KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RAS_CL0_CORE0_BASE, 0x9000U, /* 36KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RAS_CL1_CORE0_BASE, 0x9000U, /* 36KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RAS_CL2_CORE0_BASE, 0x9000U, /* 36KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PSC_MBOX_TZ_BASE, 0x2000U, /* 8KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	{0}
};

/*******************************************************************************
 * Set up the pagetables as per the platform memory map & initialize the MMU
 ******************************************************************************/
const mmap_region_t *plat_get_mmio_map(void)
{
	/* MMIO space */
	return tegra_mmap;
}

/*******************************************************************************
 * Handler to get the System Counter Frequency
 ******************************************************************************/
uint32_t plat_get_syscnt_freq2(void)
{
	return 31250000;
}

#if !ENABLE_CONSOLE_SPE
/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA234_MAX_UART_PORTS		6

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra234_uart_addresses[TEGRA234_MAX_UART_PORTS + 1] = {
	0,	/* undefined - treated as an error case */
	TEGRA_UARTA_BASE,
	TEGRA_UARTB_BASE,
	TEGRA_UARTC_BASE,
	TEGRA_UARTD_BASE,
	TEGRA_UARTE_BASE,
	TEGRA_UARTF_BASE,
};
#endif

/*******************************************************************************
 * Enable console corresponding to the console ID
 ******************************************************************************/
void plat_enable_console(int32_t id)
{
	uint32_t console_clock = 0U;

#if ENABLE_CONSOLE_SPE
	static console_t spe_console;

	if (id == TEGRA_CONSOLE_SPE_ID) {
		(void)console_spe_register(TEGRA_CONSOLE_SPE_BASE,
					   console_clock,
					   TEGRA_CONSOLE_BAUDRATE,
					   &spe_console);
		console_set_scope(&spe_console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#else
	static console_t uart_console;

	if ((id > 0) && (id < TEGRA234_MAX_UART_PORTS)) {
		/*
		 * Reference clock used by the FPGAs is a lot slower.
		 */
		if (tegra_platform_is_fpga()) {
			console_clock = TEGRA_BOOT_UART_CLK_13_MHZ;
		} else {
			console_clock = TEGRA_BOOT_UART_CLK_408_MHZ;
		}

		(void)console_16550_register(tegra234_uart_addresses[id],
					     console_clock,
					     TEGRA_CONSOLE_BAUDRATE,
					     &uart_console);
		console_set_scope(&uart_console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#endif
}

/*******************************************************************************
 * Handler for early platform setup
 ******************************************************************************/
void plat_early_platform_setup(void)
{
	uint32_t __unused val;
	unsigned int cluster;
	u_register_t reg;

	/* sanity check MCE firmware compatibility */
	mce_verify_firmware_version();

	/* SCF flush - Clean and invalidate caches */
	mce_clean_and_invalidate_caches();

#if ENABLE_FEAT_RAS
	/* Enable RAS handling for all the common nodes */
	tegra234_ras_init_common();
#endif

	/*
	 * Number of physical cores should be less than PLATFOM_CORE_COUNT
	 */
	assert(PLATFORM_CORE_COUNT >= mce_num_cores());

#if !DEBUG
	/*
	 * check ETM settings
	 * Assert if the TRCPRGCTLR.EN bit is not set.
	 */
	val = mmio_read_32(TEGRA_DBGAPB_BASE + TEGRA_TRCPRGCTLR_OFFSET);
	assert((val & TRCPRGCTLR_EN_BIT) == TRCPRGCTLR_EN_BIT);
#endif

	/*
	 * Program the following bits to '1'.
	 *
	 * Bit 0: Enable interleaving normal Non-cacheable transactions between
	 * master interfaces like Cacheable transactions.
	 * Bit 14: Enables sending WriteEvict transactions on the ACE or CHI
	 * master for UniqueClean evictions.
	 */
	reg = read_clusterectlr_el1();
	write_clusterectlr_el1(reg | DSU_WRITEEVICT_BIT | DSU_NC_CTRL_BIT);

	/* Print the Cluster Execution Mode (CEMODE) for each cluster */
	for (cluster = 0; cluster < PLATFORM_CLUSTER_COUNT; ++cluster) {
		if (mce_is_cluster_in_hybrid_mode(cluster)) {
			INFO("Cluster %d is running in %s mode\n", cluster, "hybrid");
		}
		else if (mce_is_cluster_in_lock_mode(cluster)) {
			INFO("Cluster %d is running in %s mode\n", cluster, "lock");
		}
	}
}

/******************************************************************************
 * On a GICv3 system, use Group0 interrupts as secure interrupts
 *****************************************************************************/
static const interrupt_prop_t tegra234_interrupt_props[] = {
	INTR_PROP_DESC(TEGRA234_RAS_PPI_ERI, PLAT_RAS_PRI,
			INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(TEGRA234_RAS_PPI_FHI, PLAT_RAS_PRI,
			INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(TEGRA_SDEI_SGI_PRIVATE, PLAT_SDEI_CRITICAL_PRI,
			INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(TEGRA234_TOP_WDT_IRQ, PLAT_TEGRA_WDT_PRIO,
			INTR_GROUP0, GIC_INTR_CFG_EDGE)
};

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	/* Tegra234 contains GICD, SPI Collator, Wake Request, PPI0-PPI2 blocks */
	uint64_t tegra234_gic600_fmu_blks = BIT(FMU_BLK_GICD) | BIT(FMU_BLK_SPICOL) |
			BIT(FMU_BLK_WAKERQ) | BIT(FMU_BLK_PPI0) | BIT(FMU_BLK_PPI1) |
			BIT(FMU_BLK_PPI2);
	uint32_t smen;

	tegra_gic_setup(tegra234_interrupt_props, ARRAY_SIZE(tegra234_interrupt_props));

	/* initialize the GICD and GICR */
	tegra_gic_init();

#if GICV3_SUPPORT_GIC600AE_FMU
	/* initialize the Fault Management Unit */
	if (tegra_platform_is_fpga() || tegra_platform_is_silicon()) {
		gic600_fmu_init(TEGRA_GICFMU_BASE, tegra234_gic600_fmu_blks,
			true, true);

		/*
		 * The PPI0/1/2 block SMID 0xB causes spurious RAS errors.
		 * As a workaround, disable the SMID for these blocks.
		 */
		smen = (FMU_BLK_PPI0 << FMU_SMEN_BLK_SHIFT) |
			(0xb << FMU_SMEN_SMID_SHIFT);
		gic_fmu_write_smen(TEGRA_GICFMU_BASE, smen);

		smen = (FMU_BLK_PPI1 << FMU_SMEN_BLK_SHIFT) |
			(0xb << FMU_SMEN_SMID_SHIFT);
		gic_fmu_write_smen(TEGRA_GICFMU_BASE, smen);

		smen = (FMU_BLK_PPI2 << FMU_SMEN_BLK_SHIFT) |
			(0xb << FMU_SMEN_SMID_SHIFT);
		gic_fmu_write_smen(TEGRA_GICFMU_BASE, smen);
	}
#endif
	/*
	 * Initialize the FIQ handler only if the platform supports any
	 * FIQ interrupt sources.
	 */
	tegra_fiq_handler_setup();

	/*
	 * Enable the GIC synchronization work-around
	 */
	mce_enable_gic_sync_control();
}

/*******************************************************************************
 * Return pointer to the BL31 params from previous bootloader
 ******************************************************************************/
struct tegra_bl31_params *plat_get_bl31_params(void)
{
	uint32_t reg;
	uint64_t val;

	reg = mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PARAMS_HI_ADDR);
	val = ((uint64_t)reg & SCRATCH_BL31_PARAMS_HI_ADDR_MASK) >>
			 SCRATCH_BL31_PARAMS_HI_ADDR_SHIFT;
	val <<= 32;
	val |= (uint64_t)mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PARAMS_LO_ADDR);

	return (struct tegra_bl31_params *)(uintptr_t)val;
}

/*******************************************************************************
 * Return pointer to the BL31 platform params from previous bootloader
 ******************************************************************************/
bl31_plat_params_t *plat_get_bl31_plat_params(void)
{
	uint32_t reg;
	uint64_t val;

	reg = mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PLAT_PARAMS_HI_ADDR);
	val = ((uint64_t)reg & SCRATCH_BL31_PLAT_PARAMS_HI_ADDR_MASK) >>
			 SCRATCH_BL31_PLAT_PARAMS_HI_ADDR_SHIFT;
	val <<= 32;
	val |= (uint64_t)mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PLAT_PARAMS_LO_ADDR);

	return (bl31_plat_params_t *)(uintptr_t)val;
}

/*******************************************************************************
 * Handler for late platform setup
 ******************************************************************************/
void plat_late_platform_setup(void)
{
	uint32_t ret;
	struct bpmp_ipc_platform_data *bpmp_ipc_data = plat_get_bpmp_ipc_data();

#if ENABLE_STRICT_CHECKING_MODE
	/*
	 * Enable strict checking after programming the GSC for
	 * enabling TZSRAM and TZDRAM
	 */
	mce_enable_strict_checking();
#endif

	/* memmap bpmp_ipc_tz addr */
	if (bpmp_ipc_data != NULL) {
		ret = mmap_add_dynamic_region(bpmp_ipc_data->bpmp_ipc_tx_base, /* PA */
				bpmp_ipc_data->bpmp_ipc_tx_base, 	/* VA */
				(TEGRA_BPMP_IPC_CH_MAP_SIZE * 2), 	/* 8KB size */
				MT_DEVICE | MT_RW | MT_SECURE); /* attrs */
		assert(ret == 0);
	}

	/* report erratas applied */
	tegra234_report_errata();

}

/*******************************************************************************
 * Handler to indicate support for System Suspend
 ******************************************************************************/
bool plat_supports_system_suspend(void)
{
	return true;
}

/*******************************************************************************
 * Platform specific FIQ handler.
 * Return true if `irq_num` number is handled by this function.
 ******************************************************************************/
bool plat_fiq_handler(uint32_t irq_num)
{
	return false;
}

/*******************************************************************************
 * Platform specific runtime setup.
 ******************************************************************************/
void plat_runtime_setup(void)
{
	/*
	 * During cold boot, it is observed that the arbitration
	 * bit is set in the Memory controller leading to false
	 * error interrupts in the non-secure world. To avoid
	 * this, clean the interrupt status register before
	 * booting into the non-secure world
	 */
	tegra_memctrl_clear_pending_interrupts();

	/*
	 * During boot, USB3 and flash media (SDMMC/SATA) devices need
	 * access to IRAM. Because these clients connect to the MC and
	 * do not have a direct path to the IRAM, the MC implements AHB
	 * redirection during boot to allow path to IRAM. In this mode
	 * accesses to a programmed memory address aperture are directed
	 * to the AHB bus, allowing access to the IRAM. This mode must be
	 * disabled before we jump to the non-secure world.
	 */
	tegra_memctrl_disable_ahb_redirection();

	/*
	 * SCF flush - Clean and invalidate caches before jumping to the
	 * non-secure world payload.
	 */
	mce_clean_and_invalidate_caches();
}

/*******************************************************************************
 * Return pointer to the bpmp_ipc_data
 ******************************************************************************/
static struct bpmp_ipc_platform_data bpmp_ipc;
struct bpmp_ipc_platform_data *plat_get_bpmp_ipc_data(void)
{
	uint64_t rx_end_addr;
	uint64_t sysram_end_addr;

	/*
	 * BPMP FW is not supported on QT/VSP platforms. Disable BPMP_IPC
	 * interface.
	 */
	if (tegra_platform_is_qt() || tegra_platform_is_vsp()) {
        	return NULL;
        }

	bpmp_ipc.bpmp_ipc_tx_base = (((uint64_t)mmio_read_32(TEGRA_MC_BASE +
				MC_CCPLEX_BPMP_IPC_BASE_HI) << 32)
			 	| mmio_read_32(TEGRA_MC_BASE + MC_CCPLEX_BPMP_IPC_BASE_LO));
	bpmp_ipc.bpmp_ipc_rx_base = bpmp_ipc.bpmp_ipc_tx_base + TEGRA_BPMP_IPC_CH_MAP_SIZE;
	bpmp_ipc.bpmp_ipc_map_size = TEGRA_BPMP_IPC_CH_MAP_SIZE;

	/*
	 * Disable BPMP_IPC interface if TX/RX shared memory is not present.
	 */
	if ((bpmp_ipc.bpmp_ipc_tx_base == 0U) || (bpmp_ipc.bpmp_ipc_rx_base == 0U) ||
	    (bpmp_ipc.bpmp_ipc_map_size == 0U)) {
		WARN("BPMP_IPC TX/RX memory is uninitialized\n");
		return NULL;
	}

	/*
	 * Assert with the tx_base (or) rx end address is not with in sysram range.
	 */
	rx_end_addr = bpmp_ipc.bpmp_ipc_rx_base + TEGRA_BPMP_IPC_CH_MAP_SIZE;
	sysram_end_addr = TEGRA_SYSRAM_BASE + TEGRA_SYSRAM_SIZE;
	assert((bpmp_ipc.bpmp_ipc_tx_base >= TEGRA_SYSRAM_BASE) && (rx_end_addr < sysram_end_addr));

	return &bpmp_ipc;
}

/*******************************************************************************
 * Platform handler called to check the validity of the non secure entrypoint.
 ******************************************************************************/
int32_t tegra_soc_validate_ns_entrypoint(uintptr_t entrypoint)
{
	int32_t ret = PSCI_E_INVALID_ADDRESS;

	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= TEGRA_DRAM_BASE) && (entrypoint <= TEGRA_DRAM_END)) {
		ret = PSCI_E_SUCCESS;
	}

	return ret;
}

/*******************************************************************************
 * Parse the boot parameters struct and update the internal database
 ******************************************************************************/
int32_t plat_parse_bl31_plat_params(bl31_plat_params_t *plat_bl31_params,
				void *params_from_bl2)
{
	t234_bl31_plat_params_t *plat_params =
				(t234_bl31_plat_params_t *)params_from_bl2;

	/*
	* Parse platform specific parameters
	*/
	assert(plat_params != NULL);
	assert(plat_bl31_params != NULL);

	plat_bl31_params->tzdram_base = plat_params->tzdram_base;
	plat_bl31_params->tzdram_size = plat_params->tzdram_size;
	plat_bl31_params->uart_id = plat_params->uart_port_id;
	plat_bl31_params->log_level = LOG_LEVEL;

	/* store the membase for the profiler lib */
	plat_bl31_params->boot_profiler_shmem_base =
				plat_params->boot_profiler_shmem_base;

	return 0;
}
