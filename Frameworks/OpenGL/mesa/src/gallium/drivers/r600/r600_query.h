/*
 * Copyright 2015 Advanced Micro Devices, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *  Nicolai Hähnle <nicolai.haehnle@amd.com>
 *
 */

#ifndef R600_QUERY_H
#define R600_QUERY_H

#include "util/u_threaded_context.h"

struct pipe_context;
struct pipe_query;
struct pipe_resource;

struct r600_common_context;
struct r600_common_screen;
struct r600_query;
struct r600_query_hw;
struct r600_resource;

enum {
	R600_QUERY_DRAW_CALLS = PIPE_QUERY_DRIVER_SPECIFIC,
	R600_QUERY_DECOMPRESS_CALLS,
	R600_QUERY_MRT_DRAW_CALLS,
	R600_QUERY_PRIM_RESTART_CALLS,
	R600_QUERY_SPILL_DRAW_CALLS,
	R600_QUERY_COMPUTE_CALLS,
	R600_QUERY_SPILL_COMPUTE_CALLS,
	R600_QUERY_DMA_CALLS,
	R600_QUERY_CP_DMA_CALLS,
	R600_QUERY_NUM_VS_FLUSHES,
	R600_QUERY_NUM_PS_FLUSHES,
	R600_QUERY_NUM_CS_FLUSHES,
	R600_QUERY_NUM_CB_CACHE_FLUSHES,
	R600_QUERY_NUM_DB_CACHE_FLUSHES,
	R600_QUERY_NUM_RESIDENT_HANDLES,
	R600_QUERY_TC_OFFLOADED_SLOTS,
	R600_QUERY_TC_DIRECT_SLOTS,
	R600_QUERY_TC_NUM_SYNCS,
	R600_QUERY_CS_THREAD_BUSY,
	R600_QUERY_GALLIUM_THREAD_BUSY,
	R600_QUERY_REQUESTED_VRAM,
	R600_QUERY_REQUESTED_GTT,
	R600_QUERY_MAPPED_VRAM,
	R600_QUERY_MAPPED_GTT,
	R600_QUERY_BUFFER_WAIT_TIME,
	R600_QUERY_NUM_MAPPED_BUFFERS,
	R600_QUERY_NUM_GFX_IBS,
	R600_QUERY_NUM_SDMA_IBS,
	R600_QUERY_GFX_BO_LIST_SIZE,
	R600_QUERY_NUM_BYTES_MOVED,
	R600_QUERY_NUM_EVICTIONS,
	R600_QUERY_NUM_VRAM_CPU_PAGE_FAULTS,
	R600_QUERY_VRAM_USAGE,
	R600_QUERY_VRAM_VIS_USAGE,
	R600_QUERY_GTT_USAGE,
	R600_QUERY_GPU_TEMPERATURE,
	R600_QUERY_CURRENT_GPU_SCLK,
	R600_QUERY_CURRENT_GPU_MCLK,
	R600_QUERY_GPU_LOAD,
	R600_QUERY_GPU_SHADERS_BUSY,
	R600_QUERY_GPU_TA_BUSY,
	R600_QUERY_GPU_GDS_BUSY,
	R600_QUERY_GPU_VGT_BUSY,
	R600_QUERY_GPU_IA_BUSY,
	R600_QUERY_GPU_SX_BUSY,
	R600_QUERY_GPU_WD_BUSY,
	R600_QUERY_GPU_BCI_BUSY,
	R600_QUERY_GPU_SC_BUSY,
	R600_QUERY_GPU_PA_BUSY,
	R600_QUERY_GPU_DB_BUSY,
	R600_QUERY_GPU_CP_BUSY,
	R600_QUERY_GPU_CB_BUSY,
	R600_QUERY_GPU_SDMA_BUSY,
	R600_QUERY_GPU_PFP_BUSY,
	R600_QUERY_GPU_MEQ_BUSY,
	R600_QUERY_GPU_ME_BUSY,
	R600_QUERY_GPU_SURF_SYNC_BUSY,
	R600_QUERY_GPU_CP_DMA_BUSY,
	R600_QUERY_GPU_SCRATCH_RAM_BUSY,
	R600_QUERY_NUM_COMPILATIONS,
	R600_QUERY_NUM_SHADERS_CREATED,
	R600_QUERY_NUM_SHADER_CACHE_HITS,
	R600_QUERY_GPIN_ASIC_ID,
	R600_QUERY_GPIN_NUM_SIMD,
	R600_QUERY_GPIN_NUM_RB,
	R600_QUERY_GPIN_NUM_SPI,
	R600_QUERY_GPIN_NUM_SE,

	R600_QUERY_FIRST_PERFCOUNTER = PIPE_QUERY_DRIVER_SPECIFIC + 100,
};

enum {
	R600_QUERY_GROUP_GPIN = 0,
	R600_NUM_SW_QUERY_GROUPS
};

struct r600_query_ops {
	void (*destroy)(struct r600_common_screen *, struct r600_query *);
	bool (*begin)(struct r600_common_context *, struct r600_query *);
	bool (*end)(struct r600_common_context *, struct r600_query *);
	bool (*get_result)(struct r600_common_context *,
			   struct r600_query *, bool wait,
			   union pipe_query_result *result);
	void (*get_result_resource)(struct r600_common_context *,
				    struct r600_query *,
				    enum pipe_query_flags flags,
				    enum pipe_query_value_type result_type,
				    int index,
				    struct pipe_resource *resource,
				    unsigned offset);
};

struct r600_query {
	struct threaded_query b;
	struct r600_query_ops *ops;

	/* The type of query */
	unsigned type;
};

enum {
	R600_QUERY_HW_FLAG_NO_START = (1 << 0),
	/* gap */
	/* whether begin_query doesn't clear the result */
	R600_QUERY_HW_FLAG_BEGIN_RESUMES = (1 << 2),
};

struct r600_query_hw_ops {
	bool (*prepare_buffer)(struct r600_common_screen *,
			       struct r600_query_hw *,
			       struct r600_resource *);
	void (*emit_start)(struct r600_common_context *,
			   struct r600_query_hw *,
			   struct r600_resource *buffer, uint64_t va);
	void (*emit_stop)(struct r600_common_context *,
			  struct r600_query_hw *,
			  struct r600_resource *buffer, uint64_t va);
	void (*clear_result)(struct r600_query_hw *, union pipe_query_result *);
	void (*add_result)(struct r600_common_screen *screen,
			   struct r600_query_hw *, void *buffer,
			   union pipe_query_result *result);
};

struct r600_query_buffer {
	/* The buffer where query results are stored. */
	struct r600_resource		*buf;
	/* Offset of the next free result after current query data */
	unsigned			results_end;
	/* If a query buffer is full, a new buffer is created and the old one
	 * is put in here. When we calculate the result, we sum up the samples
	 * from all buffers. */
	struct r600_query_buffer	*previous;
};

struct r600_query_hw {
	struct r600_query b;
	struct r600_query_hw_ops *ops;
	unsigned flags;

	/* The query buffer and how many results are in it. */
	struct r600_query_buffer buffer;
	/* Size of the result in memory for both begin_query and end_query,
	 * this can be one or two numbers, or it could even be a size of a structure. */
	unsigned result_size;
	/* The number of dwords for begin_query or end_query. */
	unsigned num_cs_dw_begin;
	unsigned num_cs_dw_end;
	/* Linked list of queries */
	struct list_head list;
	/* For transform feedback: which stream the query is for */
	unsigned stream;
};

bool r600_query_hw_init(struct r600_common_screen *rscreen,
			struct r600_query_hw *query);
void r600_query_hw_destroy(struct r600_common_screen *rscreen,
			   struct r600_query *rquery);
bool r600_query_hw_begin(struct r600_common_context *rctx,
			 struct r600_query *rquery);
bool r600_query_hw_end(struct r600_common_context *rctx,
		       struct r600_query *rquery);
bool r600_query_hw_get_result(struct r600_common_context *rctx,
			      struct r600_query *rquery,
			      bool wait,
			      union pipe_query_result *result);

/* Performance counters */
enum {
	/* This block is part of the shader engine */
	R600_PC_BLOCK_SE = (1 << 0),

	/* Expose per-instance groups instead of summing all instances (within
	 * an SE). */
	R600_PC_BLOCK_INSTANCE_GROUPS = (1 << 1),

	/* Expose per-SE groups instead of summing instances across SEs. */
	R600_PC_BLOCK_SE_GROUPS = (1 << 2),

	/* Shader block */
	R600_PC_BLOCK_SHADER = (1 << 3),

	/* Non-shader block with perfcounters windowed by shaders. */
	R600_PC_BLOCK_SHADER_WINDOWED = (1 << 4),
};

/* Describes a hardware block with performance counters. Multiple instances of
 * each block, possibly per-SE, may exist on the chip. Depending on the block
 * and on the user's configuration, we either
 *  (a) expose every instance as a performance counter group,
 *  (b) expose a single performance counter group that reports the sum over all
 *      instances, or
 *  (c) expose one performance counter group per instance, but summed over all
 *      shader engines.
 */
struct r600_perfcounter_block {
	const char *basename;
	unsigned flags;
	unsigned num_counters;
	unsigned num_selectors;
	unsigned num_instances;

	unsigned num_groups;
	char *group_names;
	unsigned group_name_stride;

	char *selector_names;
	unsigned selector_name_stride;

	void *data;
};

struct r600_perfcounters {
	unsigned num_groups;
	unsigned num_blocks;
	struct r600_perfcounter_block *blocks;

	unsigned num_start_cs_dwords;
	unsigned num_stop_cs_dwords;
	unsigned num_instance_cs_dwords;
	unsigned num_shaders_cs_dwords;

	unsigned num_shader_types;
	const char * const *shader_type_suffixes;
	const unsigned *shader_type_bits;

	void (*get_size)(struct r600_perfcounter_block *,
			 unsigned count, unsigned *selectors,
			 unsigned *num_select_dw, unsigned *num_read_dw);

	void (*emit_instance)(struct r600_common_context *,
			      int se, int instance);
	void (*emit_shaders)(struct r600_common_context *, unsigned shaders);
	void (*emit_select)(struct r600_common_context *,
			    struct r600_perfcounter_block *,
			    unsigned count, unsigned *selectors);
	void (*emit_start)(struct r600_common_context *,
			  struct r600_resource *buffer, uint64_t va);
	void (*emit_stop)(struct r600_common_context *,
			  struct r600_resource *buffer, uint64_t va);
	void (*emit_read)(struct r600_common_context *,
			  struct r600_perfcounter_block *,
			  unsigned count, unsigned *selectors,
			  struct r600_resource *buffer, uint64_t va);

	void (*cleanup)(struct r600_common_screen *);

	bool separate_se;
	bool separate_instance;
};

struct pipe_query *r600_create_batch_query(struct pipe_context *ctx,
					   unsigned num_queries,
					   unsigned *query_types);

int r600_get_perfcounter_info(struct r600_common_screen *,
			      unsigned index,
			      struct pipe_driver_query_info *info);
int r600_get_perfcounter_group_info(struct r600_common_screen *,
				    unsigned index,
				    struct pipe_driver_query_group_info *info);

bool r600_perfcounters_init(struct r600_perfcounters *, unsigned num_blocks);
void r600_perfcounters_add_block(struct r600_common_screen *,
				 struct r600_perfcounters *,
				 const char *name, unsigned flags,
				 unsigned counters, unsigned selectors,
				 unsigned instances, void *data);
void r600_perfcounters_do_destroy(struct r600_perfcounters *);
void r600_query_hw_reset_buffers(struct r600_common_context *rctx,
				 struct r600_query_hw *query);

struct r600_qbo_state {
	void *saved_compute;
	struct pipe_constant_buffer saved_const0;
	struct pipe_shader_buffer saved_ssbo[3];
};

#endif /* R600_QUERY_H */
