/*
 * Copyright 2020 Advanced Micro Devices, Inc.
 * Copyright 2020 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SQTT_H
#define AC_SQTT_H

#include <stdint.h>
#include <stdbool.h>

#include <assert.h>
#include "ac_rgp.h"
#include "amd_family.h"

struct radeon_cmdbuf;
struct radeon_info;

/**
 * SQ Thread tracing is a tracing mechanism that allows taking a detailed look
 * at what the shader cores are doing.
 *
 * Among the things recorded are:
 *  - draws/dispatches + state
 *  - when each wave starts and stops.
 *  - for one SIMD per SE all instructions executed on that SIMD.
 *
 * The hardware stores all these as events in a buffer, no manual barrier
 * around each command needed. The primary user of this is RGP.
 */
struct ac_sqtt {
   struct radeon_cmdbuf *start_cs[2];
   struct radeon_cmdbuf *stop_cs[2];
   /* struct radeon_winsys_bo or struct pb_buffer */
   void *bo;
   void *ptr;
   uint32_t buffer_size;
   int start_frame;
   char *trigger_file;

   uint32_t cmdbuf_ids_per_queue[AMD_NUM_IP_TYPES];

   struct rgp_code_object rgp_code_object;
   struct rgp_loader_events rgp_loader_events;
   struct rgp_pso_correlation rgp_pso_correlation;

   struct rgp_queue_info rgp_queue_info;
   struct rgp_queue_event rgp_queue_event;

   struct rgp_clock_calibration rgp_clock_calibration;

   struct hash_table_u64 *pipeline_bos;
};

#define SQTT_BUFFER_ALIGN_SHIFT 12

struct ac_sqtt_data_info {
   uint32_t cur_offset;
   uint32_t trace_status;
   union {
      uint32_t gfx9_write_counter;
      uint32_t gfx10_dropped_cntr;
   };
};

struct ac_sqtt_data_se {
   struct ac_sqtt_data_info info;
   void *data_ptr;
   uint32_t shader_engine;
   uint32_t compute_unit;
};

#define SQTT_MAX_TRACES 6

struct ac_sqtt_trace {
   const struct rgp_code_object *rgp_code_object;
   const struct rgp_loader_events *rgp_loader_events;
   const struct rgp_pso_correlation *rgp_pso_correlation;
   const struct rgp_queue_info *rgp_queue_info;
   const struct rgp_queue_event *rgp_queue_event;
   const struct rgp_clock_calibration *rgp_clock_calibration;

   uint32_t num_traces;
   struct ac_sqtt_data_se traces[SQTT_MAX_TRACES];
};

uint64_t ac_sqtt_get_info_offset(unsigned se);

uint64_t ac_sqtt_get_data_offset(const struct radeon_info *rad_info, const struct ac_sqtt *sqtt,
                                 unsigned se);
uint64_t ac_sqtt_get_info_va(uint64_t va, unsigned se);

uint64_t ac_sqtt_get_data_va(const struct radeon_info *rad_info, const struct ac_sqtt *sqtt,
                             uint64_t va, unsigned se);

void ac_sqtt_init(struct ac_sqtt *data);

void ac_sqtt_finish(struct ac_sqtt *data);

bool ac_is_sqtt_complete(const struct radeon_info *rad_info, const struct ac_sqtt *sqtt,
                         const struct ac_sqtt_data_info *info);

uint32_t ac_get_expected_buffer_size(struct radeon_info *rad_info,
                                     const struct ac_sqtt_data_info *info);

/**
 * Identifiers for RGP SQ thread-tracing markers (Table 1)
 */
enum rgp_sqtt_marker_identifier
{
   RGP_SQTT_MARKER_IDENTIFIER_EVENT = 0x0,
   RGP_SQTT_MARKER_IDENTIFIER_CB_START = 0x1,
   RGP_SQTT_MARKER_IDENTIFIER_CB_END = 0x2,
   RGP_SQTT_MARKER_IDENTIFIER_BARRIER_START = 0x3,
   RGP_SQTT_MARKER_IDENTIFIER_BARRIER_END = 0x4,
   RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT = 0x5,
   RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API = 0x6,
   RGP_SQTT_MARKER_IDENTIFIER_SYNC = 0x7,
   RGP_SQTT_MARKER_IDENTIFIER_PRESENT = 0x8,
   RGP_SQTT_MARKER_IDENTIFIER_LAYOUT_TRANSITION = 0x9,
   RGP_SQTT_MARKER_IDENTIFIER_RENDER_PASS = 0xA,
   RGP_SQTT_MARKER_IDENTIFIER_RESERVED2 = 0xB,
   RGP_SQTT_MARKER_IDENTIFIER_BIND_PIPELINE = 0xC,
   RGP_SQTT_MARKER_IDENTIFIER_RESERVED4 = 0xD,
   RGP_SQTT_MARKER_IDENTIFIER_RESERVED5 = 0xE,
   RGP_SQTT_MARKER_IDENTIFIER_RESERVED6 = 0xF
};

/**
 * Command buffer IDs used in RGP SQ thread-tracing markers (only 20 bits).
 */
union rgp_sqtt_marker_cb_id {
   struct {
      uint32_t per_frame : 1; /* Must be 1, frame-based command buffer ID. */
      uint32_t frame_index : 7;
      uint32_t cb_index : 12; /* Command buffer index within the frame. */
      uint32_t reserved : 12;
   } per_frame_cb_id;

   struct {
      uint32_t per_frame : 1; /* Must be 0, global command buffer ID. */
      uint32_t cb_index : 19; /* Global command buffer index. */
      uint32_t reserved : 12;
   } global_cb_id;

   uint32_t all;
};

/**
 * RGP SQ thread-tracing marker for the start of a command buffer. (Table 2)
 */
struct rgp_sqtt_marker_cb_start {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t cb_id : 20;
         uint32_t queue : 5;
      };
      uint32_t dword01;
   };
   union {
      uint32_t device_id_low;
      uint32_t dword02;
   };
   union {
      uint32_t device_id_high;
      uint32_t dword03;
   };
   union {
      uint32_t queue_flags;
      uint32_t dword04;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_cb_start) == 16,
              "rgp_sqtt_marker_cb_start doesn't match RGP spec");

/**
 *
 * RGP SQ thread-tracing marker for the end of a command buffer. (Table 3)
 */
struct rgp_sqtt_marker_cb_end {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t cb_id : 20;
         uint32_t reserved : 5;
      };
      uint32_t dword01;
   };
   union {
      uint32_t device_id_low;
      uint32_t dword02;
   };
   union {
      uint32_t device_id_high;
      uint32_t dword03;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_cb_end) == 12,
              "rgp_sqtt_marker_cb_end doesn't match RGP spec");

/**
 * API types used in RGP SQ thread-tracing markers for the "General API"
 * packet.
 */
enum rgp_sqtt_marker_general_api_type
{
   ApiCmdBindPipeline = 0,
   ApiCmdBindDescriptorSets = 1,
   ApiCmdBindIndexBuffer = 2,
   ApiCmdBindVertexBuffers = 3,
   ApiCmdDraw = 4,
   ApiCmdDrawIndexed = 5,
   ApiCmdDrawIndirect = 6,
   ApiCmdDrawIndexedIndirect = 7,
   ApiCmdDrawIndirectCountAMD = 8,
   ApiCmdDrawIndexedIndirectCountAMD = 9,
   ApiCmdDispatch = 10,
   ApiCmdDispatchIndirect = 11,
   ApiCmdCopyBuffer = 12,
   ApiCmdCopyImage = 13,
   ApiCmdBlitImage = 14,
   ApiCmdCopyBufferToImage = 15,
   ApiCmdCopyImageToBuffer = 16,
   ApiCmdUpdateBuffer = 17,
   ApiCmdFillBuffer = 18,
   ApiCmdClearColorImage = 19,
   ApiCmdClearDepthStencilImage = 20,
   ApiCmdClearAttachments = 21,
   ApiCmdResolveImage = 22,
   ApiCmdWaitEvents = 23,
   ApiCmdPipelineBarrier = 24,
   ApiCmdBeginQuery = 25,
   ApiCmdEndQuery = 26,
   ApiCmdResetQueryPool = 27,
   ApiCmdWriteTimestamp = 28,
   ApiCmdCopyQueryPoolResults = 29,
   ApiCmdPushConstants = 30,
   ApiCmdBeginRenderPass = 31,
   ApiCmdNextSubpass = 32,
   ApiCmdEndRenderPass = 33,
   ApiCmdExecuteCommands = 34,
   ApiCmdSetViewport = 35,
   ApiCmdSetScissor = 36,
   ApiCmdSetLineWidth = 37,
   ApiCmdSetDepthBias = 38,
   ApiCmdSetBlendConstants = 39,
   ApiCmdSetDepthBounds = 40,
   ApiCmdSetStencilCompareMask = 41,
   ApiCmdSetStencilWriteMask = 42,
   ApiCmdSetStencilReference = 43,
   ApiCmdDrawIndirectCount = 44,
   ApiCmdDrawIndexedIndirectCount = 45,
   /* gap */
   ApiCmdDrawMeshTasksEXT = 47,
   ApiCmdDrawMeshTasksIndirectCountEXT = 48,
   ApiCmdDrawMeshTasksIndirectEXT = 49,

   ApiRayTracingSeparateCompiled = 0x800000,
   ApiInvalid = 0xffffffff
};

/**
 * RGP SQ thread-tracing marker for a "General API" instrumentation packet.
 */
struct rgp_sqtt_marker_general_api {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t api_type : 20;
         uint32_t is_end : 1;
         uint32_t reserved : 4;
      };
      uint32_t dword01;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_general_api) == 4,
              "rgp_sqtt_marker_general_api doesn't match RGP spec");

/**
 * API types used in RGP SQ thread-tracing markers (Table 16).
 */
enum rgp_sqtt_marker_event_type
{
   EventCmdDraw = 0,
   EventCmdDrawIndexed = 1,
   EventCmdDrawIndirect = 2,
   EventCmdDrawIndexedIndirect = 3,
   EventCmdDrawIndirectCountAMD = 4,
   EventCmdDrawIndexedIndirectCountAMD = 5,
   EventCmdDispatch = 6,
   EventCmdDispatchIndirect = 7,
   EventCmdCopyBuffer = 8,
   EventCmdCopyImage = 9,
   EventCmdBlitImage = 10,
   EventCmdCopyBufferToImage = 11,
   EventCmdCopyImageToBuffer = 12,
   EventCmdUpdateBuffer = 13,
   EventCmdFillBuffer = 14,
   EventCmdClearColorImage = 15,
   EventCmdClearDepthStencilImage = 16,
   EventCmdClearAttachments = 17,
   EventCmdResolveImage = 18,
   EventCmdWaitEvents = 19,
   EventCmdPipelineBarrier = 20,
   EventCmdResetQueryPool = 21,
   EventCmdCopyQueryPoolResults = 22,
   EventRenderPassColorClear = 23,
   EventRenderPassDepthStencilClear = 24,
   EventRenderPassResolve = 25,
   EventInternalUnknown = 26,
   EventCmdDrawIndirectCount = 27,
   EventCmdDrawIndexedIndirectCount = 28,
   /* gap */
   EventCmdTraceRaysKHR = 30,
   EventCmdTraceRaysIndirectKHR = 31,
   EventCmdBuildAccelerationStructuresKHR = 32,
   EventCmdBuildAccelerationStructuresIndirectKHR = 33,
   EventCmdCopyAccelerationStructureKHR = 34,
   EventCmdCopyAccelerationStructureToMemoryKHR = 35,
   EventCmdCopyMemoryToAccelerationStructureKHR = 36,
   /* gap */
   EventCmdDrawMeshTasksEXT = 41,
   EventCmdDrawMeshTasksIndirectCountEXT = 42,
   EventCmdDrawMeshTasksIndirectEXT = 43,
   EventUnknown = 0x7fff,
   EventInvalid = 0xffffffff
};

/**
 * "Event (Per-draw/dispatch)" RGP SQ thread-tracing marker. (Table 4)
 */
struct rgp_sqtt_marker_event {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t api_type : 24;
         uint32_t has_thread_dims : 1;
      };
      uint32_t dword01;
   };
   union {
      struct {
         uint32_t cb_id : 20;
         uint32_t vertex_offset_reg_idx : 4;
         uint32_t instance_offset_reg_idx : 4;
         uint32_t draw_index_reg_idx : 4;
      };
      uint32_t dword02;
   };
   union {
      uint32_t cmd_id;
      uint32_t dword03;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_event) == 12,
              "rgp_sqtt_marker_event doesn't match RGP spec");

/**
 * Per-dispatch specific marker where workgroup dims are included.
 */
struct rgp_sqtt_marker_event_with_dims {
   struct rgp_sqtt_marker_event event;
   uint32_t thread_x;
   uint32_t thread_y;
   uint32_t thread_z;
};

static_assert(sizeof(struct rgp_sqtt_marker_event_with_dims) == 24,
              "rgp_sqtt_marker_event_with_dims doesn't match RGP spec");

/**
 * "Barrier Start" RGP SQTT instrumentation marker (Table 5)
 */
struct rgp_sqtt_marker_barrier_start {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t cb_id : 20;
         uint32_t reserved : 5;
      };
      uint32_t dword01;
   };
   union {
      struct {
         uint32_t driver_reason : 31;
         uint32_t internal : 1;
      };
      uint32_t dword02;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_barrier_start) == 8,
              "rgp_sqtt_marker_barrier_start doesn't match RGP spec");

/**
 * "Barrier End" RGP SQTT instrumentation marker (Table 6)
 */
struct rgp_sqtt_marker_barrier_end {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t cb_id : 20;
         uint32_t wait_on_eop_ts : 1;
         uint32_t vs_partial_flush : 1;
         uint32_t ps_partial_flush : 1;
         uint32_t cs_partial_flush : 1;
         uint32_t pfp_sync_me : 1;
      };
      uint32_t dword01;
   };
   union {
      struct {
         uint32_t sync_cp_dma : 1;
         uint32_t inval_tcp : 1;
         uint32_t inval_sqI : 1;
         uint32_t inval_sqK : 1;
         uint32_t flush_tcc : 1;
         uint32_t inval_tcc : 1;
         uint32_t flush_cb : 1;
         uint32_t inval_cb : 1;
         uint32_t flush_db : 1;
         uint32_t inval_db : 1;
         uint32_t num_layout_transitions : 16;
         uint32_t inval_gl1 : 1;
         uint32_t wait_on_ts : 1;
         uint32_t eop_ts_bottom_of_pipe : 1;
         uint32_t eos_ts_ps_done : 1;
         uint32_t eos_ts_cs_done : 1;
         uint32_t reserved : 1;
      };
      uint32_t dword02;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_barrier_end) == 8,
              "rgp_sqtt_marker_barrier_end doesn't match RGP spec");

/**
 * "Layout Transition" RGP SQTT instrumentation marker (Table 7)
 */
struct rgp_sqtt_marker_layout_transition {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t depth_stencil_expand : 1;
         uint32_t htile_hiz_range_expand : 1;
         uint32_t depth_stencil_resummarize : 1;
         uint32_t dcc_decompress : 1;
         uint32_t fmask_decompress : 1;
         uint32_t fast_clear_eliminate : 1;
         uint32_t fmask_color_expand : 1;
         uint32_t init_mask_ram : 1;
         uint32_t reserved1 : 17;
      };
      uint32_t dword01;
   };
   union {
      struct {
         uint32_t reserved2 : 32;
      };
      uint32_t dword02;
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_layout_transition) == 8,
              "rgp_sqtt_marker_layout_transition doesn't match RGP spec");


/**
 * "User Event" RGP SQTT instrumentation marker (Table 8)
 */
struct rgp_sqtt_marker_user_event {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t reserved0 : 8;
         uint32_t data_type : 8;
         uint32_t reserved1 : 12;
      };
      uint32_t dword01;
   };
};
struct rgp_sqtt_marker_user_event_with_length {
   struct rgp_sqtt_marker_user_event user_event;
   uint32_t length;
};

static_assert(sizeof(struct rgp_sqtt_marker_user_event) == 4,
              "rgp_sqtt_marker_user_event doesn't match RGP spec");

enum rgp_sqtt_marker_user_event_type
{
   UserEventTrigger = 0,
   UserEventPop,
   UserEventPush,
   UserEventObjectName,
};

/**
 * "Pipeline bind" RGP SQTT instrumentation marker (Table 12)
 */
struct rgp_sqtt_marker_pipeline_bind {
   union {
      struct {
         uint32_t identifier : 4;
         uint32_t ext_dwords : 3;
         uint32_t bind_point : 1;
         uint32_t cb_id : 20;
         uint32_t reserved : 4;
      };
      uint32_t dword01;
   };
   union {
      uint32_t api_pso_hash[2];
      struct {
         uint32_t dword02;
         uint32_t dword03;
      };
   };
};

static_assert(sizeof(struct rgp_sqtt_marker_pipeline_bind) == 12,
              "rgp_sqtt_marker_pipeline_bind doesn't match RGP spec");

bool ac_sqtt_add_pso_correlation(struct ac_sqtt *sqtt, uint64_t pipeline_hash, uint64_t api_hash);

bool ac_sqtt_add_code_object_loader_event(struct ac_sqtt *sqtt, uint64_t pipeline_hash,
                                          uint64_t base_address);

bool ac_sqtt_add_clock_calibration(struct ac_sqtt *sqtt, uint64_t cpu_timestamp,
                                   uint64_t gpu_timestamp);

bool ac_check_profile_state(const struct radeon_info *info);

union rgp_sqtt_marker_cb_id ac_sqtt_get_next_cmdbuf_id(struct ac_sqtt *sqtt,
                                                       enum amd_ip_type ip_type);

bool ac_sqtt_se_is_disabled(const struct radeon_info *info, unsigned se);

bool ac_sqtt_get_trace(struct ac_sqtt *sqtt, const struct radeon_info *info,
                       struct ac_sqtt_trace *sqtt_trace);

uint32_t ac_sqtt_get_shader_mask(const struct radeon_info *info);

uint32_t ac_sqtt_get_active_cu(const struct radeon_info *info, unsigned se);

#endif
