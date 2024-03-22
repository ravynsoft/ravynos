/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
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
 */
#include "nir_serialize.h"
#include "pipe/p_defines.h"
#include "r600_asm.h"
#include "r600_isa.h"
#include "r600_sq.h"
#include "r600_formats.h"
#include "r600_opcodes.h"
#include "r600_sfn.h"
#include "r600_shader.h"
#include "r600_dump.h"
#include "r600d.h"
#include "sfn/sfn_nir.h"

#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_from_mesa.h"
#include "nir/tgsi_to_nir.h"
#include "nir/nir_to_tgsi_info.h"
#include "compiler/nir/nir.h"
#include "util/macros.h"
#include "util/u_bitcast.h"
#include "util/u_dump.h"
#include "util/u_endian.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>

/* CAYMAN notes
Why CAYMAN got loops for lots of instructions is explained here.

-These 8xx t-slot only ops are implemented in all vector slots.
MUL_LIT, FLT_TO_UINT, INT_TO_FLT, UINT_TO_FLT
These 8xx t-slot only opcodes become vector ops, with all four
slots expecting the arguments on sources a and b. Result is
broadcast to all channels.
MULLO_INT, MULHI_INT, MULLO_UINT, MULHI_UINT, MUL_64
These 8xx t-slot only opcodes become vector ops in the z, y, and
x slots.
EXP_IEEE, LOG_IEEE/CLAMPED, RECIP_IEEE/CLAMPED/FF/INT/UINT/_64/CLAMPED_64
RECIPSQRT_IEEE/CLAMPED/FF/_64/CLAMPED_64
SQRT_IEEE/_64
SIN/COS
The w slot may have an independent co-issued operation, or if the
result is required to be in the w slot, the opcode above may be
issued in the w slot as well.
The compiler must issue the source argument to slots z, y, and x
*/

/* Contents of r0 on entry to various shaders

 VS - .x = VertexID
      .y = RelVertexID (??)
      .w = InstanceID

 GS - r0.xyw, r1.xyz = per-vertex offsets
      r0.z = PrimitiveID

 TCS - .x = PatchID
       .y = RelPatchID (??)
       .z = InvocationID
       .w = tess factor base.

 TES - .x = TessCoord.x
     - .y = TessCoord.y
     - .z = RelPatchID (??)
     - .w = PrimitiveID

 PS - face_gpr.z = SampleMask
      face_gpr.w = SampleID
*/

static void r600_dump_streamout(struct pipe_stream_output_info *so)
{
	unsigned i;

	fprintf(stderr, "STREAMOUT\n");
	for (i = 0; i < so->num_outputs; i++) {
		unsigned mask = ((1 << so->output[i].num_components) - 1) <<
				so->output[i].start_component;
		fprintf(stderr, "  %i: MEM_STREAM%d_BUF%i[%i..%i] <- OUT[%i].%s%s%s%s%s\n",
			i,
			so->output[i].stream,
			so->output[i].output_buffer,
			so->output[i].dst_offset, so->output[i].dst_offset + so->output[i].num_components - 1,
			so->output[i].register_index,
			mask & 1 ? "x" : "",
		        mask & 2 ? "y" : "",
		        mask & 4 ? "z" : "",
		        mask & 8 ? "w" : "",
			so->output[i].dst_offset < so->output[i].start_component ? " (will lower)" : "");
	}
}

static int store_shader(struct pipe_context *ctx,
			struct r600_pipe_shader *shader)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	uint32_t *ptr, i;

	if (shader->bo == NULL) {
		shader->bo = (struct r600_resource*)
			pipe_buffer_create(ctx->screen, 0, PIPE_USAGE_IMMUTABLE, shader->shader.bc.ndw * 4);
		if (shader->bo == NULL) {
			return -ENOMEM;
		}
		ptr = r600_buffer_map_sync_with_rings(
			&rctx->b, shader->bo,
			PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
		if (UTIL_ARCH_BIG_ENDIAN) {
			for (i = 0; i < shader->shader.bc.ndw; ++i) {
				ptr[i] = util_cpu_to_le32(shader->shader.bc.bytecode[i]);
			}
		} else {
			memcpy(ptr, shader->shader.bc.bytecode, shader->shader.bc.ndw * sizeof(*ptr));
		}
		rctx->b.ws->buffer_unmap(rctx->b.ws, shader->bo->buf);
	}

	return 0;
}

extern const struct nir_shader_compiler_options r600_nir_options;
static int nshader = 0;
int r600_pipe_shader_create(struct pipe_context *ctx,
			    struct r600_pipe_shader *shader,
			    union r600_shader_key key)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_pipe_shader_selector *sel = shader->selector;
	int r;
	const nir_shader_compiler_options *nir_options =
		(const nir_shader_compiler_options *)
			ctx->screen->get_compiler_options(ctx->screen,
		                                     PIPE_SHADER_IR_NIR,
		                                     shader->shader.processor_type);
	if (!sel->nir && !(sel->ir_type == PIPE_SHADER_IR_TGSI)) {
		assert(sel->nir_blob);
		struct blob_reader blob_reader;
		blob_reader_init(&blob_reader, sel->nir_blob, sel->nir_blob_size);
		sel->nir = nir_deserialize(NULL, nir_options, &blob_reader);
	}

	int processor = sel->ir_type == PIPE_SHADER_IR_TGSI ?
		tgsi_get_processor_type(sel->tokens):
		pipe_shader_type_from_mesa(sel->nir->info.stage);
	
	bool dump = r600_can_dump_shader(&rctx->screen->b, processor);

	unsigned export_shader;
	
	shader->shader.bc.isa = rctx->isa;
	
	{
		glsl_type_singleton_init_or_ref();
		if (sel->ir_type == PIPE_SHADER_IR_TGSI) {
			if (sel->nir)
				ralloc_free(sel->nir);
			if (sel->nir_blob) {
				free(sel->nir_blob);
				sel->nir_blob = NULL;
			}
			sel->nir = tgsi_to_nir(sel->tokens, ctx->screen, true);
			/* Lower int64 ops because we have some r600 built-in shaders that use it */
			if (nir_options->lower_int64_options) {
				NIR_PASS_V(sel->nir, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
				NIR_PASS_V(sel->nir, nir_lower_int64);
			}
			NIR_PASS_V(sel->nir, nir_lower_flrp, ~0, false);
		}
		nir_tgsi_scan_shader(sel->nir, &sel->info, true);

		r = r600_shader_from_nir(rctx, shader, &key);

		glsl_type_singleton_decref();

		if (r) {
			fprintf(stderr, "--Failed shader--------------------------------------------------\n");
			
			if (sel->ir_type == PIPE_SHADER_IR_TGSI) {
				fprintf(stderr, "--TGSI--------------------------------------------------------\n");
				tgsi_dump(sel->tokens, 0);
			}
			
			fprintf(stderr, "--NIR --------------------------------------------------------\n");
			nir_print_shader(sel->nir, stderr);
			
			R600_ERR("translation from NIR failed !\n");
			goto error;
		}
	}
	
	if (dump) {
		if (sel->ir_type == PIPE_SHADER_IR_TGSI) {
			fprintf(stderr, "--TGSI--------------------------------------------------------\n");
			tgsi_dump(sel->tokens, 0);
		}
		
		if (sel->so.num_outputs) {
			r600_dump_streamout(&sel->so);
		}
	}

	/* Check if the bytecode has already been built. */
	if (!shader->shader.bc.bytecode) {
		r = r600_bytecode_build(&shader->shader.bc);
		if (r) {
			R600_ERR("building bytecode failed !\n");
			goto error;
		}
	}

	if (dump) {
		fprintf(stderr, "--------------------------------------------------------------\n");
		r600_bytecode_disasm(&shader->shader.bc);
		fprintf(stderr, "______________________________________________________________\n");

                print_shader_info(stderr, nshader++, &shader->shader);
		print_pipe_info(stderr, &sel->info);
	}

	if (shader->gs_copy_shader) {
		if (dump) {
			// dump copy shader
			r600_bytecode_disasm(&shader->gs_copy_shader->shader.bc);
                }

		if ((r = store_shader(ctx, shader->gs_copy_shader)))
			goto error;
	}

	/* Store the shader in a buffer. */
	if ((r = store_shader(ctx, shader)))
		goto error;

	/* Build state. */
	switch (shader->shader.processor_type) {
	case PIPE_SHADER_TESS_CTRL:
		evergreen_update_hs_state(ctx, shader);
		break;
	case PIPE_SHADER_TESS_EVAL:
		if (key.tes.as_es)
			evergreen_update_es_state(ctx, shader);
		else
			evergreen_update_vs_state(ctx, shader);
		break;
	case PIPE_SHADER_GEOMETRY:
		if (rctx->b.gfx_level >= EVERGREEN) {
			evergreen_update_gs_state(ctx, shader);
			evergreen_update_vs_state(ctx, shader->gs_copy_shader);
		} else {
			r600_update_gs_state(ctx, shader);
			r600_update_vs_state(ctx, shader->gs_copy_shader);
		}
		break;
	case PIPE_SHADER_VERTEX:
		export_shader = key.vs.as_es;
		if (rctx->b.gfx_level >= EVERGREEN) {
			if (key.vs.as_ls)
				evergreen_update_ls_state(ctx, shader);
			else if (key.vs.as_es)
				evergreen_update_es_state(ctx, shader);
			else
				evergreen_update_vs_state(ctx, shader);
		} else {
			if (export_shader)
				r600_update_es_state(ctx, shader);
			else
				r600_update_vs_state(ctx, shader);
		}
		break;
	case PIPE_SHADER_FRAGMENT:
		if (rctx->b.gfx_level >= EVERGREEN) {
			evergreen_update_ps_state(ctx, shader);
		} else {
			r600_update_ps_state(ctx, shader);
		}
		break;
	case PIPE_SHADER_COMPUTE:
		evergreen_update_ls_state(ctx, shader);
		break;
	default:
		r = -EINVAL;
		goto error;
	}

	util_debug_message(&rctx->b.debug, SHADER_INFO, "%s shader: %d dw, %d gprs, %d alu_groups, %d loops, %d cf, %d stack",
		           _mesa_shader_stage_to_abbrev(tgsi_processor_to_shader_stage(processor)),
	                   shader->shader.bc.ndw,
	                   shader->shader.bc.ngpr,
			   shader->shader.bc.nalu_groups,
			   shader->shader.num_loops,
			   shader->shader.bc.ncf,
			   shader->shader.bc.nstack);

	if (!sel->nir_blob && sel->nir && sel->ir_type != PIPE_SHADER_IR_TGSI) {
		struct blob blob;
		blob_init(&blob);
		nir_serialize(&blob, sel->nir, false);
		sel->nir_blob = malloc(blob.size);
		memcpy(sel->nir_blob, blob.data, blob.size);
		sel->nir_blob_size = blob.size;
		blob_finish(&blob);
	}
	ralloc_free(sel->nir);
	sel->nir = NULL;

	return 0;

error:
	r600_pipe_shader_destroy(ctx, shader);
	return r;
}

void r600_pipe_shader_destroy(struct pipe_context *ctx UNUSED, struct r600_pipe_shader *shader)
{
	r600_resource_reference(&shader->bo, NULL);
	if (list_is_linked(&shader->shader.bc.cf))
		r600_bytecode_clear(&shader->shader.bc);
	r600_release_command_buffer(&shader->command_buffer);

	if (shader->shader.arrays)
		free(shader->shader.arrays);
}

struct r600_shader_src {
	unsigned				sel;
	unsigned				swizzle[4];
	unsigned				neg;
	unsigned				abs;
	unsigned				rel;
	unsigned				kc_bank;
	bool					kc_rel; /* true if cache bank is indexed */
	uint32_t				value[4];
};

struct eg_interp {
	bool					enabled;
	unsigned				ij_index;
};

struct r600_shader_ctx {
	unsigned				type;
	unsigned				temp_reg;
	struct r600_bytecode			*bc;
	struct r600_shader			*shader;
	uint32_t				max_driver_temp_used;
	unsigned				enabled_stream_buffers_mask;
};

void *r600_create_vertex_fetch_shader(struct pipe_context *ctx,
				      unsigned count,
				      const struct pipe_vertex_element *elements)
{
	struct r600_context *rctx = (struct r600_context *)ctx;
	struct r600_bytecode bc;
	struct r600_bytecode_vtx vtx;
	const struct util_format_description *desc;
	unsigned fetch_resource_start = rctx->b.gfx_level >= EVERGREEN ? 0 : 160;
	unsigned format, num_format, format_comp, endian;
	uint32_t *bytecode;
	int i, j, r, fs_size;
	uint32_t buffer_mask = 0;
	struct r600_fetch_shader *shader;
	unsigned strides[PIPE_MAX_ATTRIBS];

	assert(count < 32);

	memset(&bc, 0, sizeof(bc));
	r600_bytecode_init(&bc, rctx->b.gfx_level, rctx->b.family,
			   rctx->screen->has_compressed_msaa_texturing);

	bc.isa = rctx->isa;

	for (i = 0; i < count; i++) {
		if (elements[i].instance_divisor > 1) {
			if (rctx->b.gfx_level == CAYMAN) {
				for (j = 0; j < 4; j++) {
					struct r600_bytecode_alu alu;
					memset(&alu, 0, sizeof(alu));
					alu.op = ALU_OP2_MULHI_UINT;
					alu.src[0].sel = 0;
					alu.src[0].chan = 3;
					alu.src[1].sel = V_SQ_ALU_SRC_LITERAL;
					alu.src[1].value = (1ll << 32) / elements[i].instance_divisor + 1;
					alu.dst.sel = i + 1;
					alu.dst.chan = j;
					alu.dst.write = j == 3;
					alu.last = j == 3;
					if ((r = r600_bytecode_add_alu(&bc, &alu))) {
						r600_bytecode_clear(&bc);
						return NULL;
					}
				}
			} else {
				struct r600_bytecode_alu alu;
				memset(&alu, 0, sizeof(alu));
				alu.op = ALU_OP2_MULHI_UINT;
				alu.src[0].sel = 0;
				alu.src[0].chan = 3;
				alu.src[1].sel = V_SQ_ALU_SRC_LITERAL;
				alu.src[1].value = (1ll << 32) / elements[i].instance_divisor + 1;
				alu.dst.sel = i + 1;
				alu.dst.chan = 3;
				alu.dst.write = 1;
				alu.last = 1;
				if ((r = r600_bytecode_add_alu(&bc, &alu))) {
					r600_bytecode_clear(&bc);
					return NULL;
				}
			}
		}
		strides[elements[i].vertex_buffer_index] = elements[i].src_stride;
		buffer_mask |= BITFIELD_BIT(elements[i].vertex_buffer_index);
	}

	for (i = 0; i < count; i++) {
		r600_vertex_data_type(elements[i].src_format,
				      &format, &num_format, &format_comp, &endian);

		desc = util_format_description(elements[i].src_format);

		if (elements[i].src_offset > 65535) {
			r600_bytecode_clear(&bc);
			R600_ERR("too big src_offset: %u\n", elements[i].src_offset);
			return NULL;
		}

		memset(&vtx, 0, sizeof(vtx));
		vtx.buffer_id = elements[i].vertex_buffer_index + fetch_resource_start;
		vtx.fetch_type = elements[i].instance_divisor ? SQ_VTX_FETCH_INSTANCE_DATA : SQ_VTX_FETCH_VERTEX_DATA;
		vtx.src_gpr = elements[i].instance_divisor > 1 ? i + 1 : 0;
		vtx.src_sel_x = elements[i].instance_divisor ? 3 : 0;
		vtx.mega_fetch_count = 0x1F;
		vtx.dst_gpr = i + 1;
		vtx.dst_sel_x = desc->swizzle[0];
		vtx.dst_sel_y = desc->swizzle[1];
		vtx.dst_sel_z = desc->swizzle[2];
		vtx.dst_sel_w = desc->swizzle[3];
		vtx.data_format = format;
		vtx.num_format_all = num_format;
		vtx.format_comp_all = format_comp;
		vtx.offset = elements[i].src_offset;
		vtx.endian = endian;

		if ((r = r600_bytecode_add_vtx(&bc, &vtx))) {
			r600_bytecode_clear(&bc);
			return NULL;
		}
	}

	r600_bytecode_add_cfinst(&bc, CF_OP_RET);

	if ((r = r600_bytecode_build(&bc))) {
		r600_bytecode_clear(&bc);
		return NULL;
	}

	if (rctx->screen->b.debug_flags & DBG_FS) {
		fprintf(stderr, "--------------------------------------------------------------\n");
		fprintf(stderr, "Vertex elements state:\n");
		for (i = 0; i < count; i++) {
			fprintf(stderr, "   ");
			util_dump_vertex_element(stderr, elements+i);
			fprintf(stderr, "\n");
		}

                r600_bytecode_disasm(&bc);
	}

	fs_size = bc.ndw*4;

	/* Allocate the CSO. */
	shader = CALLOC_STRUCT(r600_fetch_shader);
	if (!shader) {
		r600_bytecode_clear(&bc);
		return NULL;
	}
	memcpy(shader->strides, strides, sizeof(strides));
	shader->buffer_mask = buffer_mask;

	u_suballocator_alloc(&rctx->allocator_fetch_shader, fs_size, 256,
			     &shader->offset,
			     (struct pipe_resource**)&shader->buffer);
	if (!shader->buffer) {
		r600_bytecode_clear(&bc);
		FREE(shader);
		return NULL;
	}

	bytecode = r600_buffer_map_sync_with_rings
		(&rctx->b, shader->buffer,
		PIPE_MAP_WRITE | PIPE_MAP_UNSYNCHRONIZED | RADEON_MAP_TEMPORARY);
	bytecode += shader->offset / 4;

	if (UTIL_ARCH_BIG_ENDIAN) {
		for (i = 0; i < fs_size / 4; ++i) {
			bytecode[i] = util_cpu_to_le32(bc.bytecode[i]);
		}
	} else {
		memcpy(bytecode, bc.bytecode, fs_size);
	}
	rctx->b.ws->buffer_unmap(rctx->b.ws, shader->buffer->buf);

	r600_bytecode_clear(&bc);
	return shader;
}

int eg_get_interpolator_index(unsigned interpolate, unsigned location)
{
	if (interpolate == TGSI_INTERPOLATE_COLOR ||
		interpolate == TGSI_INTERPOLATE_LINEAR ||
		interpolate == TGSI_INTERPOLATE_PERSPECTIVE)
	{
		int is_linear = interpolate == TGSI_INTERPOLATE_LINEAR;
		int loc;

		switch(location) {
		case TGSI_INTERPOLATE_LOC_CENTER:
			loc = 1;
			break;
		case TGSI_INTERPOLATE_LOC_CENTROID:
			loc = 2;
			break;
		case TGSI_INTERPOLATE_LOC_SAMPLE:
		default:
			loc = 0; break;
		}

		return is_linear * 3 + loc;
	}

	return -1;
}

int r600_get_lds_unique_index(unsigned semantic_name, unsigned index)
{
	switch (semantic_name) {
	case TGSI_SEMANTIC_POSITION:
		return 0;
       case TGSI_SEMANTIC_PSIZE:
		return 1;
       case TGSI_SEMANTIC_CLIPDIST:
		assert(index <= 1);
		return 2 + index;
       case TGSI_SEMANTIC_TEXCOORD:
		return 4 + index;
       case TGSI_SEMANTIC_COLOR:
		return 12 + index;
       case TGSI_SEMANTIC_BCOLOR:
		return 14 + index;
       case TGSI_SEMANTIC_CLIPVERTEX:
		return 16;
       case TGSI_SEMANTIC_GENERIC:
		if (index <= 63-17)
			return 17 + index;
		else
			/* same explanation as in the default statement,
			 * the only user hitting this is st/nine.
			 */
			return 0;

	/* patch indices are completely separate and thus start from 0 */
	case TGSI_SEMANTIC_TESSOUTER:
		return 0;
	case TGSI_SEMANTIC_TESSINNER:
		return 1;
	case TGSI_SEMANTIC_PATCH:
		return 2 + index;

	default:
		/* Don't fail here. The result of this function is only used
		 * for LS, TCS, TES, and GS, where legacy GL semantics can't
		 * occur, but this function is called for all vertex shaders
		 * before it's known whether LS will be compiled or not.
		 */
		return 0;
	}
}

static int emit_streamout(struct r600_shader_ctx *ctx, struct pipe_stream_output_info *so,
                          int stream, unsigned *stream_item_size UNUSED)
{
	unsigned so_gpr[PIPE_MAX_SHADER_OUTPUTS];
	unsigned start_comp[PIPE_MAX_SHADER_OUTPUTS];
	int j, r;
	unsigned i;

	/* Sanity checking. */
	if (so->num_outputs > PIPE_MAX_SO_OUTPUTS) {
		R600_ERR("Too many stream outputs: %d\n", so->num_outputs);
		r = -EINVAL;
		goto out_err;
	}
	for (i = 0; i < so->num_outputs; i++) {
		if (so->output[i].output_buffer >= 4) {
			R600_ERR("Exceeded the max number of stream output buffers, got: %d\n",
				 so->output[i].output_buffer);
			r = -EINVAL;
			goto out_err;
		}
	}

	if (so->num_outputs && ctx->bc->cf_last->op != CF_OP_ALU &&
            ctx->bc->cf_last->op != CF_OP_ALU_PUSH_BEFORE)
		ctx->bc->force_add_cf = 1;
	/* Initialize locations where the outputs are stored. */
	for (i = 0; i < so->num_outputs; i++) {

		so_gpr[i] = ctx->shader->output[so->output[i].register_index].gpr;
		start_comp[i] = so->output[i].start_component;
		/* Lower outputs with dst_offset < start_component.
		 *
		 * We can only output 4D vectors with a write mask, e.g. we can
		 * only output the W component at offset 3, etc. If we want
		 * to store Y, Z, or W at buffer offset 0, we need to use MOV
		 * to move it to X and output X. */
		if (so->output[i].dst_offset < so->output[i].start_component) {
			unsigned tmp = ctx->temp_reg + ctx->max_driver_temp_used++;

			for (j = 0; j < so->output[i].num_components; j++) {
				struct r600_bytecode_alu alu;
				memset(&alu, 0, sizeof(struct r600_bytecode_alu));
				alu.op = ALU_OP1_MOV;
				alu.src[0].sel = so_gpr[i];
				alu.src[0].chan = so->output[i].start_component + j;

				alu.dst.sel = tmp;
				alu.dst.chan = j;
				alu.dst.write = 1;
				if (j == so->output[i].num_components - 1)
					alu.last = 1;
				r = r600_bytecode_add_alu(ctx->bc, &alu);
				if (r)
					return r;
			}
			start_comp[i] = 0;
			so_gpr[i] = tmp;
		}
	}

	/* Write outputs to buffers. */
	for (i = 0; i < so->num_outputs; i++) {
		struct r600_bytecode_output output;

		if (stream != -1 && stream != so->output[i].stream)
			continue;

		memset(&output, 0, sizeof(struct r600_bytecode_output));
		output.gpr = so_gpr[i];
		output.elem_size = so->output[i].num_components - 1;
		if (output.elem_size == 2)
			output.elem_size = 3; // 3 not supported, write 4 with junk at end
		output.array_base = so->output[i].dst_offset - start_comp[i];
		output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE;
		output.burst_count = 1;
		/* array_size is an upper limit for the burst_count
		 * with MEM_STREAM instructions */
		output.array_size = 0xFFF;
		output.comp_mask = ((1 << so->output[i].num_components) - 1) << start_comp[i];

		if (ctx->bc->gfx_level >= EVERGREEN) {
			switch (so->output[i].output_buffer) {
			case 0:
				output.op = CF_OP_MEM_STREAM0_BUF0;
				break;
			case 1:
				output.op = CF_OP_MEM_STREAM0_BUF1;
				break;
			case 2:
				output.op = CF_OP_MEM_STREAM0_BUF2;
				break;
			case 3:
				output.op = CF_OP_MEM_STREAM0_BUF3;
				break;
			}
			output.op += so->output[i].stream * 4;
			assert(output.op >= CF_OP_MEM_STREAM0_BUF0 && output.op <= CF_OP_MEM_STREAM3_BUF3);
			ctx->enabled_stream_buffers_mask |= (1 << so->output[i].output_buffer) << so->output[i].stream * 4;
		} else {
			switch (so->output[i].output_buffer) {
			case 0:
				output.op = CF_OP_MEM_STREAM0;
				break;
			case 1:
				output.op = CF_OP_MEM_STREAM1;
				break;
			case 2:
				output.op = CF_OP_MEM_STREAM2;
				break;
			case 3:
				output.op = CF_OP_MEM_STREAM3;
					break;
			}
			ctx->enabled_stream_buffers_mask |= 1 << so->output[i].output_buffer;
		}
		r = r600_bytecode_add_output(ctx->bc, &output);
		if (r)
			goto out_err;
	}
	return 0;
out_err:
	return r;
}

int generate_gs_copy_shader(struct r600_context *rctx,
                            struct r600_pipe_shader *gs,
                            struct pipe_stream_output_info *so)
{
	struct r600_shader_ctx ctx = {};
	struct r600_shader *gs_shader = &gs->shader;
	struct r600_pipe_shader *cshader;
	unsigned ocnt = gs_shader->noutput;
	struct r600_bytecode_alu alu;
	struct r600_bytecode_vtx vtx;
	struct r600_bytecode_output output;
	struct r600_bytecode_cf *cf_jump, *cf_pop,
		*last_exp_pos = NULL, *last_exp_param = NULL;
	int next_clip_pos = 61, next_param = 0;
	unsigned i, j;
	int ring;
	bool only_ring_0 = true;
	cshader = calloc(1, sizeof(struct r600_pipe_shader));
	if (!cshader)
		return 0;

	memcpy(cshader->shader.output, gs_shader->output, ocnt *
	       sizeof(struct r600_shader_io));

	cshader->shader.noutput = ocnt;

	ctx.shader = &cshader->shader;
	ctx.bc = &ctx.shader->bc;
	ctx.type = ctx.bc->type = PIPE_SHADER_VERTEX;

	r600_bytecode_init(ctx.bc, rctx->b.gfx_level, rctx->b.family,
			   rctx->screen->has_compressed_msaa_texturing);

	ctx.bc->isa = rctx->isa;

	cf_jump = NULL;
	memset(cshader->shader.ring_item_sizes, 0, sizeof(cshader->shader.ring_item_sizes));

	/* R0.x = R0.x & 0x3fffffff */
	memset(&alu, 0, sizeof(alu));
	alu.op = ALU_OP2_AND_INT;
	alu.src[1].sel = V_SQ_ALU_SRC_LITERAL;
	alu.src[1].value = 0x3fffffff;
	alu.dst.write = 1;
	r600_bytecode_add_alu(ctx.bc, &alu);

	/* R0.y = R0.x >> 30 */
	memset(&alu, 0, sizeof(alu));
	alu.op = ALU_OP2_LSHR_INT;
	alu.src[1].sel = V_SQ_ALU_SRC_LITERAL;
	alu.src[1].value = 0x1e;
	alu.dst.chan = 1;
	alu.dst.write = 1;
	alu.last = 1;
	r600_bytecode_add_alu(ctx.bc, &alu);

	/* fetch vertex data from GSVS ring */
	for (i = 0; i < ocnt; ++i) {
		struct r600_shader_io *out = &ctx.shader->output[i];

		out->gpr = i + 1;
		out->ring_offset = i * 16;

		memset(&vtx, 0, sizeof(vtx));
		vtx.op = FETCH_OP_VFETCH;
		vtx.buffer_id = R600_GS_RING_CONST_BUFFER;
		vtx.fetch_type = SQ_VTX_FETCH_NO_INDEX_OFFSET;
		vtx.mega_fetch_count = 16;
		vtx.offset = out->ring_offset;
		vtx.dst_gpr = out->gpr;
		vtx.src_gpr = 0;
		vtx.dst_sel_x = 0;
		vtx.dst_sel_y = 1;
		vtx.dst_sel_z = 2;
		vtx.dst_sel_w = 3;
		if (rctx->b.gfx_level >= EVERGREEN) {
			vtx.use_const_fields = 1;
		} else {
			vtx.data_format = FMT_32_32_32_32_FLOAT;
		}

		r600_bytecode_add_vtx(ctx.bc, &vtx);
	}
	ctx.temp_reg = i + 1;
	for (ring = 3; ring >= 0; --ring) {
		bool enabled = false;
		for (i = 0; i < so->num_outputs; i++) {
			if (so->output[i].stream == ring) {
				enabled = true;
				if (ring > 0)
					only_ring_0 = false;
				break;
			}
		}
		if (ring != 0 && !enabled) {
			cshader->shader.ring_item_sizes[ring] = 0;
			continue;
		}

		if (cf_jump) {
			// Patch up jump label
			r600_bytecode_add_cfinst(ctx.bc, CF_OP_POP);
			cf_pop = ctx.bc->cf_last;

			cf_jump->cf_addr = cf_pop->id + 2;
			cf_jump->pop_count = 1;
			cf_pop->cf_addr = cf_pop->id + 2;
			cf_pop->pop_count = 1;
		}

		/* PRED_SETE_INT __, R0.y, ring */
		memset(&alu, 0, sizeof(alu));
		alu.op = ALU_OP2_PRED_SETE_INT;
		alu.src[0].chan = 1;
		alu.src[1].sel = V_SQ_ALU_SRC_LITERAL;
		alu.src[1].value = ring;
		alu.execute_mask = 1;
		alu.update_pred = 1;
		alu.last = 1;
		ctx.bc->force_add_cf = 1;
		r600_bytecode_add_alu_type(ctx.bc, &alu, CF_OP_ALU_PUSH_BEFORE);

		r600_bytecode_add_cfinst(ctx.bc, CF_OP_JUMP);
		cf_jump = ctx.bc->cf_last;

		if (enabled)
			emit_streamout(&ctx, so, only_ring_0 ? -1 : ring, &cshader->shader.ring_item_sizes[ring]);
		cshader->shader.ring_item_sizes[ring] = ocnt * 16;
	}

	/* bc adds nops - copy it */
	if (ctx.bc->gfx_level == R600) {
		ctx.bc->force_add_cf = 1;
		memset(&alu, 0, sizeof(struct r600_bytecode_alu));
		alu.op = ALU_OP0_NOP;
		alu.last = 1;
		r600_bytecode_add_alu(ctx.bc, &alu);

		r600_bytecode_add_cfinst(ctx.bc, CF_OP_NOP);
	}

	/* export vertex data */
	/* XXX factor out common code with r600_shader_from_tgsi ? */
	for (i = 0; i < ocnt; ++i) {
		struct r600_shader_io *out = &ctx.shader->output[i];
		/* The actual parameter export indices will be calculated here, ignore the copied ones. */
		out->export_param = -1;
		bool instream0 = true;
		if (out->varying_slot == VARYING_SLOT_CLIP_VERTEX)
			continue;

		for (j = 0; j < so->num_outputs; j++) {
			if (so->output[j].register_index == i) {
				if (so->output[j].stream == 0)
					break;
				if (so->output[j].stream > 0)
					instream0 = false;
			}
		}
		if (!instream0)
			continue;
		memset(&output, 0, sizeof(output));
		output.gpr = out->gpr;
		output.elem_size = 3;
		output.swizzle_x = 0;
		output.swizzle_y = 1;
		output.swizzle_z = 2;
		output.swizzle_w = 3;
		output.burst_count = 1;
		output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PARAM;
		output.op = CF_OP_EXPORT;
		switch (out->varying_slot) {
		case VARYING_SLOT_POS:
			output.array_base = 60;
			output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
			break;

		case VARYING_SLOT_PSIZ:
			output.array_base = 61;
			if (next_clip_pos == 61)
				next_clip_pos = 62;
			output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
			output.swizzle_y = 7;
			output.swizzle_z = 7;
			output.swizzle_w = 7;
			ctx.shader->vs_out_misc_write = 1;
			ctx.shader->vs_out_point_size = 1;
			break;
		case VARYING_SLOT_LAYER:
			if (out->spi_sid) {
				/* duplicate it as PARAM to pass to the pixel shader */
				output.array_base = next_param++;
				out->export_param = output.array_base;
				r600_bytecode_add_output(ctx.bc, &output);
				last_exp_param = ctx.bc->cf_last;
			}
			output.array_base = 61;
			if (next_clip_pos == 61)
				next_clip_pos = 62;
			output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
			output.swizzle_x = 7;
			output.swizzle_y = 7;
			output.swizzle_z = 0;
			output.swizzle_w = 7;
			ctx.shader->vs_out_misc_write = 1;
			ctx.shader->vs_out_layer = 1;
			break;
		case VARYING_SLOT_VIEWPORT:
			if (out->spi_sid) {
				/* duplicate it as PARAM to pass to the pixel shader */
				output.array_base = next_param++;
				out->export_param = output.array_base;
				r600_bytecode_add_output(ctx.bc, &output);
				last_exp_param = ctx.bc->cf_last;
			}
			output.array_base = 61;
			if (next_clip_pos == 61)
				next_clip_pos = 62;
			output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
			ctx.shader->vs_out_misc_write = 1;
			ctx.shader->vs_out_viewport = 1;
			output.swizzle_x = 7;
			output.swizzle_y = 7;
			output.swizzle_z = 7;
			output.swizzle_w = 0;
			break;
		case VARYING_SLOT_CLIP_DIST0:
		case VARYING_SLOT_CLIP_DIST1:
			/* spi_sid is 0 for clipdistance outputs that were generated
			 * for clipvertex - we don't need to pass them to PS */
			ctx.shader->clip_dist_write = gs->shader.clip_dist_write;
			ctx.shader->cull_dist_write = gs->shader.cull_dist_write;
			ctx.shader->cc_dist_mask = gs->shader.cc_dist_mask;
			if (out->spi_sid) {
				/* duplicate it as PARAM to pass to the pixel shader */
				output.array_base = next_param++;
				out->export_param = output.array_base;
				r600_bytecode_add_output(ctx.bc, &output);
				last_exp_param = ctx.bc->cf_last;
			}
			output.array_base = next_clip_pos++;
			output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
			break;
		case VARYING_SLOT_FOGC:
			output.swizzle_y = 4; /* 0 */
			output.swizzle_z = 4; /* 0 */
			output.swizzle_w = 5; /* 1 */
			break;
		default:
			break;
		}
		if (output.type == V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PARAM) {
			output.array_base = next_param++;
			out->export_param = output.array_base;
		}
		r600_bytecode_add_output(ctx.bc, &output);
		if (output.type == V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PARAM)
			last_exp_param = ctx.bc->cf_last;
		else
			last_exp_pos = ctx.bc->cf_last;
	}

	if (!last_exp_pos) {
		memset(&output, 0, sizeof(output));
		output.gpr = 0;
		output.elem_size = 3;
		output.swizzle_x = 7;
		output.swizzle_y = 7;
		output.swizzle_z = 7;
		output.swizzle_w = 7;
		output.burst_count = 1;
		output.type = 2;
		output.op = CF_OP_EXPORT;
		output.array_base = 60;
		output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS;
		r600_bytecode_add_output(ctx.bc, &output);
		last_exp_pos = ctx.bc->cf_last;
	}

	if (!last_exp_param) {
		memset(&output, 0, sizeof(output));
		output.gpr = 0;
		output.elem_size = 3;
		output.swizzle_x = 7;
		output.swizzle_y = 7;
		output.swizzle_z = 7;
		output.swizzle_w = 7;
		output.burst_count = 1;
		output.type = 2;
		output.op = CF_OP_EXPORT;
		output.array_base = next_param++;
		output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PARAM;
		r600_bytecode_add_output(ctx.bc, &output);
		last_exp_param = ctx.bc->cf_last;
	}

	last_exp_pos->op = CF_OP_EXPORT_DONE;
	last_exp_param->op = CF_OP_EXPORT_DONE;

	assert(next_param > 0);
	cshader->shader.highest_export_param = next_param - 1;

	r600_bytecode_add_cfinst(ctx.bc, CF_OP_POP);
	cf_pop = ctx.bc->cf_last;

	cf_jump->cf_addr = cf_pop->id + 2;
	cf_jump->pop_count = 1;
	cf_pop->cf_addr = cf_pop->id + 2;
	cf_pop->pop_count = 1;

	if (ctx.bc->gfx_level == CAYMAN)
		cm_bytecode_add_cf_end(ctx.bc);
	else {
		r600_bytecode_add_cfinst(ctx.bc, CF_OP_NOP);
		ctx.bc->cf_last->end_of_program = 1;
	}

	gs->gs_copy_shader = cshader;
	cshader->enabled_stream_buffers_mask = ctx.enabled_stream_buffers_mask;

	ctx.bc->nstack = 1;

	return r600_bytecode_build(ctx.bc);
}

