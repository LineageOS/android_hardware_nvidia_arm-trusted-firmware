/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PLATFORM_H
#define TEGRA_PLATFORM_H

#include <cdefs.h>
#include <lib/utils_def.h>
#include <stdbool.h>

/*******************************************************************************
 * Tegra HIDREV major, minor version helper macros
 ******************************************************************************/
#define MAJOR_VERSION_SHIFT		U(0x4)
#define MAJOR_VERSION_MASK		U(0xF)
#define MINOR_VERSION_SHIFT		U(0x10)
#define MINOR_VERSION_MASK		U(0xF)
#define CHIP_ID_SHIFT			U(8)
#define CHIP_ID_MASK			U(0xFF)
#define PRE_SI_PLATFORM_SHIFT		U(0x14)
#define PRE_SI_PLATFORM_MASK		U(0xF)

/*******************************************************************************
 * Tegra SIP_MISC sip_id, major, minor version helper macros
 ******************************************************************************/
#define SIP_MISC_MAGIC_OFFSET		ULL(0)
#define  SIP_MISC_MAGIC_VAL		ULL(0x11150C1D)
#define SIP_MISC_ID_OFFSET		ULL(0x4)
#define  SIP_MISC_ID_REVISION_SHIFT	U(24)
#define  SIP_MISC_ID_REVISION_MASK	GENMASK_32(31, 24)
#define  SIP_MISC_ID_SIPID_MASK		GENMASK_32(11, 0)
#define SIP_MISC_FEATURE_OFFSET		ULL(0x8)
#define  SIP_MISC_FEATURE_TBL_VER_SHIFT	U(24)
#define  SIP_MISC_FEATURE_TBL_VER_MASK	GENMASK_32(31, 24)
#define  SIP_MISC_FEATURE_PRE_SI_MASK	GENMASK_32(7, 0)

/*******************************************************************************
 * Tegra chip ID values
 ******************************************************************************/
#define TEGRA_CHIPID_TEGRA13		U(0x13)
#define TEGRA_CHIPID_TEGRA21		U(0x21)
#define TEGRA_CHIPID_TEGRA18		U(0x18)
#define TEGRA_CHIPID_TEGRA19		U(0x19)
#define TEGRA_CHIPID_TEGRA23		U(0x23)
#define TEGRA_CHIPID_TEGRA24		U(0x24)
#define TEGRA_CHIPID_TEGRA410		U(0x410)

/*******************************************************************************
 * JEDEC Standard Manufacturer's Identification Code and Bank ID
 ******************************************************************************/
#define JEDEC_NVIDIA_MFID		U(0x6B)
#define JEDEC_NVIDIA_BKID		U(3)

#ifndef __ASSEMBLER__

/*
 * Tegra chip ID identifiers
 */
bool tegra_chipid_is_t186(void);
bool tegra_chipid_is_t186_a01(void);
bool tegra_chipid_is_t210(void);
bool tegra_chipid_is_t210_b01(void);
bool tegra_chipid_is_t194(void);
bool tegra_chipid_is_t234(void);
bool tegra_chipid_is_t239(void);
bool tegra_chipid_is_th500(void);

/*
 * Tegra platform identifiers
 */
bool tegra_platform_is_silicon(void);
bool tegra_platform_is_qt(void);
bool tegra_platform_is_emulation(void);
bool tegra_platform_is_linsim(void);
bool tegra_platform_is_fpga(void);
bool tegra_platform_is_unit_fpga(void);
bool tegra_platform_is_virt_dev_kit(void);
bool tegra_platform_is_vsp(void);

#endif /* __ASSEMBLER__ */

#endif /* TEGRA_PLATFORM_H */
