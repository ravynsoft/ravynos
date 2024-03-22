/*
 * Copyright 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SPM_H
#define AC_SPM_H

#include <stdint.h>

#include "ac_perfcounter.h"

#define AC_SPM_MAX_COUNTER_PER_BLOCK 16
#define AC_SPM_GLOBAL_TIMESTAMP_COUNTERS 4 /* in unit of 16-bit counters*/
#define AC_SPM_NUM_COUNTER_PER_MUXSEL 16 /* 16 16-bit counters per muxsel */
#define AC_SPM_MUXSEL_LINE_SIZE ((AC_SPM_NUM_COUNTER_PER_MUXSEL * 2) / 4) /* in dwords */
#define AC_SPM_NUM_PERF_SEL 4

/* GFX10+ */
enum ac_spm_global_block {
    AC_SPM_GLOBAL_BLOCK_CPG,
    AC_SPM_GLOBAL_BLOCK_CPC,
    AC_SPM_GLOBAL_BLOCK_CPF,
    AC_SPM_GLOBAL_BLOCK_GDS,
    AC_SPM_GLOBAL_BLOCK_GCR,
    AC_SPM_GLOBAL_BLOCK_PH,
    AC_SPM_GLOBAL_BLOCK_GE,
    AC_SPM_GLOBAL_BLOCK_GE1 = AC_SPM_GLOBAL_BLOCK_GE,
    AC_SPM_GLOBAL_BLOCK_GL2A,
    AC_SPM_GLOBAL_BLOCK_GL2C,
    AC_SPM_GLOBAL_BLOCK_SDMA,
    AC_SPM_GLOBAL_BLOCK_GUS,
    AC_SPM_GLOBAL_BLOCK_EA,
    AC_SPM_GLOBAL_BLOCK_CHA,
    AC_SPM_GLOBAL_BLOCK_CHC,
    AC_SPM_GLOBAL_BLOCK_CHCG,
    AC_SPM_GLOBAL_BLOCK_GPUVMATTCL2,
    AC_SPM_GLOBAL_BLOCK_GPUVMVML2,
    AC_SPM_GLOBAL_BLOCK_GE2SE, /* Per-SE counters */
    AC_SPM_GLOBAL_BLOCK_GE2DIST,

    /* GFX11+ */
    /* gap */
    AC_SPM_GLOBAL_BLOCK_RSPM = 31,
};

enum ac_spm_se_block {
    AC_SPM_SE_BLOCK_CB,
    AC_SPM_SE_BLOCK_DB,
    AC_SPM_SE_BLOCK_PA,
    AC_SPM_SE_BLOCK_SX,
    AC_SPM_SE_BLOCK_SC,
    AC_SPM_SE_BLOCK_TA,
    AC_SPM_SE_BLOCK_TD,
    AC_SPM_SE_BLOCK_TCP,
    AC_SPM_SE_BLOCK_SPI,
    AC_SPM_SE_BLOCK_SQG,
    AC_SPM_SE_BLOCK_GL1A,
    AC_SPM_SE_BLOCK_RMI,
    AC_SPM_SE_BLOCK_GL1C,
    AC_SPM_SE_BLOCK_GL1CG,

    /* GFX11+ */
    AC_SPM_SE_BLOCK_CBR,
    AC_SPM_SE_BLOCK_DBR,
    AC_SPM_SE_BLOCK_GL1H,
    AC_SPM_SE_BLOCK_SQC,
    AC_SPM_SE_BLOCK_PC,
    /* gap */
    AC_SPM_SE_BLOCK_SE_RPM = 31,
};

enum ac_spm_segment_type {
   AC_SPM_SEGMENT_TYPE_SE0,
   AC_SPM_SEGMENT_TYPE_SE1,
   AC_SPM_SEGMENT_TYPE_SE2,
   AC_SPM_SEGMENT_TYPE_SE3,
   AC_SPM_SEGMENT_TYPE_SE4,
   AC_SPM_SEGMENT_TYPE_SE5,
   AC_SPM_SEGMENT_TYPE_GLOBAL,
   AC_SPM_SEGMENT_TYPE_COUNT,
};

struct ac_spm_counter_descr {
   enum ac_pc_gpu_block gpu_block;
   uint32_t event_id;
};

struct ac_spm_counter_create_info {
   struct ac_spm_counter_descr *b;
   uint32_t instance;
};

union ac_spm_muxsel {
   struct {
      uint16_t counter      : 6;
      uint16_t block        : 4;
      uint16_t shader_array : 1; /* 0: SA0, 1: SA1 */
      uint16_t instance     : 5;
   } gfx10;

   struct {
      uint16_t counter      : 5;
      uint16_t instance     : 5;
      uint16_t shader_array : 1;
      uint16_t block        : 5;
   } gfx11;
   uint16_t value;
};

struct ac_spm_muxsel_line {
   union ac_spm_muxsel muxsel[AC_SPM_NUM_COUNTER_PER_MUXSEL];
};

struct ac_spm_counter_info {
   /* General info. */
   enum ac_pc_gpu_block gpu_block;
   uint32_t instance;
   uint32_t event_id;

   /* Muxsel info. */
   enum ac_spm_segment_type segment_type;
   bool is_even;
   union ac_spm_muxsel muxsel;

   /* Output info. */
   uint64_t offset;
};

struct ac_spm_counter_select {
   uint8_t active; /* mask of used 16-bit counters. */
   uint32_t sel0;
   uint32_t sel1;
};

struct ac_spm_block_instance {
   uint32_t grbm_gfx_index;

   uint32_t num_counters;
   struct ac_spm_counter_select counters[AC_SPM_MAX_COUNTER_PER_BLOCK];
};

struct ac_spm_block_select {
   const struct ac_pc_block *b;

   uint32_t num_instances;
   struct ac_spm_block_instance *instances;
};

struct ac_spm {
   /* struct radeon_winsys_bo or struct pb_buffer */
   void *bo;
   void *ptr;
   uint8_t ptr_granularity;
   uint32_t buffer_size;
   uint16_t sample_interval;

   /* Enabled counters. */
   unsigned num_counters;
   struct ac_spm_counter_info *counters;

   /* Block/counters selection. */
   uint32_t num_block_sel;
   struct ac_spm_block_select *block_sel;

   struct {
      uint32_t num_counters;
      struct ac_spm_counter_select counters[16];
   } sqg[AC_SPM_SEGMENT_TYPE_GLOBAL];

   struct {
      uint32_t grbm_gfx_index;
      uint32_t num_counters;
      struct ac_spm_counter_select counters[16];
   } sq_wgp[AMD_MAX_WGP];

   /* Muxsel lines. */
   unsigned num_muxsel_lines[AC_SPM_SEGMENT_TYPE_COUNT];
   struct ac_spm_muxsel_line *muxsel_lines[AC_SPM_SEGMENT_TYPE_COUNT];
   unsigned max_se_muxsel_lines;
};

struct ac_spm_trace {
   void *ptr;
   uint16_t sample_interval;
   unsigned num_counters;
   struct ac_spm_counter_info *counters;
   uint32_t sample_size_in_bytes;
   uint32_t num_samples;
};

bool ac_init_spm(const struct radeon_info *info,
                 const struct ac_perfcounters *pc,
                 struct ac_spm *spm);
void ac_destroy_spm(struct ac_spm *spm);

void ac_spm_get_trace(const struct ac_spm *spm, struct ac_spm_trace *trace);

#endif
