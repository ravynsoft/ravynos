/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_QUERY_H
#define SI_QUERY_H

#include "util/u_threaded_context.h"

#include "ac_perfcounter.h"

struct pipe_context;
struct pipe_query;
struct pipe_resource;

struct si_screen;
struct si_context;
struct si_query;
struct si_query_buffer;
struct si_query_hw;
struct si_resource;

#define SI_MAX_STREAMS 4

enum
{
   SI_QUERY_DRAW_CALLS = PIPE_QUERY_DRIVER_SPECIFIC,
   SI_QUERY_DECOMPRESS_CALLS,
   SI_QUERY_COMPUTE_CALLS,
   SI_QUERY_CP_DMA_CALLS,
   SI_QUERY_NUM_VS_FLUSHES,
   SI_QUERY_NUM_PS_FLUSHES,
   SI_QUERY_NUM_CS_FLUSHES,
   SI_QUERY_NUM_CB_CACHE_FLUSHES,
   SI_QUERY_NUM_DB_CACHE_FLUSHES,
   SI_QUERY_NUM_L2_INVALIDATES,
   SI_QUERY_NUM_L2_WRITEBACKS,
   SI_QUERY_NUM_RESIDENT_HANDLES,
   SI_QUERY_TC_OFFLOADED_SLOTS,
   SI_QUERY_TC_DIRECT_SLOTS,
   SI_QUERY_TC_NUM_SYNCS,
   SI_QUERY_CS_THREAD_BUSY,
   SI_QUERY_GALLIUM_THREAD_BUSY,
   SI_QUERY_REQUESTED_VRAM,
   SI_QUERY_REQUESTED_GTT,
   SI_QUERY_MAPPED_VRAM,
   SI_QUERY_MAPPED_GTT,
   SI_QUERY_SLAB_WASTED_VRAM,
   SI_QUERY_SLAB_WASTED_GTT,
   SI_QUERY_BUFFER_WAIT_TIME,
   SI_QUERY_NUM_MAPPED_BUFFERS,
   SI_QUERY_NUM_GFX_IBS,
   SI_QUERY_GFX_BO_LIST_SIZE,
   SI_QUERY_GFX_IB_SIZE,
   SI_QUERY_NUM_BYTES_MOVED,
   SI_QUERY_NUM_EVICTIONS,
   SI_QUERY_NUM_VRAM_CPU_PAGE_FAULTS,
   SI_QUERY_VRAM_USAGE,
   SI_QUERY_VRAM_VIS_USAGE,
   SI_QUERY_GTT_USAGE,
   SI_QUERY_GPU_TEMPERATURE,
   SI_QUERY_CURRENT_GPU_SCLK,
   SI_QUERY_CURRENT_GPU_MCLK,
   SI_QUERY_GPU_LOAD,
   SI_QUERY_GPU_SHADERS_BUSY,
   SI_QUERY_GPU_TA_BUSY,
   SI_QUERY_GPU_GDS_BUSY,
   SI_QUERY_GPU_VGT_BUSY,
   SI_QUERY_GPU_IA_BUSY,
   SI_QUERY_GPU_SX_BUSY,
   SI_QUERY_GPU_WD_BUSY,
   SI_QUERY_GPU_BCI_BUSY,
   SI_QUERY_GPU_SC_BUSY,
   SI_QUERY_GPU_PA_BUSY,
   SI_QUERY_GPU_DB_BUSY,
   SI_QUERY_GPU_CP_BUSY,
   SI_QUERY_GPU_CB_BUSY,
   SI_QUERY_GPU_SDMA_BUSY,
   SI_QUERY_GPU_PFP_BUSY,
   SI_QUERY_GPU_MEQ_BUSY,
   SI_QUERY_GPU_ME_BUSY,
   SI_QUERY_GPU_SURF_SYNC_BUSY,
   SI_QUERY_GPU_CP_DMA_BUSY,
   SI_QUERY_GPU_SCRATCH_RAM_BUSY,
   SI_QUERY_NUM_COMPILATIONS,
   SI_QUERY_NUM_SHADERS_CREATED,
   SI_QUERY_BACK_BUFFER_PS_DRAW_RATIO,
   SI_QUERY_GPIN_ASIC_ID,
   SI_QUERY_GPIN_NUM_SIMD,
   SI_QUERY_GPIN_NUM_RB,
   SI_QUERY_GPIN_NUM_SPI,
   SI_QUERY_GPIN_NUM_SE,
   SI_QUERY_LIVE_SHADER_CACHE_HITS,
   SI_QUERY_LIVE_SHADER_CACHE_MISSES,
   SI_QUERY_MEMORY_SHADER_CACHE_HITS,
   SI_QUERY_MEMORY_SHADER_CACHE_MISSES,
   SI_QUERY_DISK_SHADER_CACHE_HITS,
   SI_QUERY_DISK_SHADER_CACHE_MISSES,

   SI_QUERY_FIRST_PERFCOUNTER = PIPE_QUERY_DRIVER_SPECIFIC + 100,
};

enum
{
   SI_QUERY_GROUP_GPIN = 0,
   SI_NUM_SW_QUERY_GROUPS
};

struct si_query_ops {
   void (*destroy)(struct si_context *, struct si_query *);
   bool (*begin)(struct si_context *, struct si_query *);
   bool (*end)(struct si_context *, struct si_query *);
   bool (*get_result)(struct si_context *, struct si_query *, bool wait,
                      union pipe_query_result *result);
   void (*get_result_resource)(struct si_context *, struct si_query *,
                               enum pipe_query_flags flags,
                               enum pipe_query_value_type result_type, int index,
                               struct pipe_resource *resource, unsigned offset);

   void (*suspend)(struct si_context *, struct si_query *);
   void (*resume)(struct si_context *, struct si_query *);
};

struct si_query {
   struct threaded_query b;
   const struct si_query_ops *ops;

   /* The PIPE_QUERY_xxx type of query */
   unsigned type;

   /* The number of dwords for suspend. */
   unsigned num_cs_dw_suspend;

   /* Linked list of queries that must be suspended at end of CS. */
   struct list_head active_list;
};

enum
{
   SI_QUERY_HW_FLAG_NO_START = (1 << 0),
   /* gap */
   /* whether begin_query doesn't clear the result */
   SI_QUERY_HW_FLAG_BEGIN_RESUMES = (1 << 2),
   /* whether GS invocations and emitted primitives counters are emulated
    * using atomic adds.
    */
   SI_QUERY_EMULATE_GS_COUNTERS = (1 << 3),
};

struct si_query_hw_ops {
   bool (*prepare_buffer)(struct si_context *, struct si_query_buffer *);
   void (*emit_start)(struct si_context *, struct si_query_hw *, struct si_resource *buffer,
                      uint64_t va);
   void (*emit_stop)(struct si_context *, struct si_query_hw *, struct si_resource *buffer,
                     uint64_t va);
   void (*clear_result)(struct si_query_hw *, union pipe_query_result *);
   void (*add_result)(struct si_screen *screen, struct si_query_hw *, void *buffer,
                      union pipe_query_result *result);
};

struct si_query_buffer {
   /* The buffer where query results are stored. */
   struct si_resource *buf;
   /* If a query buffer is full, a new buffer is created and the old one
    * is put in here. When we calculate the result, we sum up the samples
    * from all buffers. */
   struct si_query_buffer *previous;
   /* Offset of the next free result after current query data */
   unsigned results_end;
   bool unprepared;
};

void si_query_buffer_destroy(struct si_screen *sctx, struct si_query_buffer *buffer);
void si_query_buffer_reset(struct si_context *sctx, struct si_query_buffer *buffer);
bool si_query_buffer_alloc(struct si_context *sctx, struct si_query_buffer *buffer,
                           bool (*prepare_buffer)(struct si_context *, struct si_query_buffer *),
                           unsigned size);

struct si_query_hw {
   struct si_query b;
   const struct si_query_hw_ops *ops;
   unsigned flags;

   /* The query buffer and how many results are in it. */
   struct si_query_buffer buffer;
   /* Size of the result in memory for both begin_query and end_query,
    * this can be one or two numbers, or it could even be a size of a structure. */
   unsigned result_size;
   union {
      /* For transform feedback: which stream the query is for */
      unsigned stream;
      /* For pipeline stats: which counter is active */
      unsigned index;
   };

   /* Workaround via compute shader */
   struct si_resource *workaround_buf;
   unsigned workaround_offset;
};

unsigned si_query_pipestat_end_dw_offset(struct si_screen *sscreen,
                                         enum pipe_statistics_query_index index);

/* Shader-based queries */

/**
 * The query buffer is written to by ESGS NGG shaders with statistics about
 * generated and (streamout-)emitted primitives.
 *
 * The context maintains a ring of these query buffers, and queries simply
 * point into the ring, allowing an arbitrary number of queries to be active
 * without additional GPU cost.
 */
struct gfx11_sh_query_buffer {
   struct list_head list;
   struct si_resource *buf;
   unsigned refcount;

   /* Offset into the buffer in bytes; points at the first un-emitted entry. */
   unsigned head;
};

/* Memory layout of the query buffer. Must be kept in sync with shaders
 * (including QBO shaders) and should be aligned to cachelines.
 *
 * The somewhat awkward memory layout is for compatibility with the
 * SET_PREDICATION packet, which also means that we're setting the high bit
 * of all those values unconditionally.
 */
struct gfx11_sh_query_buffer_mem {
   struct {
      uint64_t generated_primitives_start_dummy;
      uint64_t emitted_primitives_start_dummy;
      uint64_t generated_primitives;
      uint64_t emitted_primitives;
   } stream[4];
   uint32_t fence; /* bottom-of-pipe fence: set to ~0 when draws have finished */
   uint32_t pad[31];
};

struct gfx11_sh_query {
   struct si_query b;

   struct gfx11_sh_query_buffer *first;
   struct gfx11_sh_query_buffer *last;
   unsigned first_begin;
   unsigned last_end;

   unsigned stream;
};

struct pipe_query *gfx11_sh_query_create(struct si_screen *screen, enum pipe_query_type query_type,
                                         unsigned index);

/* Performance counters */
struct si_perfcounters {
   struct ac_perfcounters base;

   unsigned num_stop_cs_dwords;
   unsigned num_instance_cs_dwords;
};

struct pipe_query *si_create_batch_query(struct pipe_context *ctx, unsigned num_queries,
                                         unsigned *query_types);

int si_get_perfcounter_info(struct si_screen *, unsigned index,
                            struct pipe_driver_query_info *info);
int si_get_perfcounter_group_info(struct si_screen *, unsigned index,
                                  struct pipe_driver_query_group_info *info);

struct si_qbo_state {
   struct pipe_constant_buffer saved_const0;
};

#endif /* SI_QUERY_H */
