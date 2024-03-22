/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#include "util/compiler.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_defines.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_scan.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"

#include "svgadump/svga_shader_dump.h"

#include "svga_context.h"
#include "svga_shader.h"
#include "svga_tgsi.h"
#include "svga_tgsi_emit.h"
#include "svga_debug.h"

#include "svga_hw_reg.h"
#include "svga3d_shaderdefs.h"


/* Sinkhole used only in error conditions.
 */
static char err_buf[128];


static bool
svga_shader_expand(struct svga_shader_emitter *emit)
{
   char *new_buf;
   unsigned newsize = emit->size * 2;

   if (emit->buf != err_buf)
      new_buf = REALLOC(emit->buf, emit->size, newsize);
   else
      new_buf = NULL;

   if (!new_buf) {
      emit->ptr = err_buf;
      emit->buf = err_buf;
      emit->size = sizeof(err_buf);
      return false;
   }

   emit->size = newsize;
   emit->ptr = new_buf + (emit->ptr - emit->buf);
   emit->buf = new_buf;
   return true;
}


static inline bool
reserve(struct svga_shader_emitter *emit, unsigned nr_dwords)
{
   if (emit->ptr - emit->buf + nr_dwords * sizeof(unsigned) >= emit->size) {
      if (!svga_shader_expand(emit)) {
         return false;
      }
   }

   return true;
}


bool
svga_shader_emit_dword(struct svga_shader_emitter * emit, unsigned dword)
{
   if (!reserve(emit, 1))
      return false;

   *(unsigned *) emit->ptr = dword;
   emit->ptr += sizeof dword;
   return true;
}


bool
svga_shader_emit_dwords(struct svga_shader_emitter * emit,
                        const unsigned *dwords, unsigned nr)
{
   if (!reserve(emit, nr))
      return false;

   memcpy(emit->ptr, dwords, nr * sizeof *dwords);
   emit->ptr += nr * sizeof *dwords;
   return true;
}


bool
svga_shader_emit_opcode(struct svga_shader_emitter * emit, unsigned opcode)
{
   SVGA3dShaderInstToken *here;

   if (!reserve(emit, 1))
      return false;

   here = (SVGA3dShaderInstToken *) emit->ptr;
   here->value = opcode;

   if (emit->insn_offset) {
      SVGA3dShaderInstToken *prev =
         (SVGA3dShaderInstToken *) (emit->buf + emit->insn_offset);
      prev->size = (here - prev) - 1;
   }

   emit->insn_offset = emit->ptr - emit->buf;
   emit->ptr += sizeof(unsigned);
   return true;
}


static bool
svga_shader_emit_header(struct svga_shader_emitter *emit)
{
   SVGA3dShaderVersion header;

   memset(&header, 0, sizeof header);

   switch (emit->unit) {
   case PIPE_SHADER_FRAGMENT:
      header.value = SVGA3D_PS_30;
      break;
   case PIPE_SHADER_VERTEX:
      header.value = SVGA3D_VS_30;
      break;
   }

   return svga_shader_emit_dword(emit, header.value);
}


/**
 * Parse TGSI shader and translate to SVGA/DX9 serialized
 * representation.
 *
 * In this function SVGA shader is emitted to an in-memory buffer that
 * can be dynamically grown.  Once we've finished and know how large
 * it is, it will be copied to a hardware buffer for upload.
 */
struct svga_shader_variant *
svga_tgsi_vgpu9_translate(struct svga_context *svga,
                          const struct svga_shader *shader,
                          const struct svga_compile_key *key,
                          enum pipe_shader_type unit)
{
   struct svga_shader_variant *variant = NULL;
   struct svga_shader_emitter emit;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_TGSIVGPU9TRANSLATE);

   memset(&emit, 0, sizeof(emit));

   emit.size = 1024;
   emit.buf = MALLOC(emit.size);
   if (emit.buf == NULL) {
      goto fail;
   }

   emit.ptr = emit.buf;
   emit.unit = unit;
   emit.key = *key;

   tgsi_scan_shader(shader->tokens, &emit.info);

   emit.imm_start = emit.info.file_max[TGSI_FILE_CONSTANT] + 1;

   if (unit == PIPE_SHADER_FRAGMENT)
      emit.imm_start += key->num_unnormalized_coords;

   if (unit == PIPE_SHADER_VERTEX) {
      emit.imm_start += key->vs.need_prescale ? 2 : 0;
   }

   emit.nr_hw_float_const =
      (emit.imm_start + emit.info.file_max[TGSI_FILE_IMMEDIATE] + 1);

   emit.nr_hw_temp = emit.info.file_max[TGSI_FILE_TEMPORARY] + 1;

   if (emit.nr_hw_temp >= SVGA3D_TEMPREG_MAX) {
      debug_printf("svga: too many temporary registers (%u)\n",
                   emit.nr_hw_temp);
      goto fail;
   }

   if (emit.info.indirect_files & (1 << TGSI_FILE_TEMPORARY)) {
      debug_printf(
         "svga: indirect indexing of temporary registers is not supported.\n");
      goto fail;
   }

   emit.in_main_func = true;

   if (!svga_shader_emit_header(&emit)) {
      debug_printf("svga: emit header failed\n");
      goto fail;
   }

   if (!svga_shader_emit_instructions(&emit, shader->tokens)) {
      debug_printf("svga: emit instructions failed\n");
      goto fail;
   }

   variant = svga_new_shader_variant(svga, unit);
   if (!variant)
      goto fail;

   variant->shader = shader;
   variant->tokens = (const unsigned *) emit.buf;
   variant->nr_tokens = (emit.ptr - emit.buf) / sizeof(unsigned);
   memcpy(&variant->key, key, sizeof(*key));
   variant->id = UTIL_BITMASK_INVALID_INDEX;

   if (unit == PIPE_SHADER_FRAGMENT) {
      struct svga_fs_variant *fs_variant = svga_fs_variant(variant);

      fs_variant->pstipple_sampler_unit = emit.pstipple_sampler_unit;

      /* If there was exactly one write to a fragment shader output register
       * and it came from a constant buffer, we know all fragments will have
       * the same color (except for blending).
       */
      fs_variant->constant_color_output =
         emit.constant_color_output && emit.num_output_writes == 1;
   }

#if 0
   if (!svga_shader_verify(variant->tokens, variant->nr_tokens) ||
       SVGA_DEBUG & DEBUG_TGSI) {
      debug_printf("#####################################\n");
      debug_printf("Shader %u below\n", shader->id);
      tgsi_dump(shader->tokens, 0);
      if (SVGA_DEBUG & DEBUG_TGSI) {
         debug_printf("Shader %u compiled below\n", shader->id);
         svga_shader_dump(variant->tokens, variant->nr_tokens, false);
      }
      debug_printf("#####################################\n");
   }
#endif

   goto done;

fail:
   FREE(variant);
   if (emit.buf != err_buf)
      FREE(emit.buf);
   variant = NULL;

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return variant;
}


/**
 * Helper function to convert tgsi semantic name to vertex attribute
 * semantic name.
 */
static gl_vert_attrib
svga_tgsi_to_gl_vert_attrib_semantic(unsigned sem_name,
                                     unsigned sem_index)
{
   switch (sem_name) {
   case TGSI_SEMANTIC_POSITION:
      return VERT_ATTRIB_POS;
   case TGSI_SEMANTIC_COLOR:
      assert(sem_index <= 1);
      return VERT_ATTRIB_COLOR0;
   case TGSI_SEMANTIC_FOG:
      return VERT_ATTRIB_FOG;
   case TGSI_SEMANTIC_PSIZE:
      return VERT_ATTRIB_POINT_SIZE;
   case TGSI_SEMANTIC_GENERIC:
      return VERT_ATTRIB_GENERIC0;
   case TGSI_SEMANTIC_EDGEFLAG:
      return VERT_ATTRIB_EDGEFLAG;
   case TGSI_SEMANTIC_TEXCOORD:
      assert(sem_index <= 7);
      return VERT_ATTRIB_TEX0;
   default:
      assert(0);
      return VERT_ATTRIB_POS;
   }
}


/**
 * Helper function to convert tgsi semantic name to varying semantic name.
 */
static gl_varying_slot
svga_tgsi_to_gl_varying_semantic(unsigned sem_name,
                                 unsigned sem_index)
{
   switch (sem_name) {
   case TGSI_SEMANTIC_POSITION:
      return VARYING_SLOT_POS;
   case TGSI_SEMANTIC_COLOR:
      assert(sem_index <= 1);
      return VARYING_SLOT_COL0;
   case TGSI_SEMANTIC_BCOLOR:
      assert(sem_index <= 1);
      return VARYING_SLOT_BFC0;
   case TGSI_SEMANTIC_FOG:
      return VARYING_SLOT_FOGC;
   case TGSI_SEMANTIC_PSIZE:
      return VARYING_SLOT_PSIZ;
   case TGSI_SEMANTIC_GENERIC:
      return VARYING_SLOT_VAR0;
   case TGSI_SEMANTIC_FACE:
      return VARYING_SLOT_FACE;
   case TGSI_SEMANTIC_EDGEFLAG:
      return VARYING_SLOT_EDGE;
   case TGSI_SEMANTIC_CLIPDIST:
      assert(sem_index <= 1);
      return VARYING_SLOT_CLIP_DIST0;
   case TGSI_SEMANTIC_CLIPVERTEX:
      return VARYING_SLOT_CLIP_VERTEX;
   case TGSI_SEMANTIC_TEXCOORD:
      assert(sem_index <= 7);
      return VARYING_SLOT_TEX0;
   case TGSI_SEMANTIC_PCOORD:
      return VARYING_SLOT_PNTC;
   case TGSI_SEMANTIC_VIEWPORT_INDEX:
      return VARYING_SLOT_VIEWPORT;
   case TGSI_SEMANTIC_LAYER:
      return VARYING_SLOT_LAYER;
   case TGSI_SEMANTIC_PATCH:
      return VARYING_SLOT_PATCH0;
   case TGSI_SEMANTIC_TESSOUTER:
      return VARYING_SLOT_TESS_LEVEL_OUTER;
   case TGSI_SEMANTIC_TESSINNER:
      return VARYING_SLOT_TESS_LEVEL_INNER;
   case TGSI_SEMANTIC_VIEWPORT_MASK:
      return VARYING_SLOT_VIEWPORT_MASK;
   case TGSI_SEMANTIC_PRIMID:
      return VARYING_SLOT_PRIMITIVE_ID;
   default:
      assert(0);
      return VARYING_SLOT_POS;
   }
}


/**
 * Helper function to convert tgsi semantic name to fragment result
 * semantic name.
 */
static gl_frag_result
svga_tgsi_to_gl_frag_result_semantic(unsigned sem_name,
                                     unsigned sem_index)
{
   switch (sem_name) {
   case TGSI_SEMANTIC_POSITION:
      return FRAG_RESULT_DEPTH;
   case TGSI_SEMANTIC_COLOR:
      assert(sem_index <= 7);
      return FRAG_RESULT_DATA0;
   case TGSI_SEMANTIC_STENCIL:
      return FRAG_RESULT_STENCIL;
   case TGSI_SEMANTIC_SAMPLEMASK:
      return FRAG_RESULT_SAMPLE_MASK;
   default:
      assert(0);
      return FRAG_RESULT_DATA0;
   }
}


/**
 * svga_tgsi_scan_shader is called to collect information of the
 * specified tgsi shader.
 */
void
svga_tgsi_scan_shader(struct svga_shader *shader)
{
   struct tgsi_shader_info *tgsi_info = &shader->tgsi_info;
   struct svga_shader_info *info = &shader->info;

   tgsi_scan_shader(shader->tokens, tgsi_info);

   /* Save some common shader info in IR neutral format */
   info->num_inputs = tgsi_info->num_inputs;
   info->num_outputs = tgsi_info->num_outputs;
   info->writes_edgeflag = tgsi_info->writes_edgeflag;
   info->writes_layer = tgsi_info->writes_layer;
   info->writes_position = tgsi_info->writes_position;
   info->writes_psize = tgsi_info->writes_psize;
   info->writes_viewport_index = tgsi_info->writes_viewport_index;

   info->uses_grid_size = tgsi_info->uses_grid_size;
   info->uses_const_buffers = tgsi_info->const_buffers_declared != 0;
   info->uses_hw_atomic = tgsi_info->hw_atomic_declared != 0;
   info->uses_images = tgsi_info->images_declared != 0;
   info->uses_image_size = tgsi_info->opcode_count[TGSI_OPCODE_RESQ] ? 1 : 0;
   info->uses_shader_buffers = tgsi_info->shader_buffers_declared != 0;
   info->uses_samplers = tgsi_info->samplers_declared != 0;
   info->const_buffers_declared = tgsi_info->const_buffers_declared;
   info->shader_buffers_declared = tgsi_info->shader_buffers_declared;

   info->generic_inputs_mask = svga_get_generic_inputs_mask(tgsi_info);
   info->generic_outputs_mask = svga_get_generic_outputs_mask(tgsi_info);

   /* Convert TGSI inputs semantic.
    * Vertex shader does not have varying inputs but vertex attributes.
    */
   if (shader->stage == PIPE_SHADER_VERTEX) {
      for (unsigned i = 0; i < info->num_inputs; i++) {
         info->input_semantic_name[i] =
            svga_tgsi_to_gl_vert_attrib_semantic(
               tgsi_info->input_semantic_name[i],
               tgsi_info->input_semantic_index[i]);
         info->input_semantic_index[i] = tgsi_info->input_semantic_index[i];
      }
   }
   else {
      for (unsigned i = 0; i < info->num_inputs; i++) {
         info->input_semantic_name[i] =
            svga_tgsi_to_gl_varying_semantic(
               tgsi_info->input_semantic_name[i],
               tgsi_info->input_semantic_index[i]);
         info->input_semantic_index[i] = tgsi_info->input_semantic_index[i];
      }
   }

   /* Convert TGSI outputs semantic.
    * Fragment shader does not have varying outputs but fragment results.
    */
   if (shader->stage == PIPE_SHADER_FRAGMENT) {
      for (unsigned i = 0; i < info->num_outputs; i++) {
         info->output_semantic_name[i] =
            svga_tgsi_to_gl_frag_result_semantic(
               tgsi_info->output_semantic_name[i],
               tgsi_info->output_semantic_index[i]);
         info->output_semantic_index[i] = tgsi_info->output_semantic_index[i];
      }
   }
   else {
      for (unsigned i = 0; i < info->num_outputs; i++) {
         info->output_semantic_name[i] =
            svga_tgsi_to_gl_varying_semantic(
               tgsi_info->output_semantic_name[i],
               tgsi_info->output_semantic_index[i]);
         info->output_semantic_index[i] = tgsi_info->output_semantic_index[i];
      }
   }

   info->constbuf0_num_uniforms = tgsi_info->const_file_max[0] + 1;

   switch (tgsi_info->processor) {
   case PIPE_SHADER_FRAGMENT:
      info->fs.color0_writes_all_cbufs =
         tgsi_info->properties[TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS];
      break;
   case PIPE_SHADER_GEOMETRY:
      info->gs.out_prim = tgsi_info->properties[TGSI_PROPERTY_GS_OUTPUT_PRIM];
      info->gs.in_prim = tgsi_info->properties[TGSI_PROPERTY_GS_INPUT_PRIM];
      break;
   case PIPE_SHADER_TESS_CTRL:
      info->tcs.vertices_out =
         tgsi_info->properties[TGSI_PROPERTY_TCS_VERTICES_OUT];

      for (unsigned i = 0; i < info->num_outputs; i++) {
         switch (tgsi_info->output_semantic_name[i]) {
         case TGSI_SEMANTIC_TESSOUTER:
         case TGSI_SEMANTIC_TESSINNER:
            info->tcs.writes_tess_factor = true;
            break;
         default:
            break;
         }
      }
      break;
   case PIPE_SHADER_TESS_EVAL:
      info->tes.prim_mode =
         tgsi_info->properties[TGSI_PROPERTY_TES_PRIM_MODE];

      for (unsigned i = 0; i < info->num_inputs; i++) {
         switch (tgsi_info->input_semantic_name[i]) {
         case TGSI_SEMANTIC_PATCH:
         case TGSI_SEMANTIC_TESSOUTER:
         case TGSI_SEMANTIC_TESSINNER:
            break;
         default:
              info->tes.reads_control_point = true;
         }
      }
      break;
   default:
      break;
   }
}


/**
 * Compile a TGSI shader
 */
struct svga_shader_variant *
svga_tgsi_compile_shader(struct svga_context *svga,
                         struct svga_shader *shader,
                         const struct svga_compile_key *key)
{
   if (svga_have_vgpu10(svga)) {
      return svga_tgsi_vgpu10_translate(svga, shader, key, shader->stage);
   }
   else {
      return svga_tgsi_vgpu9_translate(svga, shader, key, shader->stage);
   }
}
