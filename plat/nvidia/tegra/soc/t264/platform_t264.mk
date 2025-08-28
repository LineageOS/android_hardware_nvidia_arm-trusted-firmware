#
# Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# override SOC_DIR to point to "private" folder
SOC_DIR := ${CURDIR}/../arm-trusted-firmware-private/plat_next/nvidia/tegra/soc/${TARGET_SOC}

include ${SOC_DIR}/platform_t264.mk
