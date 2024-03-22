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

#include "util/u_memory.h"
#include "r600_query.h"
#include "r600_pipe_common.h"
#include "r600d_common.h"

/* Max counters per HW block */
#define R600_QUERY_MAX_COUNTERS 16

static struct r600_perfcounter_block *
lookup_counter(struct r600_perfcounters *pc, unsigned index,
	       unsigned *base_gid, unsigned *sub_index)
{
	struct r600_perfcounter_block *block = pc->blocks;
	unsigned bid;

	*base_gid = 0;
	for (bid = 0; bid < pc->num_blocks; ++bid, ++block) {
		unsigned total = block->num_groups * block->num_selectors;

		if (index < total) {
			*sub_index = index;
			return block;
		}

		index -= total;
		*base_gid += block->num_groups;
	}

	return NULL;
}

static struct r600_perfcounter_block *
lookup_group(struct r600_perfcounters *pc, unsigned *index)
{
	unsigned bid;
	struct r600_perfcounter_block *block = pc->blocks;

	for (bid = 0; bid < pc->num_blocks; ++bid, ++block) {
		if (*index < block->num_groups)
			return block;
		*index -= block->num_groups;
	}

	return NULL;
}

struct r600_pc_group {
	struct r600_pc_group *next;
	struct r600_perfcounter_block *block;
	unsigned sub_gid; /* only used during init */
	unsigned result_base; /* only used during init */
	int se;
	int instance;
	unsigned num_counters;
	unsigned selectors[R600_QUERY_MAX_COUNTERS];
};

struct r600_pc_counter {
	unsigned base;
	unsigned qwords;
	unsigned stride; /* in uint64s */
};

#define R600_PC_SHADERS_WINDOWING (1 << 31)

struct r600_query_pc {
	struct r600_query_hw b;

	unsigned shaders;
	unsigned num_counters;
	struct r600_pc_counter *counters;
	struct r600_pc_group *groups;
};

static void r600_pc_query_destroy(struct r600_common_screen *rscreen,
				  struct r600_query *rquery)
{
	struct r600_query_pc *query = (struct r600_query_pc *)rquery;

	while (query->groups) {
		struct r600_pc_group *group = query->groups;
		query->groups = group->next;
		FREE(group);
	}

	FREE(query->counters);

	r600_query_hw_destroy(rscreen, rquery);
}

static bool r600_pc_query_prepare_buffer(struct r600_common_screen *screen,
					 struct r600_query_hw *hwquery,
					 struct r600_resource *buffer)
{
	/* no-op */
	return true;
}

static void r600_pc_query_emit_start(struct r600_common_context *ctx,
				     struct r600_query_hw *hwquery,
				     struct r600_resource *buffer, uint64_t va)
{
	struct r600_perfcounters *pc = ctx->screen->perfcounters;
	struct r600_query_pc *query = (struct r600_query_pc *)hwquery;
	struct r600_pc_group *group;
	int current_se = -1;
	int current_instance = -1;

	if (query->shaders)
		pc->emit_shaders(ctx, query->shaders);

	for (group = query->groups; group; group = group->next) {
		struct r600_perfcounter_block *block = group->block;

		if (group->se != current_se || group->instance != current_instance) {
			current_se = group->se;
			current_instance = group->instance;
			pc->emit_instance(ctx, group->se, group->instance);
		}

		pc->emit_select(ctx, block, group->num_counters, group->selectors);
	}

	if (current_se != -1 || current_instance != -1)
		pc->emit_instance(ctx, -1, -1);

	pc->emit_start(ctx, buffer, va);
}

static void r600_pc_query_emit_stop(struct r600_common_context *ctx,
				    struct r600_query_hw *hwquery,
				    struct r600_resource *buffer, uint64_t va)
{
	struct r600_perfcounters *pc = ctx->screen->perfcounters;
	struct r600_query_pc *query = (struct r600_query_pc *)hwquery;
	struct r600_pc_group *group;

	pc->emit_stop(ctx, buffer, va);

	for (group = query->groups; group; group = group->next) {
		struct r600_perfcounter_block *block = group->block;
		unsigned se = group->se >= 0 ? group->se : 0;
		unsigned se_end = se + 1;

		if ((block->flags & R600_PC_BLOCK_SE) && (group->se < 0))
			se_end = ctx->screen->info.max_se;

		do {
			unsigned instance = group->instance >= 0 ? group->instance : 0;

			do {
				pc->emit_instance(ctx, se, instance);
				pc->emit_read(ctx, block,
					      group->num_counters, group->selectors,
					      buffer, va);
				va += sizeof(uint64_t) * group->num_counters;
			} while (group->instance < 0 && ++instance < block->num_instances);
		} while (++se < se_end);
	}

	pc->emit_instance(ctx, -1, -1);
}

static void r600_pc_query_clear_result(struct r600_query_hw *hwquery,
				       union pipe_query_result *result)
{
	struct r600_query_pc *query = (struct r600_query_pc *)hwquery;

	memset(result, 0, sizeof(result->batch[0]) * query->num_counters);
}

static void r600_pc_query_add_result(struct r600_common_screen *rscreen,
				     struct r600_query_hw *hwquery,
				     void *buffer,
				     union pipe_query_result *result)
{
	struct r600_query_pc *query = (struct r600_query_pc *)hwquery;
	uint64_t *results = buffer;
	unsigned i, j;

	for (i = 0; i < query->num_counters; ++i) {
		struct r600_pc_counter *counter = &query->counters[i];

		for (j = 0; j < counter->qwords; ++j) {
			uint32_t value = results[counter->base + j * counter->stride];
			result->batch[i].u64 += value;
		}
	}
}

static struct r600_query_ops batch_query_ops = {
	.destroy = r600_pc_query_destroy,
	.begin = r600_query_hw_begin,
	.end = r600_query_hw_end,
	.get_result = r600_query_hw_get_result
};

static struct r600_query_hw_ops batch_query_hw_ops = {
	.prepare_buffer = r600_pc_query_prepare_buffer,
	.emit_start = r600_pc_query_emit_start,
	.emit_stop = r600_pc_query_emit_stop,
	.clear_result = r600_pc_query_clear_result,
	.add_result = r600_pc_query_add_result,
};

static struct r600_pc_group *get_group_state(struct r600_common_screen *screen,
					     struct r600_query_pc *query,
					     struct r600_perfcounter_block *block,
					     unsigned sub_gid)
{
	struct r600_pc_group *group = query->groups;

	while (group) {
		if (group->block == block && group->sub_gid == sub_gid)
			return group;
		group = group->next;
	}

	group = CALLOC_STRUCT(r600_pc_group);
	if (!group)
		return NULL;

	group->block = block;
	group->sub_gid = sub_gid;

	if (block->flags & R600_PC_BLOCK_SHADER) {
		unsigned sub_gids = block->num_instances;
		unsigned shader_id;
		unsigned shaders;
		unsigned query_shaders;

		if (block->flags & R600_PC_BLOCK_SE_GROUPS)
			sub_gids = sub_gids * screen->info.max_se;
		shader_id = sub_gid / sub_gids;
		sub_gid = sub_gid % sub_gids;

		shaders = screen->perfcounters->shader_type_bits[shader_id];

		query_shaders = query->shaders & ~R600_PC_SHADERS_WINDOWING;
		if (query_shaders && query_shaders != shaders) {
			fprintf(stderr, "r600_perfcounter: incompatible shader groups\n");
			FREE(group);
			return NULL;
		}
		query->shaders = shaders;
	}

	if (block->flags & R600_PC_BLOCK_SHADER_WINDOWED && !query->shaders) {
		// A non-zero value in query->shaders ensures that the shader
		// masking is reset unless the user explicitly requests one.
		query->shaders = R600_PC_SHADERS_WINDOWING;
	}

	if (block->flags & R600_PC_BLOCK_SE_GROUPS) {
		group->se = sub_gid / block->num_instances;
		sub_gid = sub_gid % block->num_instances;
	} else {
		group->se = -1;
	}

	if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS) {
		group->instance = sub_gid;
	} else {
		group->instance = -1;
	}

	group->next = query->groups;
	query->groups = group;

	return group;
}

struct pipe_query *r600_create_batch_query(struct pipe_context *ctx,
					   unsigned num_queries,
					   unsigned *query_types)
{
	struct r600_common_screen *screen =
		(struct r600_common_screen *)ctx->screen;
	struct r600_perfcounters *pc = screen->perfcounters;
	struct r600_perfcounter_block *block;
	struct r600_pc_group *group;
	struct r600_query_pc *query;
	unsigned base_gid, sub_gid, sub_index;
	unsigned i, j;

	if (!pc)
		return NULL;

	query = CALLOC_STRUCT(r600_query_pc);
	if (!query)
		return NULL;

	query->b.b.ops = &batch_query_ops;
	query->b.ops = &batch_query_hw_ops;

	query->num_counters = num_queries;

	/* Collect selectors per group */
	for (i = 0; i < num_queries; ++i) {
		unsigned sub_gid;

		if (query_types[i] < R600_QUERY_FIRST_PERFCOUNTER)
			goto error;

		block = lookup_counter(pc, query_types[i] - R600_QUERY_FIRST_PERFCOUNTER,
				       &base_gid, &sub_index);
		if (!block)
			goto error;

		sub_gid = sub_index / block->num_selectors;
		sub_index = sub_index % block->num_selectors;

		group = get_group_state(screen, query, block, sub_gid);
		if (!group)
			goto error;

		if (group->num_counters >= block->num_counters) {
			fprintf(stderr,
				"perfcounter group %s: too many selected\n",
				block->basename);
			goto error;
		}
		group->selectors[group->num_counters] = sub_index;
		++group->num_counters;
	}

	/* Compute result bases and CS size per group */
	query->b.num_cs_dw_begin = pc->num_start_cs_dwords;
	query->b.num_cs_dw_end = pc->num_stop_cs_dwords;

	query->b.num_cs_dw_begin += pc->num_instance_cs_dwords; /* conservative */
	query->b.num_cs_dw_end += pc->num_instance_cs_dwords;

	i = 0;
	for (group = query->groups; group; group = group->next) {
		struct r600_perfcounter_block *block = group->block;
		unsigned select_dw, read_dw;
		unsigned instances = 1;

		if ((block->flags & R600_PC_BLOCK_SE) && group->se < 0)
			instances = screen->info.max_se;
		if (group->instance < 0)
			instances *= block->num_instances;

		group->result_base = i;
		query->b.result_size += sizeof(uint64_t) * instances * group->num_counters;
		i += instances * group->num_counters;

		pc->get_size(block, group->num_counters, group->selectors,
			     &select_dw, &read_dw);
		query->b.num_cs_dw_begin += select_dw;
		query->b.num_cs_dw_end += instances * read_dw;
		query->b.num_cs_dw_begin += pc->num_instance_cs_dwords; /* conservative */
		query->b.num_cs_dw_end += instances * pc->num_instance_cs_dwords;
	}

	if (query->shaders) {
		if (query->shaders == R600_PC_SHADERS_WINDOWING)
			query->shaders = 0xffffffff;
		query->b.num_cs_dw_begin += pc->num_shaders_cs_dwords;
	}

	/* Map user-supplied query array to result indices */
	query->counters = CALLOC(num_queries, sizeof(*query->counters));
	for (i = 0; i < num_queries; ++i) {
		struct r600_pc_counter *counter = &query->counters[i];
		struct r600_perfcounter_block *block;

		block = lookup_counter(pc, query_types[i] - R600_QUERY_FIRST_PERFCOUNTER,
				       &base_gid, &sub_index);

		sub_gid = sub_index / block->num_selectors;
		sub_index = sub_index % block->num_selectors;

		group = get_group_state(screen, query, block, sub_gid);
		assert(group != NULL);

		for (j = 0; j < group->num_counters; ++j) {
			if (group->selectors[j] == sub_index)
				break;
		}

		counter->base = group->result_base + j;
		counter->stride = group->num_counters;

		counter->qwords = 1;
		if ((block->flags & R600_PC_BLOCK_SE) && group->se < 0)
			counter->qwords = screen->info.max_se;
		if (group->instance < 0)
			counter->qwords *= block->num_instances;
	}

	if (!r600_query_hw_init(screen, &query->b))
		goto error;

	return (struct pipe_query *)query;

error:
	r600_pc_query_destroy(screen, &query->b.b);
	return NULL;
}

static bool r600_init_block_names(struct r600_common_screen *screen,
				  struct r600_perfcounter_block *block)
{
	unsigned i, j, k;
	unsigned groups_shader = 1, groups_se = 1, groups_instance = 1;
	unsigned namelen;
	char *groupname;
	char *p;

	if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS)
		groups_instance = block->num_instances;
	if (block->flags & R600_PC_BLOCK_SE_GROUPS)
		groups_se = screen->info.max_se;
	if (block->flags & R600_PC_BLOCK_SHADER)
		groups_shader = screen->perfcounters->num_shader_types;

	namelen = strlen(block->basename);
	block->group_name_stride = namelen + 1;
	if (block->flags & R600_PC_BLOCK_SHADER)
		block->group_name_stride += 3;
	if (block->flags & R600_PC_BLOCK_SE_GROUPS) {
		assert(groups_se <= 10);
		block->group_name_stride += 1;

		if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS)
			block->group_name_stride += 1;
	}
	if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS) {
		assert(groups_instance <= 100);
		block->group_name_stride += 2;
	}

	block->group_names = MALLOC(block->num_groups * block->group_name_stride);
	if (!block->group_names)
		return false;

	groupname = block->group_names;
	for (i = 0; i < groups_shader; ++i) {
		const char *shader_suffix = screen->perfcounters->shader_type_suffixes[i];
		unsigned shaderlen = strlen(shader_suffix);
		for (j = 0; j < groups_se; ++j) {
			for (k = 0; k < groups_instance; ++k) {
				strcpy(groupname, block->basename);
				p = groupname + namelen;

				if (block->flags & R600_PC_BLOCK_SHADER) {
					strcpy(p, shader_suffix);
					p += shaderlen;
				}

				if (block->flags & R600_PC_BLOCK_SE_GROUPS) {
					p += sprintf(p, "%d", j);
					if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS)
						*p++ = '_';
				}

				if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS)
					p += sprintf(p, "%d", k);

				groupname += block->group_name_stride;
			}
		}
	}

	assert(block->num_selectors <= 1000);
	block->selector_name_stride = block->group_name_stride + 4;
	block->selector_names = MALLOC(block->num_groups * block->num_selectors *
				       block->selector_name_stride);
	if (!block->selector_names)
		return false;

	groupname = block->group_names;
	p = block->selector_names;
	for (i = 0; i < block->num_groups; ++i) {
		for (j = 0; j < block->num_selectors; ++j) {
			sprintf(p, "%s_%03d", groupname, j);
			p += block->selector_name_stride;
		}
		groupname += block->group_name_stride;
	}

	return true;
}

int r600_get_perfcounter_info(struct r600_common_screen *screen,
			      unsigned index,
			      struct pipe_driver_query_info *info)
{
	struct r600_perfcounters *pc = screen->perfcounters;
	struct r600_perfcounter_block *block;
	unsigned base_gid, sub;

	if (!pc)
		return 0;

	if (!info) {
		unsigned bid, num_queries = 0;

		for (bid = 0; bid < pc->num_blocks; ++bid) {
			num_queries += pc->blocks[bid].num_selectors *
				       pc->blocks[bid].num_groups;
		}

		return num_queries;
	}

	block = lookup_counter(pc, index, &base_gid, &sub);
	if (!block)
		return 0;

	if (!block->selector_names) {
		if (!r600_init_block_names(screen, block))
			return 0;
	}
	info->name = block->selector_names + sub * block->selector_name_stride;
	info->query_type = R600_QUERY_FIRST_PERFCOUNTER + index;
	info->max_value.u64 = 0;
	info->type = PIPE_DRIVER_QUERY_TYPE_UINT64;
	info->result_type = PIPE_DRIVER_QUERY_RESULT_TYPE_AVERAGE;
	info->group_id = base_gid + sub / block->num_selectors;
	info->flags = PIPE_DRIVER_QUERY_FLAG_BATCH;
	if (sub > 0 && sub + 1 < block->num_selectors * block->num_groups)
		info->flags |= PIPE_DRIVER_QUERY_FLAG_DONT_LIST;
	return 1;
}

int r600_get_perfcounter_group_info(struct r600_common_screen *screen,
				    unsigned index,
				    struct pipe_driver_query_group_info *info)
{
	struct r600_perfcounters *pc = screen->perfcounters;
	struct r600_perfcounter_block *block;

	if (!pc)
		return 0;

	if (!info)
		return pc->num_groups;

	block = lookup_group(pc, &index);
	if (!block)
		return 0;

	if (!block->group_names) {
		if (!r600_init_block_names(screen, block))
			return 0;
	}
	info->name = block->group_names + index * block->group_name_stride;
	info->num_queries = block->num_selectors;
	info->max_active_queries = block->num_counters;
	return 1;
}

void r600_perfcounters_destroy(struct r600_common_screen *rscreen)
{
	if (rscreen->perfcounters)
		rscreen->perfcounters->cleanup(rscreen);
}

bool r600_perfcounters_init(struct r600_perfcounters *pc,
			    unsigned num_blocks)
{
	pc->blocks = CALLOC(num_blocks, sizeof(struct r600_perfcounter_block));
	if (!pc->blocks)
		return false;

	pc->separate_se = debug_get_bool_option("RADEON_PC_SEPARATE_SE", false);
	pc->separate_instance = debug_get_bool_option("RADEON_PC_SEPARATE_INSTANCE", false);

	return true;
}

void r600_perfcounters_add_block(struct r600_common_screen *rscreen,
				 struct r600_perfcounters *pc,
				 const char *name, unsigned flags,
				 unsigned counters, unsigned selectors,
				 unsigned instances, void *data)
{
	struct r600_perfcounter_block *block = &pc->blocks[pc->num_blocks];

	assert(counters <= R600_QUERY_MAX_COUNTERS);

	block->basename = name;
	block->flags = flags;
	block->num_counters = counters;
	block->num_selectors = selectors;
	block->num_instances = MAX2(instances, 1);
	block->data = data;

	if (pc->separate_se && (block->flags & R600_PC_BLOCK_SE))
		block->flags |= R600_PC_BLOCK_SE_GROUPS;
	if (pc->separate_instance && block->num_instances > 1)
		block->flags |= R600_PC_BLOCK_INSTANCE_GROUPS;

	if (block->flags & R600_PC_BLOCK_INSTANCE_GROUPS) {
		block->num_groups = block->num_instances;
	} else {
		block->num_groups = 1;
	}

	if (block->flags & R600_PC_BLOCK_SE_GROUPS)
		block->num_groups *= rscreen->info.max_se;
	if (block->flags & R600_PC_BLOCK_SHADER)
		block->num_groups *= pc->num_shader_types;

	++pc->num_blocks;
	pc->num_groups += block->num_groups;
}

void r600_perfcounters_do_destroy(struct r600_perfcounters *pc)
{
	unsigned i;

	for (i = 0; i < pc->num_blocks; ++i) {
		FREE(pc->blocks[i].group_names);
		FREE(pc->blocks[i].selector_names);
	}
	FREE(pc->blocks);
	FREE(pc);
}
