/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 * Copyright (c) 2022-2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_V2_H
#define NEOVERSE_V2_H

#define NEOVERSE_V2_MIDR				U(0x410FD4F0)

/* Neoverse V2 loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_V2_BHB_LOOP_COUNT			U(132)

/*******************************************************************************
 * CPU Auxillary Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR_EL1			S3_0_C15_C1_0
#define NEOVERSE_V2_CPUACTLR_EL1_BIT_1			(ULL(1) << 1)

/*******************************************************************************
 * CPU Auxillary Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR2_EL1			S3_0_C15_C1_1
#define NEOVERSE_V2_CPUACTLR2_EL1_BIT_0			(ULL(1) << 0)
#define NEOVERSE_V2_CPUACTLR2_EL1_BIT_40		(ULL(1) << 40)

/*******************************************************************************
 * CPU Auxillary Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR3_EL1			S3_0_C15_C1_2
#define NEOVERSE_V2_CPUACTLR3_EL1_BIT_47		(ULL(1) << 47)
#define NEOVERSE_V2_CPUACTLR3_EL1_BIT_58		(ULL(1) << 58)

/*******************************************************************************
 * CPU Auxiliary Control register 5 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_ACTLR5_EL1				S3_0_C15_C9_0
#define NEOVERSE_V2_ACTLR5_EL1_BIT_55			(ULL(1) << 55)
#define NEOVERSE_V2_ACTLR5_EL1_BIT_56			(ULL(1) << 56)

/*******************************************************************************
 * CPU Auxiliary Control register 6 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR6_EL1			S3_0_C15_C8_1
#define NEOVERSE_V2_CPUACTLR6_EL1_BIT_41		(ULL(1) << 41)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUECTLR_EL1			S3_0_C15_C1_4
#define NEOVERSE_V2_CPUECTLR_EL1_EXTLLC_BIT		BIT(0)
#define NEOVERSE_V2_CPUECTLR_EL1_PF_DIS_BIT		(ULL(1) << 15)
#define NEOVERSE_V2_CPUECTLR_EL1_WS_THR_DRAM_512KB	U(0)
#define CPUECTLR_EL1_WS_THR_DRAM_LSB			U(18)
#define CPUECTLR_EL1_WS_THR_DRAM_WIDTH			U(2)
#define CPUECTLR_EL1_WS_THR_DRAM_MASK			GENMASK_64(19, 18)
#define NEOVERSE_V2_CPUECTLR_EL1_WS_THR_L4_256KB	U(0)
#define CPUECTLR_EL1_WS_THR_L4_LSB			U(20)
#define CPUECTLR_EL1_WS_THR_L4_WIDTH			U(2)
#define CPUECTLR_EL1_WS_THR_L4_MASK			GENMASK_64(21, 20)
#define NEOVERSE_V2_CPUECTLR_EL1_WS_THR_L3_128KB	U(0)
#define CPUECTLR_EL1_WS_THR_L3_LSB			U(22)
#define CPUECTLR_EL1_WS_THR_L3_WIDTH			U(2)
#define CPUECTLR_EL1_WS_THR_L3_MASK			GENMASK_64(23, 22)
#define NEOVERSE_V2_CPUECTLR_EL1_DSFP_BIT		(ULL(1) << 42)
#define NEOVERSE_V2_CPUECTLR_EL1_DCC_ALL		ULL(3)
#define CPUECTLR_EL1_DCC_LSB				U(44)
#define CPUECTLR_EL1_DCC_WIDTH				U(2)
#define CPUECTLR_EL1_DCC_MASK				GENMASK_64(45, 44)
#define CPUECTLR_EL1_CMC_MIN_WAY_LSB			U(61)
#define CPUECTLR_EL1_CMC_MIN_WAY_WIDTH			U(3)
#define CPUECTLR_EL1_CMC_MIN_WAY_MASK			GENMASK_64(63, 61)
#define NEOVERSE_V2_CPUECTLR_EL1_BIT_46			(ULL(1) << 46)

/*******************************************************************************
 * CPU Extended Control register 2 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUECTLR2_EL1			S3_0_C15_C1_5
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_STATIC_FULL	ULL(0)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_LSB		U(0)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_WIDTH		U(3)
#define CPUECTLR2_EL1_TXREQ_MAX_MASK			GENMASK_64(1, 0)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_LIM_DYN_BIT	(ULL(1) << 2)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_LIM_INC_2	ULL(3)
#define CPUECTLR2_EL1_TXREQ_LIM_INC_LSB			U(3)
#define CPUECTLR2_EL1_TXREQ_LIM_INC_WIDTH		U(2)
#define CPUECTLR2_EL1_TXREQ_LIM_INC_MASK		GENMASK_64(4, 3)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_LIM_DEC_2	ULL(3)
#define CPUECTLR2_EL1_TXREQ_LIM_DEC_LSB			U(5)
#define CPUECTLR2_EL1_TXREQ_LIM_DEC_WIDTH		U(2)
#define CPUECTLR2_EL1_TXREQ_LIM_DEC_MASK		GENMASK_64(6, 5)
#define NEOVERSE_V2_CPUECTLR2_EL1_CBUSY_FLTR_THR_1BY8	ULL(2)
#define CPUECTLR2_EL1_CBUSY_FLTR_THR_LSB		U(7)
#define CPUECTLR2_EL1_CBUSY_FLTR_THR_WIDTH		U(2)
#define CPUECTLR2_EL1_CBUSY_FLTR_THR_MASK		GENMASK_64(8, 7)
#define CPUECTLR2_EL1_CBUSY_FILTER_WINDOW_LSB		U(9)
#define CPUECTLR2_EL1_CBUSY_FILTER_WINDOW_WIDTH		U(2)
#define CPUECTLR2_EL1_CBUSY_FILTER_WINDOW_MASK		GENMASK_64(10, 9)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_DEFAULT	ULL(3)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_CNSRV		ULL(9)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_LSB		U(11)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_WIDTH		U(4)
#define CPUECTLR2_EL1_PF_MODE_MASK			GENMASK_64(14, 11)
#define NEOVERSE_V2_CPUECTLR2_TXREQ_MIN_1BY4		ULL(0)
#define NEOVERSE_V2_CPUECTLR2_TXREQ_MIN_1BY8		ULL(1)
#define NEOVERSE_V2_CPUECTLR2_TXREQ_MIN_1BY16		ULL(2)
#define NEOVERSE_V2_CPUECTLR2_TXREQ_MIN_1BY32		ULL(3)
#define CPUECTLR2_EL1_TXREQ_MIN_LSB			U(15)
#define CPUECTLR2_EL1_TXREQ_MIN_WIDTH			U(2)
#define CPUECTLR2_EL1_TXREQ_MIN_MASK			GENMASK_64(16, 15)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_V2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT	U(4)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH	U(3)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT	U(7)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH	U(3)

/*******************************************************************************
 * ERRSELR_EL1 record indices
 ******************************************************************************/
#define NEOVERSE_V2_ERRSELR_SEL_CORE_RAMS		U(0)
#define NEOVERSE_V2_ERRSELR_SEL_MAX			U(1)

#endif /* NEOVERSE_V2_H */
