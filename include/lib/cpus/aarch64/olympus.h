/*
 * Copyright (c) 2024-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OLYMPUS_H
#define OLYMPUS_H

#define OLYMPUS_MIDR					U(0x4E0F0100)

/* Olympus loop count for CVE-2022-23960 mitigation */
#define OLYMPUS_BHB_LOOP_COUNT				U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define OLYMPUS_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define OLYMPUS_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define OLYMPUS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary Control register 6 specific definitions.
 ******************************************************************************/
#define OLYMPUS_CPUACTLR6_EL1				S3_0_C15_C8_1
#define OLYMPUS_CPUACTLR6_EL1_BIT_41			(ULL(1) << 41)

/*******************************************************************************
 * FEAT_RNG - RND Base Address Register
 ******************************************************************************/
#define OLYMPUS_CPURNDBR_EL3				S3_6_C15_C3_0

/*******************************************************************************
 * FEAT_RNG - RND PE ID Register
 ******************************************************************************/
#define OLYMPUS_CPURNDPEID_EL3				S3_6_C15_C3_1

/*******************************************************************************
 * NV specific CPU registers
 ******************************************************************************/
#define OLYMPUS_NV_CPUACTLR2_EL1				S3_1_C15_C0_2
#define OLYMPUS_NV_CPUACTLR2_BIT_57				(ULL(1) << 57)
#define OLYMPUS_NV_CPUACTLR2_BIT_56				(ULL(1) << 56)
#define OLYMPUS_NV_CPUACTLR2_BIT_55				(ULL(1) << 55)
#define OLYMPUS_NV_CPUACTLR2_BIT_18				(ULL(1) << 18)
#define OLYMPUS_NV_CPUACTLR2_BIT_0				(ULL(1) << 0)

#define OLYMPUS_NV_CPUACTLR6_EL1				S3_1_C15_C0_6
#define OLYMPUS_NV_CPUACTLR6_BIT_63				(ULL(1) << 63)
#define OLYMPUS_NV_CPUACTLR6_BIT_62				(ULL(1) << 62)
#define OLYMPUS_NV_CPUACTLR6_BIT_61				(ULL(1) << 61)

#define OLYMPUS_NV_CPUACTLR7_EL1				S3_1_C15_C0_7
#define OLYMPUS_NV_CPUACTLR7_BIT_18				(ULL(1) << 18)

#define OLYMPUS_NV_CPUACTLR22_EL1				S3_1_C15_C6_3
#define OLYMPUS_NV_CPUACTLR22_BIT_45				(ULL(1) << 45)
#define OLYMPUS_NV_CPUACTLR22_BIT_46				(ULL(1) << 46)
#define OLYMPUS_NV_CPUACTLR22_BIT_47				(ULL(1) << 47)

#define OLYMPUS_NV_CPUACTLR28_EL1				S3_1_C15_C7_1
#define OLYMPUS_NV_CPUACTLR28_EL1_BLOCK_SECONDARY_PE_BIT	U(1)

#define OLYMPUS_CPUECTLR_EL1_BIT_40				(ULL(1) << 40)
#define OLYMPUS_CPUECTLR_EL1_BIT_41				(ULL(1) << 41)
#define OLYMPUS_CPUECTLR_EL1_BIT_38				(ULL(1) << 38)
#define OLYMPUS_CPUECTLR_EL1_BIT_37				(ULL(1) << 37)
#define OLYMPUS_CPUECTLR_EL1_BIT_36				(ULL(1) << 36)

#endif /* OLYMPUS_H */
