/*
 * Copyright 2011 Adam Rak <adam.rak@streamnovation.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Adam Rak <adam.rak@streamnovation.com>
 */

#ifdef HAVE_OPENCL
#include <gelf.h>
#include <libelf.h>
#endif
#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "util/u_blitter.h"
#include "util/list.h"
#include "util/u_transfer.h"
#include "util/u_surface.h"
#include "util/u_pack_color.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_framebuffer.h"
#include "pipebuffer/pb_buffer.h"
#include "evergreend.h"
#include "r600_shader.h"
#include "r600_pipe.h"
#include "r600_formats.h"
#include "evergreen_compute.h"
#include "evergreen_compute_internal.h"
#include "compute_memory_pool.h"
#include <inttypes.h>

/**
RAT0 is for global binding write
VTX1 is for global binding read

for writing images RAT1...
for reading images TEX2...
  TEX2-RAT1 is paired

TEX2... consumes the same fetch resources, that VTX2... would consume

CONST0 and VTX0 is for parameters
  CONST0 is binding smaller input parameter buffer, and for constant indexing,
  also constant cached
  VTX0 is for indirect/non-constant indexing, or if the input is bigger than
  the constant cache can handle

RAT-s are limited to 12, so we can only bind at most 11 texture for writing
because we reserve RAT0 for global bindings. With byteaddressing enabled,
we should reserve another one too.=> 10 image binding for writing max.

from Nvidia OpenCL:
  CL_DEVICE_MAX_READ_IMAGE_ARGS:        128
  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:       8 

so 10 for writing is enough. 176 is the max for reading according to the docs

writable images should be listed first < 10, so their id corresponds to RAT(id+1)
writable images will consume TEX slots, VTX slots too because of linear indexing

*/

#ifdef HAVE_OPENCL
static void radeon_shader_binary_init(struct r600_shader_binary *b)
{
	memset(b, 0, sizeof(*b));
}

static void radeon_shader_binary_clean(struct r600_shader_binary *b)
{
	if (!b)
		return;
	FREE(b->code);
	FREE(b->config);
	FREE(b->rodata);
	FREE(b->global_symbol_offsets);
	FREE(b->relocs);
	FREE(b->disasm_string);
}
#endif

struct r600_resource *r600_compute_buffer_alloc_vram(struct r600_screen *screen,
						     unsigned size)
{
	struct pipe_resource *buffer = NULL;
	assert(size);

	buffer = pipe_buffer_create((struct pipe_screen*) screen,
				    0, PIPE_USAGE_IMMUTABLE, size);

	return (struct r600_resource *)buffer;
}


static void evergreen_set_rat(struct r600_pipe_compute *pipe,
			      unsigned id,
			      struct r600_resource *bo,
			      int start,
			      int size)
{
	struct pipe_surface rat_templ;
	struct r600_surface *surf = NULL;
	struct r600_context *rctx = NULL;

	assert(id < 12);
	assert((size & 3) == 0);
	assert((start & 0xFF) == 0);

	rctx = pipe->ctx;

	COMPUTE_DBG(rctx->screen, "bind rat: %i \n", id);

	/* Create the RAT surface */
	memset(&rat_templ, 0, sizeof(rat_templ));
	rat_templ.format = PIPE_FORMAT_R32_UINT;
	rat_templ.u.tex.level = 0;
	rat_templ.u.tex.first_layer = 0;
	rat_templ.u.tex.last_layer = 0;

	/* Add the RAT the list of color buffers. Drop the old buffer first. */
	pipe_surface_reference(&pipe->ctx->framebuffer.state.cbufs[id], NULL);
	pipe->ctx->framebuffer.state.cbufs[id] = pipe->ctx->b.b.create_surface(
		(struct pipe_context *)pipe->ctx,
		(struct pipe_resource *)bo, &rat_templ);

	/* Update the number of color buffers */
	pipe->ctx->framebuffer.state.nr_cbufs =
		MAX2(id + 1, pipe->ctx->framebuffer.state.nr_cbufs);

	/* Update the cb_target_mask
	 * XXX: I think this is a potential spot for bugs once we start doing
	 * GL interop.  cb_target_mask may be modified in the 3D sections
	 * of this driver. */
	pipe->ctx->compute_cb_target_mask |= (0xf << (id * 4));

	surf = (struct r600_surface*)pipe->ctx->framebuffer.state.cbufs[id];
	evergreen_init_color_surface_rat(rctx, surf);
}

static void evergreen_cs_set_vertex_buffer(struct r600_context *rctx,
					   unsigned vb_index,
					   unsigned offset,
					   struct pipe_resource *buffer)
{
	struct r600_vertexbuf_state *state = &rctx->cs_vertex_buffer_state;
	struct pipe_vertex_buffer *vb = &state->vb[vb_index];
	vb->buffer_offset = offset;
	vb->buffer.resource = buffer;
	vb->is_user_buffer = false;

	/* The vertex instructions in the compute shaders use the texture cache,
	 * so we need to invalidate it. */
	rctx->b.flags |= R600_CONTEXT_INV_VERTEX_CACHE;
	state->enabled_mask |= 1 << vb_index;
	state->dirty_mask |= 1 << vb_index;
	r600_mark_atom_dirty(rctx, &state->atom);
}

static void evergreen_cs_set_constant_buffer(struct r600_context *rctx,
					     unsigned cb_index,
					     unsigned offset,
					     unsigned size,
					     struct pipe_resource *buffer)
{
	struct pipe_constant_buffer cb;
	cb.buffer_size = size;
	cb.buffer_offset = offset;
	cb.buffer = buffer;
	cb.user_buffer = NULL;

	rctx->b.b.set_constant_buffer(&rctx->b.b, PIPE_SHADER_COMPUTE, cb_index, false, &cb);
}

/* We need to define these R600 registers here, because we can't include
 * evergreend.h and r600d.h.
 */
#define R_028868_SQ_PGM_RESOURCES_VS                 0x028868
#define R_028850_SQ_PGM_RESOURCES_PS                 0x028850

#ifdef HAVE_OPENCL
static void parse_symbol_table(Elf_Data *symbol_table_data,
				const GElf_Shdr *symbol_table_header,
				struct r600_shader_binary *binary)
{
	GElf_Sym symbol;
	unsigned i = 0;
	unsigned symbol_count =
		symbol_table_header->sh_size / symbol_table_header->sh_entsize;

	/* We are over allocating this list, because symbol_count gives the
	 * total number of symbols, and we will only be filling the list
	 * with offsets of global symbols.  The memory savings from
	 * allocating the correct size of this list will be small, and
	 * I don't think it is worth the cost of pre-computing the number
	 * of global symbols.
	 */
	binary->global_symbol_offsets = CALLOC(symbol_count, sizeof(uint64_t));

	while (gelf_getsym(symbol_table_data, i++, &symbol)) {
		unsigned i;
		if (GELF_ST_BIND(symbol.st_info) != STB_GLOBAL ||
		    symbol.st_shndx == 0 /* Undefined symbol */) {
			continue;
		}

		binary->global_symbol_offsets[binary->global_symbol_count] =
					symbol.st_value;

		/* Sort the list using bubble sort.  This list will usually
		 * be small. */
		for (i = binary->global_symbol_count; i > 0; --i) {
			uint64_t lhs = binary->global_symbol_offsets[i - 1];
			uint64_t rhs = binary->global_symbol_offsets[i];
			if (lhs < rhs) {
				break;
			}
			binary->global_symbol_offsets[i] = lhs;
			binary->global_symbol_offsets[i - 1] = rhs;
		}
		++binary->global_symbol_count;
	}
}


static void parse_relocs(Elf *elf, Elf_Data *relocs, Elf_Data *symbols,
			unsigned symbol_sh_link,
			struct r600_shader_binary *binary)
{
	unsigned i;

	if (!relocs || !symbols || !binary->reloc_count) {
		return;
	}
	binary->relocs = CALLOC(binary->reloc_count,
			sizeof(struct r600_shader_reloc));
	for (i = 0; i < binary->reloc_count; i++) {
		GElf_Sym symbol;
		GElf_Rel rel;
		char *symbol_name;
		struct r600_shader_reloc *reloc = &binary->relocs[i];

		gelf_getrel(relocs, i, &rel);
		gelf_getsym(symbols, GELF_R_SYM(rel.r_info), &symbol);
		symbol_name = elf_strptr(elf, symbol_sh_link, symbol.st_name);

		reloc->offset = rel.r_offset;
		strncpy(reloc->name, symbol_name, sizeof(reloc->name)-1);
		reloc->name[sizeof(reloc->name)-1] = 0;
	}
}

static void r600_elf_read(const char *elf_data, unsigned elf_size,
		 struct r600_shader_binary *binary)
{
	char *elf_buffer;
	Elf *elf;
	Elf_Scn *section = NULL;
	Elf_Data *symbols = NULL, *relocs = NULL;
	size_t section_str_index;
	unsigned symbol_sh_link = 0;

	/* One of the libelf implementations
	 * (http://www.mr511.de/software/english.htm) requires calling
	 * elf_version() before elf_memory().
	 */
	elf_version(EV_CURRENT);
	elf_buffer = MALLOC(elf_size);
	memcpy(elf_buffer, elf_data, elf_size);

	elf = elf_memory(elf_buffer, elf_size);

	elf_getshdrstrndx(elf, &section_str_index);

	while ((section = elf_nextscn(elf, section))) {
		const char *name;
		Elf_Data *section_data = NULL;
		GElf_Shdr section_header;
		if (gelf_getshdr(section, &section_header) != &section_header) {
			fprintf(stderr, "Failed to read ELF section header\n");
			return;
		}
		name = elf_strptr(elf, section_str_index, section_header.sh_name);
		if (!strcmp(name, ".text")) {
			section_data = elf_getdata(section, section_data);
			binary->code_size = section_data->d_size;
			binary->code = MALLOC(binary->code_size * sizeof(unsigned char));
			memcpy(binary->code, section_data->d_buf, binary->code_size);
		} else if (!strcmp(name, ".AMDGPU.config")) {
			section_data = elf_getdata(section, section_data);
			binary->config_size = section_data->d_size;
			binary->config = MALLOC(binary->config_size * sizeof(unsigned char));
			memcpy(binary->config, section_data->d_buf, binary->config_size);
		} else if (!strcmp(name, ".AMDGPU.disasm")) {
			/* Always read disassembly if it's available. */
			section_data = elf_getdata(section, section_data);
			binary->disasm_string = strndup(section_data->d_buf,
							section_data->d_size);
		} else if (!strncmp(name, ".rodata", 7)) {
			section_data = elf_getdata(section, section_data);
			binary->rodata_size = section_data->d_size;
			binary->rodata = MALLOC(binary->rodata_size * sizeof(unsigned char));
			memcpy(binary->rodata, section_data->d_buf, binary->rodata_size);
		} else if (!strncmp(name, ".symtab", 7)) {
			symbols = elf_getdata(section, section_data);
			symbol_sh_link = section_header.sh_link;
			parse_symbol_table(symbols, &section_header, binary);
		} else if (!strcmp(name, ".rel.text")) {
			relocs = elf_getdata(section, section_data);
			binary->reloc_count = section_header.sh_size /
					section_header.sh_entsize;
		}
	}

	parse_relocs(elf, relocs, symbols, symbol_sh_link, binary);

	if (elf){
		elf_end(elf);
	}
	FREE(elf_buffer);

	/* Cache the config size per symbol */
	if (binary->global_symbol_count) {
		binary->config_size_per_symbol =
			binary->config_size / binary->global_symbol_count;
	} else {
		binary->global_symbol_count = 1;
		binary->config_size_per_symbol = binary->config_size;
	}
}

static const unsigned char *r600_shader_binary_config_start(
	const struct r600_shader_binary *binary,
	uint64_t symbol_offset)
{
	unsigned i;
	for (i = 0; i < binary->global_symbol_count; ++i) {
		if (binary->global_symbol_offsets[i] == symbol_offset) {
			unsigned offset = i * binary->config_size_per_symbol;
			return binary->config + offset;
		}
	}
	return binary->config;
}

static void r600_shader_binary_read_config(const struct r600_shader_binary *binary,
					   struct r600_bytecode *bc,
					   uint64_t symbol_offset,
					   bool *use_kill)
{
       unsigned i;
       const unsigned char *config =
               r600_shader_binary_config_start(binary, symbol_offset);

       for (i = 0; i < binary->config_size_per_symbol; i+= 8) {
               unsigned reg =
                       util_le32_to_cpu(*(uint32_t*)(config + i));
               unsigned value =
                       util_le32_to_cpu(*(uint32_t*)(config + i + 4));
               switch (reg) {
               /* R600 / R700 */
               case R_028850_SQ_PGM_RESOURCES_PS:
               case R_028868_SQ_PGM_RESOURCES_VS:
               /* Evergreen / Northern Islands */
               case R_028844_SQ_PGM_RESOURCES_PS:
               case R_028860_SQ_PGM_RESOURCES_VS:
               case R_0288D4_SQ_PGM_RESOURCES_LS:
                       bc->ngpr = MAX2(bc->ngpr, G_028844_NUM_GPRS(value));
                       bc->nstack = MAX2(bc->nstack, G_028844_STACK_SIZE(value));
                       break;
               case R_02880C_DB_SHADER_CONTROL:
                       *use_kill = G_02880C_KILL_ENABLE(value);
                       break;
               case R_0288E8_SQ_LDS_ALLOC:
                       bc->nlds_dw = value;
                       break;
               }
       }
}

static unsigned r600_create_shader(struct r600_bytecode *bc,
				   const struct r600_shader_binary *binary,
				   bool *use_kill)

{
	assert(binary->code_size % 4 == 0);
	bc->bytecode = CALLOC(1, binary->code_size);
	memcpy(bc->bytecode, binary->code, binary->code_size);
	bc->ndw = binary->code_size / 4;

	r600_shader_binary_read_config(binary, bc, 0, use_kill);
	return 0;
}

#endif

static void r600_destroy_shader(struct r600_bytecode *bc)
{
	FREE(bc->bytecode);
}

static void *evergreen_create_compute_state(struct pipe_context *ctx,
					    const struct pipe_compute_state *cso)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_pipe_compute *shader = CALLOC_STRUCT(r600_pipe_compute);
#ifdef HAVE_OPENCL
	const struct pipe_binary_program_header *header;
	void *p;
	bool use_kill;
#endif

	shader->ctx = rctx;
	shader->local_size = cso->static_shared_mem;
	shader->input_size = cso->req_input_mem;

	shader->ir_type = cso->ir_type;

	if (shader->ir_type == PIPE_SHADER_IR_TGSI ||
	    shader->ir_type == PIPE_SHADER_IR_NIR) {
		shader->sel = r600_create_shader_state_tokens(ctx, cso->prog, cso->ir_type, PIPE_SHADER_COMPUTE);

		/* Precompile the shader with the expected shader key, to reduce jank at
		 * draw time. Also produces output for shader-db.
		 */
		bool dirty;
		r600_shader_select(ctx, shader->sel, &dirty, true);

		return shader;
	}
#ifdef HAVE_OPENCL
	COMPUTE_DBG(rctx->screen, "*** evergreen_create_compute_state\n");
	header = cso->prog;
	radeon_shader_binary_init(&shader->binary);
	r600_elf_read(header->blob, header->num_bytes, &shader->binary);
	r600_create_shader(&shader->bc, &shader->binary, &use_kill);

	/* Upload code + ROdata */
	shader->code_bo = r600_compute_buffer_alloc_vram(rctx->screen,
							shader->bc.ndw * 4);
	p = r600_buffer_map_sync_with_rings(
		&rctx->b, shader->code_bo,
		PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
	//TODO: use util_memcpy_cpu_to_le32 ?
	memcpy(p, shader->bc.bytecode, shader->bc.ndw * 4);
	rctx->b.ws->buffer_unmap(rctx->b.ws, shader->code_bo->buf);
#endif

	return shader;
}

static void evergreen_delete_compute_state(struct pipe_context *ctx, void *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_pipe_compute *shader = state;

	COMPUTE_DBG(rctx->screen, "*** evergreen_delete_compute_state\n");

	if (!shader)
		return;

	if (shader->ir_type == PIPE_SHADER_IR_TGSI ||
	    shader->ir_type == PIPE_SHADER_IR_NIR) {
		r600_delete_shader_selector(ctx, shader->sel);
	} else {
#ifdef HAVE_OPENCL
		radeon_shader_binary_clean(&shader->binary);
		pipe_resource_reference((struct pipe_resource**)&shader->code_bo, NULL);
		pipe_resource_reference((struct pipe_resource**)&shader->kernel_param, NULL);
#endif
		r600_destroy_shader(&shader->bc);
	}
	FREE(shader);
}

static void evergreen_bind_compute_state(struct pipe_context *ctx, void *state)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_pipe_compute *cstate = (struct r600_pipe_compute *)state;
	COMPUTE_DBG(rctx->screen, "*** evergreen_bind_compute_state\n");

	if (!state) {
		rctx->cs_shader_state.shader = (struct r600_pipe_compute *)state;
		return;
	}

	if (cstate->ir_type == PIPE_SHADER_IR_TGSI ||
	    cstate->ir_type == PIPE_SHADER_IR_NIR) {
		bool compute_dirty;
		if (r600_shader_select(ctx, cstate->sel, &compute_dirty, false))
			R600_ERR("Failed to select compute shader\n");
	}
	
	rctx->cs_shader_state.shader = (struct r600_pipe_compute *)state;
}

/* The kernel parameters are stored a vtx buffer (ID=0), besides the explicit
 * kernel parameters there are implicit parameters that need to be stored
 * in the vertex buffer as well.  Here is how these parameters are organized in
 * the buffer:
 *
 * DWORDS 0-2: Number of work groups in each dimension (x,y,z)
 * DWORDS 3-5: Number of global work items in each dimension (x,y,z)
 * DWORDS 6-8: Number of work items within each work group in each dimension
 *             (x,y,z)
 * DWORDS 9+ : Kernel parameters
 */
static void evergreen_compute_upload_input(struct pipe_context *ctx,
					   const struct pipe_grid_info *info)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_pipe_compute *shader = rctx->cs_shader_state.shader;
	unsigned i;
	/* We need to reserve 9 dwords (36 bytes) for implicit kernel
	 * parameters.
	 */
	unsigned input_size;
	uint32_t *num_work_groups_start;
	uint32_t *global_size_start;
	uint32_t *local_size_start;
	uint32_t *kernel_parameters_start;
	struct pipe_box box;
	struct pipe_transfer *transfer = NULL;

	if (!shader)
		return;
	if (shader->input_size == 0) {
		return;
	}
	input_size = shader->input_size + 36;
	if (!shader->kernel_param) {
		/* Add space for the grid dimensions */
		shader->kernel_param = (struct r600_resource *)
			pipe_buffer_create(ctx->screen, 0,
					PIPE_USAGE_IMMUTABLE, input_size);
	}

	u_box_1d(0, input_size, &box);
	num_work_groups_start = ctx->buffer_map(ctx,
			(struct pipe_resource*)shader->kernel_param,
			0, PIPE_MAP_WRITE | PIPE_MAP_DISCARD_RANGE,
			&box, &transfer);
	global_size_start = num_work_groups_start + (3 * (sizeof(uint) /4));
	local_size_start = global_size_start + (3 * (sizeof(uint)) / 4);
	kernel_parameters_start = local_size_start + (3 * (sizeof(uint)) / 4);

	/* Copy the work group size */
	memcpy(num_work_groups_start, info->grid, 3 * sizeof(uint));

	/* Copy the global size */
	for (i = 0; i < 3; i++) {
		global_size_start[i] = info->grid[i] * info->block[i];
	}

	/* Copy the local dimensions */
	memcpy(local_size_start, info->block, 3 * sizeof(uint));

	/* Copy the kernel inputs */
	memcpy(kernel_parameters_start, info->input, shader->input_size);

	for (i = 0; i < (input_size / 4); i++) {
		COMPUTE_DBG(rctx->screen, "input %i : %u\n", i,
			((unsigned*)num_work_groups_start)[i]);
	}

	ctx->buffer_unmap(ctx, transfer);

	/* ID=0 and ID=3 are reserved for the parameters.
	 * LLVM will preferably use ID=0, but it does not work for dynamic
	 * indices. */
	evergreen_cs_set_vertex_buffer(rctx, 3, 0,
			(struct pipe_resource*)shader->kernel_param);
	evergreen_cs_set_constant_buffer(rctx, 0, 0, input_size,
			(struct pipe_resource*)shader->kernel_param);
}

static void evergreen_emit_dispatch(struct r600_context *rctx,
				    const struct pipe_grid_info *info,
				    uint32_t indirect_grid[3])
{
	int i;
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	struct r600_pipe_compute *shader = rctx->cs_shader_state.shader;
	bool render_cond_bit = rctx->b.render_cond && !rctx->b.render_cond_force_off;
	unsigned num_waves;
	unsigned num_pipes = rctx->screen->b.info.r600_max_quad_pipes;
	unsigned wave_divisor = (16 * num_pipes);
	int group_size = 1;
	unsigned lds_size = (shader->local_size + info->variable_shared_mem) / 4;

	if (shader->ir_type != PIPE_SHADER_IR_TGSI &&
	    shader->ir_type != PIPE_SHADER_IR_NIR)
		lds_size += shader->bc.nlds_dw;
	
	/* Calculate group_size */
	for (i = 0; i < 3; i++) {
		group_size *= info->block[i];
	}

	/* num_waves = ceil((tg_size.x * tg_size.y, tg_size.z) / (16 * num_pipes)) */
	num_waves = (info->block[0] * info->block[1] * info->block[2] +
			wave_divisor - 1) / wave_divisor;

	COMPUTE_DBG(rctx->screen, "Using %u pipes, "
				"%u wavefronts per thread block, "
				"allocating %u dwords lds.\n",
				num_pipes, num_waves, lds_size);

	radeon_set_config_reg(cs, R_008970_VGT_NUM_INDICES, group_size);

	radeon_set_config_reg_seq(cs, R_00899C_VGT_COMPUTE_START_X, 3);
	radeon_emit(cs, 0); /* R_00899C_VGT_COMPUTE_START_X */
	radeon_emit(cs, 0); /* R_0089A0_VGT_COMPUTE_START_Y */
	radeon_emit(cs, 0); /* R_0089A4_VGT_COMPUTE_START_Z */

	radeon_set_config_reg(cs, R_0089AC_VGT_COMPUTE_THREAD_GROUP_SIZE,
								group_size);

	radeon_compute_set_context_reg_seq(cs, R_0286EC_SPI_COMPUTE_NUM_THREAD_X, 3);
	radeon_emit(cs, info->block[0]); /* R_0286EC_SPI_COMPUTE_NUM_THREAD_X */
	radeon_emit(cs, info->block[1]); /* R_0286F0_SPI_COMPUTE_NUM_THREAD_Y */
	radeon_emit(cs, info->block[2]); /* R_0286F4_SPI_COMPUTE_NUM_THREAD_Z */

	if (rctx->b.gfx_level < CAYMAN) {
		assert(lds_size <= 8192);
	} else {
		/* Cayman appears to have a slightly smaller limit, see the
		 * value of CM_R_0286FC_SPI_LDS_MGMT.NUM_LS_LDS */
		assert(lds_size <= 8160);
	}

	radeon_compute_set_context_reg(cs, R_0288E8_SQ_LDS_ALLOC,
					lds_size | (num_waves << 14));

	if (info->indirect) {
		radeon_emit(cs, PKT3C(PKT3_DISPATCH_DIRECT, 3, render_cond_bit));
		radeon_emit(cs, indirect_grid[0]);
		radeon_emit(cs, indirect_grid[1]);
		radeon_emit(cs, indirect_grid[2]);
		radeon_emit(cs, 1);
	} else {
		/* Dispatch packet */
		radeon_emit(cs, PKT3C(PKT3_DISPATCH_DIRECT, 3, render_cond_bit));
		radeon_emit(cs, info->grid[0]);
		radeon_emit(cs, info->grid[1]);
		radeon_emit(cs, info->grid[2]);
		/* VGT_DISPATCH_INITIATOR = COMPUTE_SHADER_EN */
		radeon_emit(cs, 1);
	}

	if (rctx->is_debug)
		eg_trace_emit(rctx);
}

static void compute_setup_cbs(struct r600_context *rctx)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned i;

	/* Emit colorbuffers. */
	/* XXX support more than 8 colorbuffers (the offsets are not a multiple of 0x3C for CB8-11) */
	for (i = 0; i < 8 && i < rctx->framebuffer.state.nr_cbufs; i++) {
		struct r600_surface *cb = (struct r600_surface*)rctx->framebuffer.state.cbufs[i];
		unsigned reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
						       (struct r600_resource*)cb->base.texture,
						       RADEON_USAGE_READWRITE |
						       RADEON_PRIO_SHADER_RW_BUFFER);

		radeon_compute_set_context_reg_seq(cs, R_028C60_CB_COLOR0_BASE + i * 0x3C, 7);
		radeon_emit(cs, cb->cb_color_base);	/* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, cb->cb_color_pitch);	/* R_028C64_CB_COLOR0_PITCH */
		radeon_emit(cs, cb->cb_color_slice);	/* R_028C68_CB_COLOR0_SLICE */
		radeon_emit(cs, cb->cb_color_view);	/* R_028C6C_CB_COLOR0_VIEW */
		radeon_emit(cs, cb->cb_color_info);	/* R_028C70_CB_COLOR0_INFO */
		radeon_emit(cs, cb->cb_color_attrib);	/* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, cb->cb_color_dim);		/* R_028C78_CB_COLOR0_DIM */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C60_CB_COLOR0_BASE */
		radeon_emit(cs, reloc);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0)); /* R_028C74_CB_COLOR0_ATTRIB */
		radeon_emit(cs, reloc);
	}
	for (; i < 8 ; i++)
		radeon_compute_set_context_reg(cs, R_028C70_CB_COLOR0_INFO + i * 0x3C,
					       S_028C70_FORMAT(V_028C70_COLOR_INVALID));
	for (; i < 12; i++)
		radeon_compute_set_context_reg(cs, R_028E50_CB_COLOR8_INFO + (i - 8) * 0x1C,
					       S_028C70_FORMAT(V_028C70_COLOR_INVALID));

	/* Set CB_TARGET_MASK  XXX: Use cb_misc_state */
	radeon_compute_set_context_reg(cs, R_028238_CB_TARGET_MASK,
				       rctx->compute_cb_target_mask);
}

static void compute_emit_cs(struct r600_context *rctx,
			    const struct pipe_grid_info *info)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	bool compute_dirty = false;
	struct r600_pipe_shader *current;
	struct r600_shader_atomic combined_atomics[8];
	uint8_t atomic_used_mask;
	uint32_t indirect_grid[3] = { 0, 0, 0 };

	/* make sure that the gfx ring is only one active */
	if (radeon_emitted(&rctx->b.dma.cs, 0)) {
		rctx->b.dma.flush(rctx, PIPE_FLUSH_ASYNC, NULL);
	}

	r600_update_compressed_resource_state(rctx, true);

	if (!rctx->cmd_buf_is_compute) {
		rctx->b.gfx.flush(rctx, PIPE_FLUSH_ASYNC, NULL);
		rctx->cmd_buf_is_compute = true;
	}

	if (rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_TGSI||
	    rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_NIR) {
		if (r600_shader_select(&rctx->b.b, rctx->cs_shader_state.shader->sel, &compute_dirty, false)) {
			R600_ERR("Failed to select compute shader\n");
			return;
		}
		
		current = rctx->cs_shader_state.shader->sel->current;
		if (compute_dirty) {
			rctx->cs_shader_state.atom.num_dw = current->command_buffer.num_dw;
			r600_context_add_resource_size(&rctx->b.b, (struct pipe_resource *)current->bo);
			r600_set_atom_dirty(rctx, &rctx->cs_shader_state.atom, true);
		}

		bool need_buf_const = current->shader.uses_tex_buffers ||
			current->shader.has_txq_cube_array_z_comp;

		if (info->indirect) {
			struct r600_resource *indirect_resource = (struct r600_resource *)info->indirect;
			unsigned *data = r600_buffer_map_sync_with_rings(&rctx->b, indirect_resource, PIPE_MAP_READ);
			unsigned offset = info->indirect_offset / 4;
			indirect_grid[0] = data[offset];
			indirect_grid[1] = data[offset + 1];
			indirect_grid[2] = data[offset + 2];
		}
		for (int i = 0; i < 3; i++) {
			rctx->cs_block_grid_sizes[i] = info->block[i];
			rctx->cs_block_grid_sizes[i + 4] = info->indirect ? indirect_grid[i] : info->grid[i];
		}
		rctx->cs_block_grid_sizes[3] = rctx->cs_block_grid_sizes[7] = 0;
		rctx->driver_consts[PIPE_SHADER_COMPUTE].cs_block_grid_size_dirty = true;

		evergreen_emit_atomic_buffer_setup_count(rctx, current, combined_atomics, &atomic_used_mask);
		r600_need_cs_space(rctx, 0, true, util_bitcount(atomic_used_mask));

		if (need_buf_const) {
			eg_setup_buffer_constants(rctx, PIPE_SHADER_COMPUTE);
		}
		r600_update_driver_const_buffers(rctx, true);

		evergreen_emit_atomic_buffer_setup(rctx, true, combined_atomics, atomic_used_mask);
		if (atomic_used_mask) {
			radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
			radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));
		}
	} else
		r600_need_cs_space(rctx, 0, true, 0);

	/* Initialize all the compute-related registers.
	 *
	 * See evergreen_init_atom_start_compute_cs() in this file for the list
	 * of registers initialized by the start_compute_cs_cmd atom.
	 */
	r600_emit_command_buffer(cs, &rctx->start_compute_cs_cmd);

	/* emit config state */
	if (rctx->b.gfx_level == EVERGREEN) {
		if (rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_TGSI||
		    rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_NIR) {
			radeon_set_config_reg_seq(cs, R_008C04_SQ_GPR_RESOURCE_MGMT_1, 3);
			radeon_emit(cs, S_008C04_NUM_CLAUSE_TEMP_GPRS(rctx->r6xx_num_clause_temp_gprs));
			radeon_emit(cs, 0);
			radeon_emit(cs, 0);
			radeon_set_config_reg(cs, R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, (1 << 8));
		} else
			r600_emit_atom(rctx, &rctx->config_state.atom);
	}

	rctx->b.flags |= R600_CONTEXT_WAIT_3D_IDLE | R600_CONTEXT_FLUSH_AND_INV;
	r600_flush_emit(rctx);

	if (rctx->cs_shader_state.shader->ir_type != PIPE_SHADER_IR_TGSI &&
	    rctx->cs_shader_state.shader->ir_type != PIPE_SHADER_IR_NIR) {

		compute_setup_cbs(rctx);

		/* Emit vertex buffer state */
		rctx->cs_vertex_buffer_state.atom.num_dw = 12 * util_bitcount(rctx->cs_vertex_buffer_state.dirty_mask);
		r600_emit_atom(rctx, &rctx->cs_vertex_buffer_state.atom);
	} else {
		uint32_t rat_mask;

		rat_mask = evergreen_construct_rat_mask(rctx, &rctx->cb_misc_state, 0);
		radeon_compute_set_context_reg(cs, R_028238_CB_TARGET_MASK,
					       rat_mask);
	}

	r600_emit_atom(rctx, &rctx->b.render_cond_atom);

	/* Emit constant buffer state */
	r600_emit_atom(rctx, &rctx->constbuf_state[PIPE_SHADER_COMPUTE].atom);

	/* Emit sampler state */
	r600_emit_atom(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE].states.atom);

	/* Emit sampler view (texture resource) state */
	r600_emit_atom(rctx, &rctx->samplers[PIPE_SHADER_COMPUTE].views.atom);

	/* Emit images state */
	r600_emit_atom(rctx, &rctx->compute_images.atom);

	/* Emit buffers state */
	r600_emit_atom(rctx, &rctx->compute_buffers.atom);

	/* Emit shader state */
	r600_emit_atom(rctx, &rctx->cs_shader_state.atom);

	/* Emit dispatch state and dispatch packet */
	evergreen_emit_dispatch(rctx, info, indirect_grid);

	/* XXX evergreen_flush_emit() hardcodes the CP_COHER_SIZE to 0xffffffff
	 */
	rctx->b.flags |= R600_CONTEXT_INV_CONST_CACHE |
		      R600_CONTEXT_INV_VERTEX_CACHE |
	              R600_CONTEXT_INV_TEX_CACHE;
	r600_flush_emit(rctx);
	rctx->b.flags = 0;

	if (rctx->b.gfx_level >= CAYMAN) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));
		/* DEALLOC_STATE prevents the GPU from hanging when a
		 * SURFACE_SYNC packet is emitted some time after a DISPATCH_DIRECT
		 * with any of the CB*_DEST_BASE_ENA or DB_DEST_BASE_ENA bits set.
		 */
		radeon_emit(cs, PKT3C(PKT3_DEALLOC_STATE, 0, 0));
		radeon_emit(cs, 0);
	}
	if (rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_TGSI ||
	    rctx->cs_shader_state.shader->ir_type == PIPE_SHADER_IR_NIR)
		evergreen_emit_atomic_buffer_save(rctx, true, combined_atomics, &atomic_used_mask);

#if 0
	COMPUTE_DBG(rctx->screen, "cdw: %i\n", cs->cdw);
	for (i = 0; i < cs->cdw; i++) {
		COMPUTE_DBG(rctx->screen, "%4i : 0x%08X\n", i, cs->buf[i]);
	}
#endif

}


/**
 * Emit function for r600_cs_shader_state atom
 */
void evergreen_emit_cs_shader(struct r600_context *rctx,
			      struct r600_atom *atom)
{
	struct r600_cs_shader_state *state =
					(struct r600_cs_shader_state*)atom;
	struct r600_pipe_compute *shader = state->shader;
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	uint64_t va;
	struct r600_resource *code_bo;
	unsigned ngpr, nstack;

	if (shader->ir_type == PIPE_SHADER_IR_TGSI ||
	    shader->ir_type == PIPE_SHADER_IR_NIR) {
		code_bo = shader->sel->current->bo;
		va = shader->sel->current->bo->gpu_address;
		ngpr = shader->sel->current->shader.bc.ngpr;
		nstack = shader->sel->current->shader.bc.nstack;
	} else {
		code_bo = shader->code_bo;
		va = shader->code_bo->gpu_address + state->pc;
		ngpr = shader->bc.ngpr;
		nstack = shader->bc.nstack;
	}

	radeon_compute_set_context_reg_seq(cs, R_0288D0_SQ_PGM_START_LS, 3);
	radeon_emit(cs, va >> 8); /* R_0288D0_SQ_PGM_START_LS */
	radeon_emit(cs,           /* R_0288D4_SQ_PGM_RESOURCES_LS */
			S_0288D4_NUM_GPRS(ngpr) |
			S_0288D4_DX10_CLAMP(1) |
			S_0288D4_STACK_SIZE(nstack));
	radeon_emit(cs, 0);	/* R_0288D8_SQ_PGM_RESOURCES_LS_2 */

	radeon_emit(cs, PKT3C(PKT3_NOP, 0, 0));
	radeon_emit(cs, radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
					      code_bo, RADEON_USAGE_READ |
					      RADEON_PRIO_SHADER_BINARY));
}

static void evergreen_launch_grid(struct pipe_context *ctx,
				  const struct pipe_grid_info *info)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
#ifdef HAVE_OPENCL
	struct r600_pipe_compute *shader = rctx->cs_shader_state.shader;
	bool use_kill;

	if (shader->ir_type != PIPE_SHADER_IR_TGSI &&
	    shader->ir_type != PIPE_SHADER_IR_NIR) {
		rctx->cs_shader_state.pc = info->pc;
		/* Get the config information for this kernel. */
		r600_shader_binary_read_config(&shader->binary, &shader->bc,
					       info->pc, &use_kill);
	} else {
		use_kill = false;
		rctx->cs_shader_state.pc = 0;
	}
#endif

	COMPUTE_DBG(rctx->screen, "*** evergreen_launch_grid: pc = %u\n", info->pc);


	evergreen_compute_upload_input(ctx, info);
	compute_emit_cs(rctx, info);
}

static void evergreen_set_compute_resources(struct pipe_context *ctx,
					    unsigned start, unsigned count,
					    struct pipe_surface **surfaces)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_surface **resources = (struct r600_surface **)surfaces;

	COMPUTE_DBG(rctx->screen, "*** evergreen_set_compute_resources: start = %u count = %u\n",
			start, count);

	for (unsigned i = 0; i < count; i++) {
		/* The First four vertex buffers are reserved for parameters and
		 * global buffers. */
		unsigned vtx_id = 4 + i;
		if (resources[i]) {
			struct r600_resource_global *buffer =
				(struct r600_resource_global*)
				resources[i]->base.texture;
			if (resources[i]->base.writable) {
				assert(i+1 < 12);

				evergreen_set_rat(rctx->cs_shader_state.shader, i+1,
				(struct r600_resource *)resources[i]->base.texture,
				buffer->chunk->start_in_dw*4,
				resources[i]->base.texture->width0);
			}

			evergreen_cs_set_vertex_buffer(rctx, vtx_id,
					buffer->chunk->start_in_dw * 4,
					resources[i]->base.texture);
		}
	}
}

static void evergreen_set_global_binding(struct pipe_context *ctx,
					 unsigned first, unsigned n,
					 struct pipe_resource **resources,
					 uint32_t **handles)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct compute_memory_pool *pool = rctx->screen->global_pool;
	struct r600_resource_global **buffers =
		(struct r600_resource_global **)resources;
	unsigned i;

	COMPUTE_DBG(rctx->screen, "*** evergreen_set_global_binding first = %u n = %u\n",
			first, n);

	if (!resources) {
		/* XXX: Unset */
		return;
	}

	/* We mark these items for promotion to the pool if they
	 * aren't already there */
	for (i = first; i < first + n; i++) {
		struct compute_memory_item *item = buffers[i]->chunk;

		if (!is_item_in_pool(item))
			buffers[i]->chunk->status |= ITEM_FOR_PROMOTING;
	}

	if (compute_memory_finalize_pending(pool, ctx) == -1) {
		/* XXX: Unset */
		return;
	}

	for (i = first; i < first + n; i++)
	{
		uint32_t buffer_offset;
		uint32_t handle;
		assert(resources[i]->target == PIPE_BUFFER);
		assert(resources[i]->bind & PIPE_BIND_GLOBAL);

		buffer_offset = util_le32_to_cpu(*(handles[i]));
		handle = buffer_offset + buffers[i]->chunk->start_in_dw * 4;

		*(handles[i]) = util_cpu_to_le32(handle);
	}

	/* globals for writing */
	evergreen_set_rat(rctx->cs_shader_state.shader, 0, pool->bo, 0, pool->size_in_dw * 4);
	/* globals for reading */
	evergreen_cs_set_vertex_buffer(rctx, 1, 0,
				(struct pipe_resource*)pool->bo);

	/* constants for reading, LLVM puts them in text segment */
	evergreen_cs_set_vertex_buffer(rctx, 2, 0,
				(struct pipe_resource*)rctx->cs_shader_state.shader->code_bo);
}

/**
 * This function initializes all the compute specific registers that need to
 * be initialized for each compute command stream.  Registers that are common
 * to both compute and 3D will be initialized at the beginning of each compute
 * command stream by the start_cs_cmd atom.  However, since the SET_CONTEXT_REG
 * packet requires that the shader type bit be set, we must initialize all
 * context registers needed for compute in this function.  The registers
 * initialized by the start_cs_cmd atom can be found in evergreen_state.c in the
 * functions evergreen_init_atom_start_cs or cayman_init_atom_start_cs depending
 * on the GPU family.
 */
void evergreen_init_atom_start_compute_cs(struct r600_context *rctx)
{
	struct r600_command_buffer *cb = &rctx->start_compute_cs_cmd;
	int num_threads;
	int num_stack_entries;

	/* since all required registers are initialized in the
	 * start_compute_cs_cmd atom, we can EMIT_EARLY here.
	 */
	r600_init_command_buffer(cb, 256);
	cb->pkt_flags = RADEON_CP_PACKET3_COMPUTE_MODE;

	/* We're setting config registers here. */
	r600_store_value(cb, PKT3(PKT3_EVENT_WRITE, 0, 0));
	r600_store_value(cb, EVENT_TYPE(EVENT_TYPE_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));

	switch (rctx->b.family) {
	case CHIP_CEDAR:
	default:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	case CHIP_REDWOOD:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	case CHIP_JUNIPER:
		num_threads = 128;
		num_stack_entries = 512;
		break;
	case CHIP_CYPRESS:
	case CHIP_HEMLOCK:
		num_threads = 128;
		num_stack_entries = 512;
		break;
	case CHIP_PALM:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	case CHIP_SUMO:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	case CHIP_SUMO2:
		num_threads = 128;
		num_stack_entries = 512;
		break;
	case CHIP_BARTS:
		num_threads = 128;
		num_stack_entries = 512;
		break;
	case CHIP_TURKS:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	case CHIP_CAICOS:
		num_threads = 128;
		num_stack_entries = 256;
		break;
	}

	/* The primitive type always needs to be POINTLIST for compute. */
	r600_store_config_reg(cb, R_008958_VGT_PRIMITIVE_TYPE,
						V_008958_DI_PT_POINTLIST);

	if (rctx->b.gfx_level < CAYMAN) {

		/* These registers control which simds can be used by each stage.
		 * The default for these registers is 0xffffffff, which means
		 * all simds are available for each stage.  It's possible we may
		 * want to play around with these in the future, but for now
		 * the default value is fine.
		 *
		 * R_008E20_SQ_STATIC_THREAD_MGMT1
		 * R_008E24_SQ_STATIC_THREAD_MGMT2
		 * R_008E28_SQ_STATIC_THREAD_MGMT3
		 */

		/* XXX: We may need to adjust the thread and stack resource
		 * values for 3D/compute interop */

		r600_store_config_reg_seq(cb, R_008C18_SQ_THREAD_RESOURCE_MGMT_1, 5);

		/* R_008C18_SQ_THREAD_RESOURCE_MGMT_1
		 * Set the number of threads used by the PS/VS/GS/ES stage to
		 * 0.
		 */
		r600_store_value(cb, 0);

		/* R_008C1C_SQ_THREAD_RESOURCE_MGMT_2
		 * Set the number of threads used by the CS (aka LS) stage to
		 * the maximum number of threads and set the number of threads
		 * for the HS stage to 0. */
		r600_store_value(cb, S_008C1C_NUM_LS_THREADS(num_threads));

		/* R_008C20_SQ_STACK_RESOURCE_MGMT_1
		 * Set the Control Flow stack entries to 0 for PS/VS stages */
		r600_store_value(cb, 0);

		/* R_008C24_SQ_STACK_RESOURCE_MGMT_2
		 * Set the Control Flow stack entries to 0 for GS/ES stages */
		r600_store_value(cb, 0);

		/* R_008C28_SQ_STACK_RESOURCE_MGMT_3
		 * Set the Control Flow stack entries to 0 for the HS stage, and
		 * set it to the maximum value for the CS (aka LS) stage. */
		r600_store_value(cb,
			S_008C28_NUM_LS_STACK_ENTRIES(num_stack_entries));
	}
	/* Give the compute shader all the available LDS space.
	 * NOTE: This only sets the maximum number of dwords that a compute
	 * shader can allocate.  When a shader is executed, we still need to
	 * allocate the appropriate amount of LDS dwords using the
	 * CM_R_0288E8_SQ_LDS_ALLOC register.
	 */
	if (rctx->b.gfx_level < CAYMAN) {
		r600_store_config_reg(cb, R_008E2C_SQ_LDS_RESOURCE_MGMT,
			S_008E2C_NUM_PS_LDS(0x0000) | S_008E2C_NUM_LS_LDS(8192));
	} else {
		r600_store_context_reg(cb, CM_R_0286FC_SPI_LDS_MGMT,
			S_0286FC_NUM_PS_LDS(0) |
			S_0286FC_NUM_LS_LDS(255)); /* 255 * 32 = 8160 dwords */
	}

	/* Context Registers */

	if (rctx->b.gfx_level < CAYMAN) {
		/* workaround for hw issues with dyn gpr - must set all limits
		 * to 240 instead of 0, 0x1e == 240 / 8
		 */
		r600_store_context_reg(cb, R_028838_SQ_DYN_GPR_RESOURCE_LIMIT_1,
				S_028838_PS_GPRS(0x1e) |
				S_028838_VS_GPRS(0x1e) |
				S_028838_GS_GPRS(0x1e) |
				S_028838_ES_GPRS(0x1e) |
				S_028838_HS_GPRS(0x1e) |
				S_028838_LS_GPRS(0x1e));
	}

	/* XXX: Investigate setting bit 15, which is FAST_COMPUTE_MODE */
	r600_store_context_reg(cb, R_028A40_VGT_GS_MODE,
		S_028A40_COMPUTE_MODE(1) | S_028A40_PARTIAL_THD_AT_EOI(1));

	r600_store_context_reg(cb, R_028B54_VGT_SHADER_STAGES_EN, 2/*CS_ON*/);

	r600_store_context_reg(cb, R_0286E8_SPI_COMPUTE_INPUT_CNTL,
			       S_0286E8_TID_IN_GROUP_ENA(1) |
			       S_0286E8_TGID_ENA(1) |
			       S_0286E8_DISABLE_INDEX_PACK(1));

	/* The LOOP_CONST registers are an optimizations for loops that allows
	 * you to store the initial counter, increment value, and maximum
	 * counter value in a register so that hardware can calculate the
	 * correct number of iterations for the loop, so that you don't need
	 * to have the loop counter in your shader code.  We don't currently use
	 * this optimization, so we must keep track of the counter in the
	 * shader and use a break instruction to exit loops.  However, the
	 * hardware will still uses this register to determine when to exit a
	 * loop, so we need to initialize the counter to 0, set the increment
	 * value to 1 and the maximum counter value to the 4095 (0xfff) which
	 * is the maximum value allowed.  This gives us a maximum of 4096
	 * iterations for our loops, but hopefully our break instruction will
	 * execute before some time before the 4096th iteration.
	 */
	eg_store_loop_const(cb, R_03A200_SQ_LOOP_CONST_0 + (160 * 4), 0x1000FFF);
}


static void evergreen_get_compute_state_info(struct pipe_context *ctx, void *state,
                                             struct pipe_compute_state_object_info *info)
{
	struct r600_context *rctx = (struct r600_context*)ctx;
	struct r600_pipe_compute *shader = state;
	
	/* This is somehow copied from RadeonSI, but in thruth this not more
	 * than an educated guess. */
	uint8_t wave_size = r600_wavefront_size(rctx->b.screen->family);
	info->private_memory = shader->sel->current->scratch_space_needed;
	info->preferred_simd_size = wave_size;
	info->simd_sizes = wave_size;
	info->max_threads = 128;
}

void evergreen_init_compute_state_functions(struct r600_context *rctx)
{
	rctx->b.b.create_compute_state = evergreen_create_compute_state;
	rctx->b.b.delete_compute_state = evergreen_delete_compute_state;
	rctx->b.b.bind_compute_state = evergreen_bind_compute_state;
//	 rctx->context.create_sampler_view = evergreen_compute_create_sampler_view;
	rctx->b.b.set_compute_resources = evergreen_set_compute_resources;
	rctx->b.b.set_global_binding = evergreen_set_global_binding;
	rctx->b.b.launch_grid = evergreen_launch_grid;
	rctx->b.b.get_compute_state_info = evergreen_get_compute_state_info;
}

void *r600_compute_global_transfer_map(struct pipe_context *ctx,
				      struct pipe_resource *resource,
				      unsigned level,
				      unsigned usage,
				      const struct pipe_box *box,
				      struct pipe_transfer **ptransfer)
{
	struct r600_context *rctx = (struct r600_context*)ctx;
	struct compute_memory_pool *pool = rctx->screen->global_pool;
	struct r600_resource_global* buffer =
		(struct r600_resource_global*)resource;

	struct compute_memory_item *item = buffer->chunk;
	struct pipe_resource *dst = NULL;
	unsigned offset = box->x;

	if (usage & PIPE_MAP_READ)
		buffer->chunk->status |= ITEM_MAPPED_FOR_READING;

	if (usage & PIPE_MAP_WRITE)
		buffer->chunk->status |= ITEM_MAPPED_FOR_WRITING;

	if (is_item_in_pool(item)) {
		compute_memory_demote_item(pool, item, ctx);
	}
	else {
		if (item->real_buffer == NULL) {
			item->real_buffer =
					r600_compute_buffer_alloc_vram(pool->screen, item->size_in_dw * 4);
		}
	}

	dst = (struct pipe_resource*)item->real_buffer;

	COMPUTE_DBG(rctx->screen, "* r600_compute_global_transfer_map()\n"
			"level = %u, usage = %u, box(x = %u, y = %u, z = %u "
			"width = %u, height = %u, depth = %u)\n", level, usage,
			box->x, box->y, box->z, box->width, box->height,
			box->depth);
	COMPUTE_DBG(rctx->screen, "Buffer id = %"PRIi64" offset = "
		"%u (box.x)\n", item->id, box->x);


	assert(resource->target == PIPE_BUFFER);
	assert(resource->bind & PIPE_BIND_GLOBAL);
	assert(box->x >= 0);
	assert(box->y == 0);
	assert(box->z == 0);

	if (buffer->base.b.is_user_ptr)
		return NULL;

	///TODO: do it better, mapping is not possible if the pool is too big
	return pipe_buffer_map_range(ctx, dst,
			offset, box->width, usage & ~PIPE_MAP_READ, ptransfer);
}

void r600_compute_global_transfer_unmap(struct pipe_context *ctx,
					struct pipe_transfer *transfer)
{
	/* struct r600_resource_global are not real resources, they just map
	 * to an offset within the compute memory pool.  The function
	 * r600_compute_global_transfer_map() maps the memory pool
	 * resource rather than the struct r600_resource_global passed to
	 * it as an argument and then initializes ptransfer->resource with
	 * the memory pool resource (via pipe_buffer_map_range).
	 * When transfer_unmap is called it uses the memory pool's
	 * vtable which calls r600_buffer_transfer_map() rather than
	 * this function.
	 */
	assert (!"This function should not be called");
}

void r600_compute_global_buffer_destroy(struct pipe_screen *screen,
					struct pipe_resource *res)
{
	struct r600_resource_global* buffer = NULL;
	struct r600_screen* rscreen = NULL;

	assert(res->target == PIPE_BUFFER);
	assert(res->bind & PIPE_BIND_GLOBAL);

	buffer = (struct r600_resource_global*)res;
	rscreen = (struct r600_screen*)screen;

	compute_memory_free(rscreen->global_pool, buffer->chunk->id);
	buffer->chunk = NULL;

	if (buffer->base.b.is_user_ptr)
		r600_buffer_destroy(screen, res);
	else
		free(res);
}

struct pipe_resource *r600_compute_global_buffer_create(struct pipe_screen *screen,
							const struct pipe_resource *templ)
{
	struct r600_resource_global* result = NULL;
	struct r600_screen* rscreen = NULL;
	int size_in_dw = 0;

	assert(templ->target == PIPE_BUFFER);
	assert(templ->bind & PIPE_BIND_GLOBAL);
	assert(templ->array_size == 1 || templ->array_size == 0);
	assert(templ->depth0 == 1 || templ->depth0 == 0);
	assert(templ->height0 == 1 || templ->height0 == 0);

	result = (struct r600_resource_global*)
	CALLOC(sizeof(struct r600_resource_global), 1);
	rscreen = (struct r600_screen*)screen;

	COMPUTE_DBG(rscreen, "*** r600_compute_global_buffer_create\n");
	COMPUTE_DBG(rscreen, "width = %u array_size = %u\n", templ->width0,
			templ->array_size);

	result->base.b.b = *templ;
	result->base.b.b.screen = screen;
	result->base.compute_global_bo = true;
	pipe_reference_init(&result->base.b.b.reference, 1);

	size_in_dw = (templ->width0+3) / 4;

	result->chunk = compute_memory_alloc(rscreen->global_pool, size_in_dw);

	if (result->chunk == NULL)
	{
		free(result);
		return NULL;
	}

	return &result->base.b.b;
}
