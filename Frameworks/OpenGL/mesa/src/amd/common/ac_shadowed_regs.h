/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SHADOWED_REGS
#define AC_SHADOWED_REGS

#include "ac_gpu_info.h"

struct radeon_cmdbuf;

struct ac_reg_range {
   unsigned offset;
   unsigned size;
};

enum ac_reg_range_type
{
   SI_REG_RANGE_UCONFIG,
   SI_REG_RANGE_CONTEXT,
   SI_REG_RANGE_SH,
   SI_REG_RANGE_CS_SH,
   SI_NUM_REG_RANGES,
};

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pm4_cmd_add_fn)(void *pm4_cmdbuf, uint32_t value);

typedef void (*set_context_reg_seq_array_fn)(struct radeon_cmdbuf *cs, unsigned reg, unsigned num,
                                             const uint32_t *values);

void ac_get_reg_ranges(enum amd_gfx_level gfx_level, enum radeon_family family,
                       enum ac_reg_range_type type, unsigned *num_ranges,
                       const struct ac_reg_range **ranges);
void ac_emulate_clear_state(const struct radeon_info *info, struct radeon_cmdbuf *cs,
                            set_context_reg_seq_array_fn set_context_reg_seq_array);
void ac_print_nonshadowed_regs(enum amd_gfx_level gfx_level, enum radeon_family family);

void ac_create_shadowing_ib_preamble(const struct radeon_info *info,
                                     pm4_cmd_add_fn pm4_cmd_add, void *pm4_cmdbuf,
                                     uint64_t gpu_address,
                                     bool dpbb_allowed);
#ifdef __cplusplus
}
#endif


#endif
