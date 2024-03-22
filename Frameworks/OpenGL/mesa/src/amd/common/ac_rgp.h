/*
 * Copyright 2020 Advanced Micro Devices, Inc.
 * Copyright 2020 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_RGP_H
#define AC_RGP_H

#include <stdint.h>
#include "compiler/shader_enums.h"
#include "util/list.h"
#include "util/simple_mtx.h"

struct radeon_info;
struct ac_sqtt_trace;
struct ac_sqtt;
struct ac_spm_trace;

enum rgp_hardware_stages {
   RGP_HW_STAGE_VS = 0,
   RGP_HW_STAGE_LS,
   RGP_HW_STAGE_HS,
   RGP_HW_STAGE_ES,
   RGP_HW_STAGE_GS,
   RGP_HW_STAGE_PS,
   RGP_HW_STAGE_CS,
   RGP_HW_STAGE_MAX,
};

struct rgp_shader_data {
   uint64_t hash[2];
   uint32_t code_size;
   uint8_t *code;
   uint32_t vgpr_count;
   uint32_t sgpr_count;
   uint32_t scratch_memory_size;
   uint32_t lds_size;
   uint32_t wavefront_size;
   uint64_t base_address;
   uint32_t elf_symbol_offset;
   uint32_t hw_stage;
   uint32_t is_combined;
   char rt_shader_name[32];
   uint32_t rt_stack_size;
};

struct rgp_code_object_record {
   uint32_t shader_stages_mask;
   struct rgp_shader_data shader_data[MESA_VULKAN_SHADER_STAGES];
   uint32_t num_shaders_combined; /* count combined shaders as one count */
   uint64_t pipeline_hash[2];

   bool is_rt;
   struct list_head list;
};

struct rgp_code_object {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

enum rgp_loader_event_type
{
   RGP_LOAD_TO_GPU_MEMORY = 0,
   RGP_UNLOAD_FROM_GPU_MEMORY,
};

struct rgp_loader_events_record {
   uint32_t loader_event_type;
   uint32_t reserved;
   uint64_t base_address;
   uint64_t code_object_hash[2];
   uint64_t time_stamp;
   struct list_head list;
};

struct rgp_loader_events {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

struct rgp_pso_correlation_record {
   uint64_t api_pso_hash;
   uint64_t pipeline_hash[2];
   char api_level_obj_name[64];
   struct list_head list;
};

struct rgp_pso_correlation {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

enum sqtt_queue_type {
   SQTT_QUEUE_TYPE_UNKNOWN   = 0x0,
   SQTT_QUEUE_TYPE_UNIVERSAL = 0x1,
   SQTT_QUEUE_TYPE_COMPUTE   = 0x2,
   SQTT_QUEUE_TYPE_DMA       = 0x3,
};

enum sqtt_engine_type {
   SQTT_ENGINE_TYPE_UNKNOWN                 = 0x0,
   SQTT_ENGINE_TYPE_UNIVERSAL               = 0x1,
   SQTT_ENGINE_TYPE_COMPUTE                 = 0x2,
   SQTT_ENGINE_TYPE_EXCLUSIVE_COMPUTE       = 0x3,
   SQTT_ENGINE_TYPE_DMA                     = 0x4,
   SQTT_ENGINE_TYPE_HIGH_PRIORITY_UNIVERSAL = 0x7,
   SQTT_ENGINE_TYPE_HIGH_PRIORITY_GRAPHICS  = 0x8,
};

struct sqtt_queue_hardware_info {
   union {
      struct {
         enum sqtt_queue_type queue_type : 8;
         enum sqtt_engine_type engine_type : 8;
         uint32_t reserved : 16;
      };
      uint32_t value;
   };
};

struct rgp_queue_info_record {
   uint64_t queue_id;
   uint64_t queue_context;
   struct sqtt_queue_hardware_info hardware_info;
   uint32_t reserved;
   struct list_head list;
};

struct rgp_queue_info {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

enum sqtt_queue_event_type {
   SQTT_QUEUE_TIMING_EVENT_CMDBUF_SUBMIT,
   SQTT_QUEUE_TIMING_EVENT_SIGNAL_SEMAPHORE,
   SQTT_QUEUE_TIMING_EVENT_WAIT_SEMAPHORE,
   SQTT_QUEUE_TIMING_EVENT_PRESENT
};

struct rgp_queue_event_record {
   enum sqtt_queue_event_type event_type;
   uint32_t sqtt_cb_id;
   uint64_t frame_index;
   uint32_t queue_info_index;
   uint32_t submit_sub_index;
   uint64_t api_id;
   uint64_t cpu_timestamp;
   uint64_t *gpu_timestamps[2];
   struct list_head list;
};

struct rgp_queue_event {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

struct rgp_clock_calibration_record {
   uint64_t cpu_timestamp;
   uint64_t gpu_timestamp;
   struct list_head list;
};

struct rgp_clock_calibration {
   uint32_t record_count;
   struct list_head record;
   simple_mtx_t lock;
};

int ac_dump_rgp_capture(const struct radeon_info *info, struct ac_sqtt_trace *sqtt_trace,
                        const struct ac_spm_trace *spm_trace);

void
ac_rgp_file_write_elf_object(FILE *output, size_t file_elf_start,
                             struct rgp_code_object_record *record,
                             uint32_t *written_size, uint32_t flags);

#endif
