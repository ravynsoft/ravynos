/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef INTEL_PERF_H
#define INTEL_PERF_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(MAJOR_IN_SYSMACROS)
#include <sys/sysmacros.h>
#elif defined(MAJOR_IN_MKDEV)
#include <sys/mkdev.h>
#endif

#include "compiler/glsl/list.h"
#include "dev/intel_device_info.h"
#include "util/bitscan.h"
#include "util/bitset.h"
#include "util/hash_table.h"
#include "util/ralloc.h"

#include "drm-uapi/i915_drm.h"

#define INTEL_PERF_MAX_METRIC_SETS (1500)

#ifdef __cplusplus
extern "C" {
#endif

struct intel_perf_config;
struct intel_perf_query_info;

#define INTEL_PERF_INVALID_CTX_ID (0xffffffff)

enum ENUM_PACKED intel_perf_counter_type {
   INTEL_PERF_COUNTER_TYPE_EVENT,
   INTEL_PERF_COUNTER_TYPE_DURATION_NORM,
   INTEL_PERF_COUNTER_TYPE_DURATION_RAW,
   INTEL_PERF_COUNTER_TYPE_THROUGHPUT,
   INTEL_PERF_COUNTER_TYPE_RAW,
   INTEL_PERF_COUNTER_TYPE_TIMESTAMP,
};

enum ENUM_PACKED intel_perf_counter_data_type {
   INTEL_PERF_COUNTER_DATA_TYPE_BOOL32,
   INTEL_PERF_COUNTER_DATA_TYPE_UINT32,
   INTEL_PERF_COUNTER_DATA_TYPE_UINT64,
   INTEL_PERF_COUNTER_DATA_TYPE_FLOAT,
   INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE,
};

enum ENUM_PACKED intel_perf_counter_units {
   /* size */
   INTEL_PERF_COUNTER_UNITS_BYTES,
   INTEL_PERF_COUNTER_UNITS_GBPS,

   /* frequency */
   INTEL_PERF_COUNTER_UNITS_HZ,

   /* time */
   INTEL_PERF_COUNTER_UNITS_NS,
   INTEL_PERF_COUNTER_UNITS_US,

   /**/
   INTEL_PERF_COUNTER_UNITS_PIXELS,
   INTEL_PERF_COUNTER_UNITS_TEXELS,
   INTEL_PERF_COUNTER_UNITS_THREADS,
   INTEL_PERF_COUNTER_UNITS_PERCENT,

   /* events */
   INTEL_PERF_COUNTER_UNITS_MESSAGES,
   INTEL_PERF_COUNTER_UNITS_NUMBER,
   INTEL_PERF_COUNTER_UNITS_CYCLES,
   INTEL_PERF_COUNTER_UNITS_EVENTS,
   INTEL_PERF_COUNTER_UNITS_UTILIZATION,

   /**/
   INTEL_PERF_COUNTER_UNITS_EU_SENDS_TO_L3_CACHE_LINES,
   INTEL_PERF_COUNTER_UNITS_EU_ATOMIC_REQUESTS_TO_L3_CACHE_LINES,
   INTEL_PERF_COUNTER_UNITS_EU_REQUESTS_TO_L3_CACHE_LINES,
   INTEL_PERF_COUNTER_UNITS_EU_BYTES_PER_L3_CACHE_LINE,

   INTEL_PERF_COUNTER_UNITS_MAX
};

struct intel_pipeline_stat {
   uint32_t reg;
   uint32_t numerator;
   uint32_t denominator;
};

/*
 * The largest OA formats we can use include:
 * For Haswell:
 *   1 timestamp, 45 A counters, 8 B counters and 8 C counters.
 * For Gfx8+
 *   1 timestamp, 1 clock, 36 A counters, 8 B counters and 8 C counters
 *
 * Plus 2 PERF_CNT registers and 1 RPSTAT register.
 */
#define MAX_OA_REPORT_COUNTERS (62 + 2 + 1)

/*
 * When currently allocate only one page for pipeline statistics queries. Here
 * we derived the maximum number of counters for that amount.
 */
#define STATS_BO_SIZE               4096
#define STATS_BO_END_OFFSET_BYTES   (STATS_BO_SIZE / 2)
#define MAX_STAT_COUNTERS           (STATS_BO_END_OFFSET_BYTES / 8)

#define I915_PERF_OA_SAMPLE_SIZE (8 +   /* drm_i915_perf_record_header */ \
                                  256)  /* OA counter report */

struct intel_perf_query_result {
   /**
    * Storage for the final accumulated OA counters.
    */
   uint64_t accumulator[MAX_OA_REPORT_COUNTERS];

   /**
    * Hw ID used by the context on which the query was running.
    */
   uint32_t hw_id;

   /**
    * Number of reports accumulated to produce the results.
    */
   uint32_t reports_accumulated;

   /**
    * Frequency in the slices of the GT at the begin and end of the
    * query.
    */
   uint64_t slice_frequency[2];

   /**
    * Frequency in the unslice of the GT at the begin and end of the
    * query.
    */
   uint64_t unslice_frequency[2];

   /**
    * Frequency of the whole GT at the begin and end of the query.
    */
   uint64_t gt_frequency[2];

   /**
    * Timestamp of the query.
    */
   uint64_t begin_timestamp;

   /**
    * Timestamp of the query.
    */
   uint64_t end_timestamp;

   /**
    * Whether the query was interrupted by another workload (aka preemption).
    */
   bool query_disjoint;
};

typedef uint64_t (*intel_counter_read_uint64_t)(struct intel_perf_config *perf,
                                                const struct intel_perf_query_info *query,
                                                const struct intel_perf_query_result *results);

typedef float (*intel_counter_read_float_t)(struct intel_perf_config *perf,
                                            const struct intel_perf_query_info *query,
                                            const struct intel_perf_query_result *results);

struct intel_perf_query_counter {
   const char *name;
   const char *desc;
   const char *symbol_name;
   const char *category;
   enum intel_perf_counter_type type;
   enum intel_perf_counter_data_type data_type;
   enum intel_perf_counter_units units;
   size_t offset;

   union {
      intel_counter_read_uint64_t oa_counter_max_uint64;
      intel_counter_read_float_t  oa_counter_max_float;
   };

   union {
      intel_counter_read_uint64_t oa_counter_read_uint64;
      intel_counter_read_float_t  oa_counter_read_float;
      struct intel_pipeline_stat pipeline_stat;
   };
};

struct intel_perf_query_register_prog {
   uint32_t reg;
   uint32_t val;
};

/* Register programming for a given query */
struct intel_perf_registers {
   const struct intel_perf_query_register_prog *flex_regs;
   uint32_t n_flex_regs;

   const struct intel_perf_query_register_prog *mux_regs;
   uint32_t n_mux_regs;

   const struct intel_perf_query_register_prog *b_counter_regs;
   uint32_t n_b_counter_regs;
};

struct intel_perf_query_info {
   struct intel_perf_config *perf;

   enum intel_perf_query_type {
      INTEL_PERF_QUERY_TYPE_OA,
      INTEL_PERF_QUERY_TYPE_RAW,
      INTEL_PERF_QUERY_TYPE_PIPELINE,
   } kind;
   const char *name;
   const char *symbol_name;
   const char *guid;
   struct intel_perf_query_counter *counters;
   int n_counters;
   int max_counters;
   size_t data_size;

   /* OA specific */
   uint64_t oa_metrics_set_id;
   int oa_format;

   /* For indexing into the accumulator[] ... */
   int gpu_time_offset;
   int gpu_clock_offset;
   int a_offset;
   int b_offset;
   int c_offset;
   int perfcnt_offset;
   int rpstat_offset;

   struct intel_perf_registers config;
};

/* When not using the MI_RPC command, this structure describes the list of
 * register offsets as well as their storage location so that they can be
 * stored through a series of MI_SRM commands and accumulated with
 * intel_perf_query_result_accumulate_snapshots().
 */
struct intel_perf_query_field_layout {
   /* Alignment for the layout */
   uint32_t alignment;

   /* Size of the whole layout */
   uint32_t size;

   uint32_t n_fields;

   struct intel_perf_query_field {
      /* MMIO location of this register */
      uint16_t mmio_offset;

      /* Location of this register in the storage */
      uint16_t location;

      /* Type of register, for accumulation (see intel_perf_query_info:*_offset
       * fields)
       */
      enum intel_perf_query_field_type {
         INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC,
         INTEL_PERF_QUERY_FIELD_TYPE_SRM_PERFCNT,
         INTEL_PERF_QUERY_FIELD_TYPE_SRM_RPSTAT,
         INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_A,
         INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_B,
         INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_C,
      } type;

      /* Index of register in the given type (for instance A31 or B2,
       * etc...)
       */
      uint8_t index;

      /* 4, 8 or 256 */
      uint16_t size;

      /* If not 0, mask to apply to the register value. */
      uint64_t mask;
   } *fields;
};

struct intel_perf_query_counter_info {
   struct intel_perf_query_counter *counter;

   BITSET_DECLARE(query_mask, INTEL_PERF_MAX_METRIC_SETS);

   /**
    * Each counter can be a part of many groups, each time at different index.
    * This struct stores one of those locations.
    */
   struct {
      int group_idx; /* query/group number */
      int counter_idx; /* index inside of query/group */
   } location;
};

struct intel_perf_config {
   /* Whether i915 has DRM_I915_QUERY_PERF_CONFIG support. */
   bool i915_query_supported;

   /* Have extended metrics been enabled */
   bool enable_all_metrics;

   /* Version of the i915-perf subsystem, refer to i915_drm.h. */
   int i915_perf_version;

   /* Number of bits to shift the OA timestamp values by to match the ring
    * timestamp.
    */
   int oa_timestamp_shift;

   /* Mask of bits valid from the OA report (for instance you might have the
    * lower 31 bits [30:0] of timestamp value). This is useful if you want to
    * recombine a full timestamp value captured from the CPU with OA
    * timestamps captured on the device but that only include 31bits of data.
    */
   uint64_t oa_timestamp_mask;

   /* Powergating configuration for the running the query. */
   struct drm_i915_gem_context_param_sseu sseu;

   struct intel_perf_query_info *queries;
   int n_queries;

   struct intel_perf_query_counter_info *counter_infos;
   int n_counters;

   struct intel_perf_query_field_layout query_layout;

   /* Variables referenced in the XML meta data for OA performance
    * counters, e.g in the normalization equations.
    *
    * All uint64_t for consistent operand types in generated code
    */
   struct {
      uint64_t n_eus;               /** $EuCoresTotalCount */
      uint64_t n_eu_slices;         /** $EuSlicesTotalCount */
      uint64_t n_eu_sub_slices;     /** $EuSubslicesTotalCount */
      uint64_t n_eu_slice0123;      /** $EuDualSubslicesSlice0123Count */
      uint64_t slice_mask;          /** $SliceMask */
      uint64_t subslice_mask;       /** $SubsliceMask */
      uint64_t gt_min_freq;         /** $GpuMinFrequency */
      uint64_t gt_max_freq;         /** $GpuMaxFrequency */
      bool     query_mode;          /** $QueryMode */
   } sys_vars;

   struct intel_device_info devinfo;

   /* OA metric sets, indexed by GUID, as know by Mesa at build time, to
    * cross-reference with the GUIDs of configs advertised by the kernel at
    * runtime
    */
   struct hash_table *oa_metrics_table;

   /* When MDAPI hasn't configured the metric we need to use by the time the
    * query begins, this OA metric is used as a fallback.
    */
   uint64_t fallback_raw_oa_metric;

   /* Whether we have support for this platform. If true && n_queries == 0,
    * this means we will not be able to use i915-perf because of it is in
    * paranoid mode.
    */
   bool platform_supported;

   /* Location of the device's sysfs entry. */
   char sysfs_dev_dir[256];

   struct {
      void *(*bo_alloc)(void *bufmgr, const char *name, uint64_t size);
      void (*bo_unreference)(void *bo);
      void *(*bo_map)(void *ctx, void *bo, unsigned flags);
      void (*bo_unmap)(void *bo);
      bool (*batch_references)(void *batch, void *bo);
      void (*bo_wait_rendering)(void *bo);
      int (*bo_busy)(void *bo);
      void (*emit_stall_at_pixel_scoreboard)(void *ctx);
      void (*emit_mi_report_perf_count)(void *ctx,
                                        void *bo,
                                        uint32_t offset_in_bytes,
                                        uint32_t report_id);
      void (*batchbuffer_flush)(void *ctx,
                                const char *file, int line);
      void (*store_register_mem)(void *ctx, void *bo, uint32_t reg, uint32_t reg_size, uint32_t offset);

   } vtbl;
};

struct intel_perf_counter_pass {
   struct intel_perf_query_info *query;
   struct intel_perf_query_counter *counter;
};

/** Initialize the intel_perf_config object for a given device.
 *
 *    include_pipeline_statistics : Whether to add a pipeline statistic query
 *                                  intel_perf_query_info object
 *
 *    use_register_snapshots : Whether the queries should include counters
 *                             that rely on register snapshots using command
 *                             streamer instructions (not possible when using
 *                             only the OA buffer data).
 */
void intel_perf_init_metrics(struct intel_perf_config *perf_cfg,
                             const struct intel_device_info *devinfo,
                             int drm_fd,
                             bool include_pipeline_statistics,
                             bool use_register_snapshots);

/** Query i915 for a metric id using guid.
 */
bool intel_perf_load_metric_id(struct intel_perf_config *perf_cfg,
                               const char *guid,
                               uint64_t *metric_id);

/** Load a configuation's content from i915 using a guid.
 */
struct intel_perf_registers *intel_perf_load_configuration(struct intel_perf_config *perf_cfg,
                                                           int fd, const char *guid);

/** Store a configuration into i915 using guid and return a new metric id.
 *
 * If guid is NULL, then a generated one will be provided by hashing the
 * content of the configuration.
 */
uint64_t intel_perf_store_configuration(struct intel_perf_config *perf_cfg, int fd,
                                        const struct intel_perf_registers *config,
                                        const char *guid);

static inline unsigned
intel_perf_query_counter_info_first_query(const struct intel_perf_query_counter_info *counter_info)
{
   return BITSET_FFS(counter_info->query_mask);
}

/** Read the slice/unslice frequency from 2 OA reports and store then into
 *  result.
 */
void intel_perf_query_result_read_frequencies(struct intel_perf_query_result *result,
                                              const struct intel_device_info *devinfo,
                                              const uint32_t *start,
                                              const uint32_t *end);

/** Store the GT frequency as reported by the RPSTAT register.
 */
void intel_perf_query_result_read_gt_frequency(struct intel_perf_query_result *result,
                                               const struct intel_device_info *devinfo,
                                               const uint32_t start,
                                               const uint32_t end);

/** Store PERFCNT registers values.
 */
void intel_perf_query_result_read_perfcnts(struct intel_perf_query_result *result,
                                           const struct intel_perf_query_info *query,
                                           const uint64_t *start,
                                           const uint64_t *end);

/** Accumulate the delta between 2 OA reports into result for a given query.
 */
void intel_perf_query_result_accumulate(struct intel_perf_query_result *result,
                                        const struct intel_perf_query_info *query,
                                        const uint32_t *start,
                                        const uint32_t *end);

/** Read the timestamp value in a report.
 */
uint64_t intel_perf_report_timestamp(const struct intel_perf_query_info *query,
                                     const uint32_t *report);

/** Accumulate the delta between 2 snapshots of OA perf registers (layout
 * should match description specified through intel_perf_query_register_layout).
 */
void intel_perf_query_result_accumulate_fields(struct intel_perf_query_result *result,
                                               const struct intel_perf_query_info *query,
                                               const void *start,
                                               const void *end,
                                               bool no_oa_accumulate);

void intel_perf_query_result_clear(struct intel_perf_query_result *result);

/** Debug helper printing out query data.
 */
void intel_perf_query_result_print_fields(const struct intel_perf_query_info *query,
                                          const void *data);

static inline size_t
intel_perf_query_counter_get_size(const struct intel_perf_query_counter *counter)
{
   switch (counter->data_type) {
   case INTEL_PERF_COUNTER_DATA_TYPE_BOOL32:
      return sizeof(uint32_t);
   case INTEL_PERF_COUNTER_DATA_TYPE_UINT32:
      return sizeof(uint32_t);
   case INTEL_PERF_COUNTER_DATA_TYPE_UINT64:
      return sizeof(uint64_t);
   case INTEL_PERF_COUNTER_DATA_TYPE_FLOAT:
      return sizeof(float);
   case INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE:
      return sizeof(double);
   default:
      unreachable("invalid counter data type");
   }
}

static inline struct intel_perf_config *
intel_perf_new(void *ctx)
{
   struct intel_perf_config *perf = rzalloc(ctx, struct intel_perf_config);
   return perf;
}

/** Whether we have the ability to hold off preemption on a batch so we don't
 * have to look at the OA buffer to subtract unrelated workloads off the
 * values captured through MI_* commands.
 */
static inline bool
intel_perf_has_hold_preemption(const struct intel_perf_config *perf)
{
   return perf->i915_perf_version >= 3;
}

/** Whether we have the ability to lock EU array power configuration for the
 * duration of the performance recording. This is useful on Gfx11 where the HW
 * architecture requires half the EU for particular workloads.
 */
static inline bool
intel_perf_has_global_sseu(const struct intel_perf_config *perf)
{
   return perf->i915_perf_version >= 4;
}

uint32_t intel_perf_get_n_passes(struct intel_perf_config *perf,
                                 const uint32_t *counter_indices,
                                 uint32_t counter_indices_count,
                                 struct intel_perf_query_info **pass_queries);
void intel_perf_get_counters_passes(struct intel_perf_config *perf,
                                    const uint32_t *counter_indices,
                                    uint32_t counter_indices_count,
                                    struct intel_perf_counter_pass *counter_pass);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INTEL_PERF_H */
