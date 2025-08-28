/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BPMP_IPC_H
#define BPMP_IPC_H

#include <lib/utils_def.h>
#include <stdbool.h>
#include <stdint.h>


/*
 * Struct describing bpmp_ipc channel info
 */
struct bpmp_ipc_platform_data {
	uint64_t bpmp_ipc_tx_base;
	uint64_t bpmp_ipc_rx_base;
	uint32_t bpmp_ipc_map_size;
};

/**
 * Currently supported reset identifiers
 */
#define TEGRA_RESET_ID_XUSB_PADCTL	U(114)
#define TEGRA_RESET_ID_GPCDMA		U(70)

#define TEGRA_SYSTEM_OFF		U(0)
#define TEGRA_SYSTEM_RESET		U(1)

/**
 * Function to initialise the IPC with the bpmp
 */
int32_t tegra_bpmp_ipc_init(uint32_t num_sockets);

/**
 * Handler to reset a module
 */
int32_t tegra_bpmp_ipc_reset_module(uint32_t rst_id);

/**
 * Handler to enable clock to a module. Only SE device is
 * supported for now.
 */
int tegra_bpmp_ipc_enable_clock(uint32_t clk_id);

/**
 * Handler to disable clock to a module. Only SE device is
 * supported for now.
 */
int tegra_bpmp_ipc_disable_clock(uint32_t clk_id);

/**
 * Response to MRQ_GEARS command
 *
 * Used to request thresholds for NDIV offset switching.
 *
 * The mrq_gears_response::ncpu array defines four thresholds in units
 * of number of online CPUS to be used for choosing between five different
 * NDIV offset settings for CCPLEX cluster NAFLLs
 *
 * 1. If number of online CPUs < ncpu[0] use offset0
 * 2. If number of online CPUs < ncpu[1] use offset1
 * 3. If number of online CPUs < ncpu[2] use offset2
 * 4. If number of online CPUs < ncpu[3] use offset3
 * 5. If number of online CPUs >= ncpu[3] disable offsetting
 *
 * mrq_response::err is
 * * 0: gears defined and response data valid
 * * -#BPMP_ENODEV: MRQ is not supported by BPMP-FW
 * * -#BPMP_EACCES: Operation not permitted for the MRQ master
 * * -#BPMP_ENAVAIL: NDIV offsetting is disabled
 */
struct __attribute__((packed)) mrq_gears_response {
	/* number of online CPUs for each gear */
	uint32_t ncpu[16];
};

/**
 * Handler to request thresholds for NDIV offset switching for a given
 * socket.
 *
 * The mrq_gears_response::ncpu array defines thresholds in units
 * of number of online CPUS to be used for choosing between different
 * NDIV offset settings for CCPLEX cluster NAFLLs
 *
 * 1. If number of online CPUs < ncpu[0] use offset0
 * 2. If number of online CPUs < ncpu[1] use offset1
 * 3. If number of online CPUs < ncpu[2] use offset2
 * 4. If number of online CPUs < ncpu[3] use offset3
 * 5. If number of online CPUs >= ncpu[3] disable offsetting
 */
int tegra_bpmp_ipc_get_ndiv_offsets(uint32_t socket, struct mrq_gears_response *resp);

/**
 * Handler to get bpmp_ipc channel info.
 */
struct bpmp_ipc_platform_data *plat_get_bpmp_ipc_data(void);

/**
 * Handler to send reset MRQ to bpmp-fw
 */
void tegra_bpmp_ipc_system_shutdown(uint32_t state);

#endif /* BPMP_IPC_H */
