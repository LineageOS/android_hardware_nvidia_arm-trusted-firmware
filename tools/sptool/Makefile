#
# Copyright (c) 2018-2024, Arm Limited. All rights reserved.
# Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

SPTOOL  ?= sptool${BIN_EXT}
PROJECT := $(notdir ${SPTOOL})
OBJECTS := sptool.o

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

INCLUDE_PATHS := -I../../include/tools_share

.PHONY: all clean distclean

all: ${SPTOOL}

HOST_OBJS :=

define MAKE_HOST_OBJS
$(eval HOST_OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))

$(HOST_OBJ): $(2)
	@echo " HOSTCC $$<"
	$${Q}$${HOSTCC} -c $${CPPFLAGS} ${HOSTCCFLAGS} $${INCLUDE_PATHS} $$< -o $$@
HOST_OBJS += $(HOST_OBJ)
endef

$(eval $(call MAKE_HOST_OBJS, $(BUILD_DIR), $(SOURCES)))

${SPTOOL}: ${HOST_OBJS} Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)$(host-cc) ${OBJECTS} -o $@ ${LDLIBS}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${HOST_OBJS} ${SPTOOL})
