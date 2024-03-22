/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_PERFCOUNTER_H
#define AC_PERFCOUNTER_H

#include <stdbool.h>

#include "sid.h"

#include "ac_gpu_info.h"

/* Max counters per HW block */
#define AC_QUERY_MAX_COUNTERS 16

#define AC_PC_SHADERS_WINDOWING (1u << 31)

enum ac_pc_block_flags
{
   /* This block is part of the shader engine */
   AC_PC_BLOCK_SE = (1 << 0),

   /* Expose per-instance groups instead of summing all instances (within
    * an SE). */
   AC_PC_BLOCK_INSTANCE_GROUPS = (1 << 1),

   /* Expose per-SE groups instead of summing instances across SEs. */
   AC_PC_BLOCK_SE_GROUPS = (1 << 2),

   /* Shader block */
   AC_PC_BLOCK_SHADER = (1 << 3),

   /* Non-shader block with perfcounters windowed by shaders. */
   AC_PC_BLOCK_SHADER_WINDOWED = (1 << 4),
};

enum ac_pc_gpu_block {
   CPF     = 0x0,
   IA      = 0x1,
   VGT     = 0x2,
   PA_SU   = 0x3,
   PA_SC   = 0x4,
   SPI     = 0x5,
   SQ      = 0x6,
   SX      = 0x7,
   TA      = 0x8,
   TD      = 0x9,
   TCP     = 0xA,
   TCC     = 0xB,
   TCA     = 0xC,
   DB      = 0xD,
   CB      = 0xE,
   GDS     = 0xF,
   SRBM    = 0x10,
   GRBM    = 0x11,
   GRBMSE  = 0x12,
   RLC     = 0x13,
   DMA     = 0x14,
   MC      = 0x15,
   CPG     = 0x16,
   CPC     = 0x17,
   WD      = 0x18,
   TCS     = 0x19,
   ATC     = 0x1A,
   ATCL2   = 0x1B,
   MCVML2  = 0x1C,
   EA      = 0x1D,
   RPB     = 0x1E,
   RMI     = 0x1F,
   UMCCH   = 0x20,
   GE      = 0x21,
   GE1     = GE,
   GL1A    = 0x22,
   GL1C    = 0x23,
   GL1CG   = 0x24,
   GL2A    = 0x25,
   GL2C    = 0x26,
   CHA     = 0x27,
   CHC     = 0x28,
   CHCG    = 0x29,
   GUS     = 0x2A,
   GCR     = 0x2B,
   PA_PH   = 0x2C,
   UTCL1   = 0x2D,
   GEDIST  = 0x2E,
   GESE    = 0x2F,
   DF      = 0x30,
   SQ_WGP  = 0x31, /* GFX11+ */
   NUM_GPU_BLOCK,
};

struct ac_pc_block_base {
   enum ac_pc_gpu_block gpu_block;
   const char *name;
   unsigned num_counters;
   unsigned flags;

   unsigned select_or;
   unsigned *select0;
   unsigned counter0_lo;
   unsigned *counters;

   /* SPM */
   unsigned num_spm_counters;
   unsigned num_spm_wires;
   unsigned *select1;
   unsigned spm_block_select;
};

struct ac_pc_block_gfxdescr {
   struct ac_pc_block_base *b;
   unsigned selectors;
   unsigned instances;
};

struct ac_pc_block {
   const struct ac_pc_block_gfxdescr *b;
   unsigned num_instances;
   unsigned num_global_instances;

   unsigned num_groups;
   char *group_names;
   unsigned group_name_stride;

   char *selector_names;
   unsigned selector_name_stride;
};

struct ac_perfcounters {
   unsigned num_groups;
   unsigned num_blocks;
   struct ac_pc_block *blocks;

   bool separate_se;
   bool separate_instance;
};

/* The order is chosen to be compatible with GPUPerfStudio's hardcoding of
 * performance counter group IDs.
 */
static const char *const ac_pc_shader_type_suffixes[] = {"",    "_ES", "_GS", "_VS",
                                                         "_PS", "_LS", "_HS", "_CS"};

static const unsigned ac_pc_shader_type_bits[] = {
   0x7f,
   S_036780_ES_EN(1),
   S_036780_GS_EN(1),
   S_036780_VS_EN(1),
   S_036780_PS_EN(1),
   S_036780_LS_EN(1),
   S_036780_HS_EN(1),
   S_036780_CS_EN(1),
};

static inline bool
ac_pc_block_has_per_se_groups(const struct ac_perfcounters *pc,
                              const struct ac_pc_block *block)
{
   return block->b->b->flags & AC_PC_BLOCK_SE_GROUPS ||
          (block->b->b->flags & AC_PC_BLOCK_SE && pc->separate_se);
}

static inline bool
ac_pc_block_has_per_instance_groups(const struct ac_perfcounters *pc,
                                    const struct ac_pc_block *block)
{
   return block->b->b->flags & AC_PC_BLOCK_INSTANCE_GROUPS ||
          (block->num_instances > 1 && pc->separate_instance);
}

struct ac_pc_block *ac_lookup_counter(const struct ac_perfcounters *pc,
                                      unsigned index, unsigned *base_gid,
                                      unsigned *sub_index);
struct ac_pc_block *ac_lookup_group(const struct ac_perfcounters *pc,
                                    unsigned *index);

struct ac_pc_block *ac_pc_get_block(const struct ac_perfcounters *pc,
                                    enum ac_pc_gpu_block gpu_block);

bool ac_init_block_names(const struct radeon_info *info,
                         const struct ac_perfcounters *pc,
                         struct ac_pc_block *block);

bool ac_init_perfcounters(const struct radeon_info *info,
                          bool separate_se,
                          bool separate_instance,
                          struct ac_perfcounters *pc);
void ac_destroy_perfcounters(struct ac_perfcounters *pc);

#endif
