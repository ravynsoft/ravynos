/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_compiler.h"
#include "d3d12_context.h"
#include "d3d12_debug.h"
#include "d3d12_screen.h"
#include "d3d12_nir_passes.h"
#include "nir_to_dxil.h"
#include "dxil_nir.h"
#include "dxil_nir_lower_int_cubemaps.h"

#include "pipe/p_state.h"

#include "nir.h"
#include "nir/nir_draw_helpers.h"
#include "nir/tgsi_to_nir.h"
#include "compiler/nir/nir_builder.h"

#include "util/hash_table.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_simple_shaders.h"
#include "util/u_dl.h"

#include <dxguids/dxguids.h>

#ifdef _WIN32
#include "dxil_validator.h"
#endif

const void *
d3d12_get_compiler_options(struct pipe_screen *screen,
                           enum pipe_shader_ir ir,
                           enum pipe_shader_type shader)
{
   assert(ir == PIPE_SHADER_IR_NIR);
   return &d3d12_screen(screen)->nir_options;
}

static uint32_t
resource_dimension(enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
      return RESOURCE_DIMENSION_TEXTURE1D;
   case GLSL_SAMPLER_DIM_2D:
      return RESOURCE_DIMENSION_TEXTURE2D;
   case GLSL_SAMPLER_DIM_3D:
      return RESOURCE_DIMENSION_TEXTURE3D;
   case GLSL_SAMPLER_DIM_CUBE:
      return RESOURCE_DIMENSION_TEXTURECUBE;
   default:
      return RESOURCE_DIMENSION_UNKNOWN;
   }
}

static bool
can_remove_dead_sampler(nir_variable *var, void *data)
{
   const struct glsl_type *base_type = glsl_without_array(var->type);
   return glsl_type_is_sampler(base_type) && !glsl_type_is_bare_sampler(base_type);
}

static struct d3d12_shader *
compile_nir(struct d3d12_context *ctx, struct d3d12_shader_selector *sel,
            struct d3d12_shader_key *key, struct nir_shader *nir)
{
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_shader *shader = rzalloc(sel, d3d12_shader);
   shader->key = *key;

   if (shader->key.n_texture_states > 0) {
      shader->key.tex_wrap_states = (dxil_wrap_sampler_state*)ralloc_size(sel, sizeof(dxil_wrap_sampler_state) * shader->key.n_texture_states);
      memcpy(shader->key.tex_wrap_states, key->tex_wrap_states, sizeof(dxil_wrap_sampler_state) * shader->key.n_texture_states);
   }
   else
      shader->key.tex_wrap_states = nullptr;

   shader->output_vars_fs = nullptr;
   shader->output_vars_gs = nullptr;
   shader->output_vars_default = nullptr;

   shader->input_vars_vs = nullptr;
   shader->input_vars_default = nullptr;

   shader->tess_eval_output_vars = nullptr;
   shader->tess_ctrl_input_vars = nullptr;
   shader->nir = nir;
   sel->current = shader;

   NIR_PASS_V(nir, nir_lower_samplers);
   NIR_PASS_V(nir, dxil_nir_split_typed_samplers);

   NIR_PASS_V(nir, nir_opt_dce);
   struct nir_remove_dead_variables_options dead_var_opts = {};
   dead_var_opts.can_remove_var = can_remove_dead_sampler;
   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_uniform, &dead_var_opts);

   if (key->samples_int_textures)
      NIR_PASS_V(nir, dxil_lower_sample_to_txf_for_integer_tex,
                 key->n_texture_states, key->tex_wrap_states, key->swizzle_state,
                 screen->base.get_paramf(&screen->base, PIPE_CAPF_MAX_TEXTURE_LOD_BIAS));

   if (key->stage == PIPE_SHADER_VERTEX && key->vs.needs_format_emulation)
      dxil_nir_lower_vs_vertex_conversion(nir, key->vs.format_conversion);

   uint32_t num_ubos_before_lower_to_ubo = nir->info.num_ubos;
   uint32_t num_uniforms_before_lower_to_ubo = nir->num_uniforms;
   NIR_PASS_V(nir, nir_lower_uniforms_to_ubo, false, false);
   shader->has_default_ubo0 = num_uniforms_before_lower_to_ubo > 0 &&
                              nir->info.num_ubos > num_ubos_before_lower_to_ubo;

   if (key->last_vertex_processing_stage) {
      if (key->invert_depth)
         NIR_PASS_V(nir, d3d12_nir_invert_depth, key->invert_depth, key->halfz);
      if (!key->halfz)
         NIR_PASS_V(nir, nir_lower_clip_halfz);
      NIR_PASS_V(nir, d3d12_lower_yflip);
   }
   NIR_PASS_V(nir, d3d12_lower_load_draw_params);
   NIR_PASS_V(nir, d3d12_lower_load_patch_vertices_in);
   NIR_PASS_V(nir, d3d12_lower_state_vars, shader);
   const struct dxil_nir_lower_loads_stores_options loads_stores_options = {};
   NIR_PASS_V(nir, dxil_nir_lower_loads_stores_to_dxil, &loads_stores_options);
   NIR_PASS_V(nir, dxil_nir_lower_double_math);

   if (key->stage == PIPE_SHADER_FRAGMENT && key->fs.multisample_disabled)
      NIR_PASS_V(nir, d3d12_disable_multisampling);

   struct nir_to_dxil_options opts = {};
   opts.interpolate_at_vertex = screen->have_load_at_vertex;
   opts.lower_int16 = !screen->opts4.Native16BitShaderOpsSupported;
   opts.no_ubo0 = !shader->has_default_ubo0;
   opts.last_ubo_is_not_arrayed = shader->num_state_vars > 0;
   if (key->stage == PIPE_SHADER_FRAGMENT)
      opts.provoking_vertex = key->fs.provoking_vertex;
   opts.input_clip_size = key->input_clip_size;
   opts.environment = DXIL_ENVIRONMENT_GL;
   opts.shader_model_max = screen->max_shader_model;
#ifdef _WIN32
   opts.validator_version_max = dxil_get_validator_version(ctx->dxil_validator);
#endif

   struct blob tmp;
   if (!nir_to_dxil(nir, &opts, NULL, &tmp)) {
      debug_printf("D3D12: nir_to_dxil failed\n");
      return NULL;
   }

   // Non-ubo variables
   shader->begin_srv_binding = (UINT_MAX);
   nir_foreach_variable_with_modes(var, nir, nir_var_uniform) {
      auto type_no_array = glsl_without_array(var->type);
      if (glsl_type_is_texture(type_no_array)) {
         unsigned count = glsl_type_is_array(var->type) ? glsl_get_aoa_size(var->type) : 1;
         for (unsigned i = 0; i < count; ++i) {
            shader->srv_bindings[var->data.binding + i].dimension = resource_dimension(glsl_get_sampler_dim(type_no_array));
         }
         shader->begin_srv_binding = MIN2(var->data.binding, shader->begin_srv_binding);
         shader->end_srv_binding = MAX2(var->data.binding + count, shader->end_srv_binding);
      }
   }

   nir_foreach_image_variable(var, nir) {
      auto type_no_array = glsl_without_array(var->type);
      unsigned count = glsl_type_is_array(var->type) ? glsl_get_aoa_size(var->type) : 1;
      for (unsigned i = 0; i < count; ++i) {
         shader->uav_bindings[var->data.driver_location + i].dimension = resource_dimension(glsl_get_sampler_dim(type_no_array));
      }
   }

   // Ubo variables
   if(nir->info.num_ubos) {
      // Ignore state_vars ubo as it is bound as root constants
      unsigned num_ubo_bindings = nir->info.num_ubos - (shader->state_vars_used ? 1 : 0);
      for(unsigned i = shader->has_default_ubo0 ? 0 : 1; i < num_ubo_bindings; ++i) {
         shader->cb_bindings[shader->num_cb_bindings++].binding = i;
      }
   }

#ifdef _WIN32
   if (ctx->dxil_validator) {
      if (!(d3d12_debug & D3D12_DEBUG_EXPERIMENTAL)) {
         char *err;
         if (!dxil_validate_module(ctx->dxil_validator, tmp.data,
                                   tmp.size, &err) && err) {
            debug_printf(
               "== VALIDATION ERROR =============================================\n"
               "%s\n"
               "== END ==========================================================\n",
               err);
            ralloc_free(err);
         }
      }

      if (d3d12_debug & D3D12_DEBUG_DISASS) {
         char *str = dxil_disasm_module(ctx->dxil_validator, tmp.data,
                                        tmp.size);
         fprintf(stderr,
                 "== BEGIN SHADER ============================================\n"
                 "%s\n"
                 "== END SHADER ==============================================\n",
               str);
         ralloc_free(str);
      }
   }
#endif

   blob_finish_get_buffer(&tmp, &shader->bytecode, &shader->bytecode_length);

   if (d3d12_debug & D3D12_DEBUG_DXIL) {
      char buf[256];
      static int i;
      snprintf(buf, sizeof(buf), "dump%02d.dxil", i++);
      FILE *fp = fopen(buf, "wb");
      fwrite(shader->bytecode, sizeof(char), shader->bytecode_length, fp);
      fclose(fp);
      fprintf(stderr, "wrote '%s'...\n", buf);
   }
   return shader;
}

struct d3d12_selection_context {
   struct d3d12_context *ctx;
   bool needs_point_sprite_lowering;
   bool needs_vertex_reordering;
   unsigned provoking_vertex;
   bool alternate_tri;
   unsigned fill_mode_lowered;
   unsigned cull_mode_lowered;
   bool manual_depth_range;
   unsigned missing_dual_src_outputs;
   unsigned frag_result_color_lowering;
   const unsigned *variable_workgroup_size;
};

unsigned
missing_dual_src_outputs(struct d3d12_context *ctx)
{
   if (!ctx->gfx_pipeline_state.blend || !ctx->gfx_pipeline_state.blend->is_dual_src)
      return 0;

   struct d3d12_shader_selector *fs = ctx->gfx_stages[PIPE_SHADER_FRAGMENT];
   if (!fs)
      return 0;

   const nir_shader *s = fs->initial;

   unsigned indices_seen = 0;
   nir_foreach_function_impl(impl, s) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_store_deref)
               continue;

            nir_variable *var = nir_intrinsic_get_var(intr, 0);
            if (var->data.mode != nir_var_shader_out)
               continue;

            unsigned index = var->data.index;
            if (var->data.location > FRAG_RESULT_DATA0)
               index = var->data.location - FRAG_RESULT_DATA0;
            else if (var->data.location != FRAG_RESULT_COLOR &&
                     var->data.location != FRAG_RESULT_DATA0)
               continue;

            indices_seen |= 1u << index;
            if ((indices_seen & 3) == 3)
               return 0;
         }
      }
   }

   return 3 & ~indices_seen;
}

static unsigned
frag_result_color_lowering(struct d3d12_context *ctx)
{
   struct d3d12_shader_selector *fs = ctx->gfx_stages[PIPE_SHADER_FRAGMENT];
   assert(fs);

   if (fs->initial->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_COLOR))
      return ctx->fb.nr_cbufs > 1 ? ctx->fb.nr_cbufs : 0;

   return 0;
}

bool
manual_depth_range(struct d3d12_context *ctx)
{
   if (!d3d12_need_zero_one_depth_range(ctx))
      return false;

   /**
    * If we can't use the D3D12 zero-one depth-range, we might have to apply
    * depth-range ourselves.
    *
    * Because we only need to override the depth-range to zero-one range in
    * the case where we write frag-depth, we only need to apply manual
    * depth-range to gl_FragCoord.z.
    *
    * No extra care is needed to be taken in the case where gl_FragDepth is
    * written conditionally, because the GLSL 4.60 spec states:
    *
    *    If a shader statically assigns a value to gl_FragDepth, and there
    *    is an execution path through the shader that does not set
    *    gl_FragDepth, then the value of the fragment’s depth may be
    *    undefined for executions of the shader that take that path. That
    *    is, if the set of linked fragment shaders statically contain a
    *    write to gl_FragDepth, then it is responsible for always writing
    *    it.
    */

   struct d3d12_shader_selector *fs = ctx->gfx_stages[PIPE_SHADER_FRAGMENT];
   return fs && fs->initial->info.inputs_read & VARYING_BIT_POS;
}

static bool
needs_edge_flag_fix(enum mesa_prim mode)
{
   return (mode == MESA_PRIM_QUADS ||
           mode == MESA_PRIM_QUAD_STRIP ||
           mode == MESA_PRIM_POLYGON);
}

static unsigned
fill_mode_lowered(struct d3d12_context *ctx, const struct pipe_draw_info *dinfo)
{
   struct d3d12_shader_selector *vs = ctx->gfx_stages[PIPE_SHADER_VERTEX];

   if ((ctx->gfx_stages[PIPE_SHADER_GEOMETRY] != NULL &&
        !ctx->gfx_stages[PIPE_SHADER_GEOMETRY]->is_variant) ||
       ctx->gfx_pipeline_state.rast == NULL ||
       (dinfo->mode != MESA_PRIM_TRIANGLES &&
        dinfo->mode != MESA_PRIM_TRIANGLE_STRIP))
      return PIPE_POLYGON_MODE_FILL;

   /* D3D12 supports line mode (wireframe) but doesn't support edge flags */
   if (((ctx->gfx_pipeline_state.rast->base.fill_front == PIPE_POLYGON_MODE_LINE &&
         ctx->gfx_pipeline_state.rast->base.cull_face != PIPE_FACE_FRONT) ||
        (ctx->gfx_pipeline_state.rast->base.fill_back == PIPE_POLYGON_MODE_LINE &&
         ctx->gfx_pipeline_state.rast->base.cull_face == PIPE_FACE_FRONT)) &&
       (vs->initial->info.outputs_written & VARYING_BIT_EDGE ||
        needs_edge_flag_fix(ctx->initial_api_prim)))
      return PIPE_POLYGON_MODE_LINE;

   if (ctx->gfx_pipeline_state.rast->base.fill_front == PIPE_POLYGON_MODE_POINT)
      return PIPE_POLYGON_MODE_POINT;

   return PIPE_POLYGON_MODE_FILL;
}

static bool
has_stream_out_for_streams(struct d3d12_context *ctx)
{
   unsigned mask = ctx->gfx_stages[PIPE_SHADER_GEOMETRY]->initial->info.gs.active_stream_mask & ~1;
   for (unsigned i = 0; i < ctx->gfx_pipeline_state.so_info.num_outputs; ++i) {
      unsigned stream = ctx->gfx_pipeline_state.so_info.output[i].stream;
      if (((1 << stream) & mask) &&
         ctx->so_buffer_views[stream].SizeInBytes)
         return true;
   }
   return false;
}

static bool
needs_point_sprite_lowering(struct d3d12_context *ctx, const struct pipe_draw_info *dinfo)
{
   struct d3d12_shader_selector *vs = ctx->gfx_stages[PIPE_SHADER_VERTEX];
   struct d3d12_shader_selector *gs = ctx->gfx_stages[PIPE_SHADER_GEOMETRY];

   if (gs != NULL && !gs->is_variant) {
      /* There is an user GS; Check if it outputs points with PSIZE */
      return (gs->initial->info.gs.output_primitive == MESA_PRIM_POINTS &&
              (gs->initial->info.outputs_written & VARYING_BIT_PSIZ ||
                 ctx->gfx_pipeline_state.rast->base.point_size > 1.0) &&
              (gs->initial->info.gs.active_stream_mask == 1 ||
                 !has_stream_out_for_streams(ctx)));
   } else {
      /* No user GS; check if we are drawing wide points */
      return ((dinfo->mode == MESA_PRIM_POINTS ||
               fill_mode_lowered(ctx, dinfo) == PIPE_POLYGON_MODE_POINT) &&
              (ctx->gfx_pipeline_state.rast->base.point_size > 1.0 ||
               ctx->gfx_pipeline_state.rast->base.offset_point ||
               (ctx->gfx_pipeline_state.rast->base.point_size_per_vertex &&
                vs->initial->info.outputs_written & VARYING_BIT_PSIZ)) &&
              (vs->initial->info.outputs_written & VARYING_BIT_POS));
   }
}

static unsigned
cull_mode_lowered(struct d3d12_context *ctx, unsigned fill_mode)
{
   if ((ctx->gfx_stages[PIPE_SHADER_GEOMETRY] != NULL &&
        !ctx->gfx_stages[PIPE_SHADER_GEOMETRY]->is_variant) ||
       ctx->gfx_pipeline_state.rast == NULL ||
       ctx->gfx_pipeline_state.rast->base.cull_face == PIPE_FACE_NONE)
      return PIPE_FACE_NONE;

   return ctx->gfx_pipeline_state.rast->base.cull_face;
}

static unsigned
get_provoking_vertex(struct d3d12_selection_context *sel_ctx, bool *alternate, const struct pipe_draw_info *dinfo)
{
   if (dinfo->mode == GL_PATCHES) {
      *alternate = false;
      return 0;
   }

   struct d3d12_shader_selector *vs = sel_ctx->ctx->gfx_stages[PIPE_SHADER_VERTEX];
   struct d3d12_shader_selector *gs = sel_ctx->ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
   struct d3d12_shader_selector *last_vertex_stage = gs && !gs->is_variant ? gs : vs;

   enum mesa_prim mode;
   switch (last_vertex_stage->stage) {
   case PIPE_SHADER_GEOMETRY:
      mode = (enum mesa_prim)last_vertex_stage->current->nir->info.gs.output_primitive;
      break;
   case PIPE_SHADER_VERTEX:
      mode = (enum mesa_prim)dinfo->mode;
      break;
   default:
      unreachable("Tesselation shaders are not supported");
   }

   bool flatshade_first = sel_ctx->ctx->gfx_pipeline_state.rast &&
                          sel_ctx->ctx->gfx_pipeline_state.rast->base.flatshade_first;
   *alternate = (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_STRIP_ADJACENCY) &&
                (!gs || gs->is_variant ||
                 gs->initial->info.gs.vertices_out > u_prim_vertex_count(mode)->min);
   return flatshade_first ? 0 : u_prim_vertex_count(mode)->min - 1;
}

bool
has_flat_varyings(struct d3d12_context *ctx)
{
   struct d3d12_shader_selector *fs = ctx->gfx_stages[PIPE_SHADER_FRAGMENT];

   if (!fs || !fs->current)
      return false;

   nir_foreach_variable_with_modes(input, fs->current->nir,
                                   nir_var_shader_in) {
      if (input->data.interpolation == INTERP_MODE_FLAT &&
          /* Disregard sysvals */
          (input->data.location >= VARYING_SLOT_VAR0 ||
             input->data.location <= VARYING_SLOT_TEX7))
         return true;
   }

   return false;
}

static bool
needs_vertex_reordering(struct d3d12_selection_context *sel_ctx, const struct pipe_draw_info *dinfo)
{
   struct d3d12_context *ctx = sel_ctx->ctx;
   bool flat = ctx->has_flat_varyings;
   bool xfb = ctx->gfx_pipeline_state.num_so_targets > 0;

   if (fill_mode_lowered(ctx, dinfo) != PIPE_POLYGON_MODE_FILL)
      return false;

   /* TODO add support for line primitives */

   /* When flat shading a triangle and provoking vertex is not the first one, we use load_at_vertex.
      If not available for this adapter, or if it's a triangle strip, we need to reorder the vertices */
   if (flat && sel_ctx->provoking_vertex >= 2 && (!d3d12_screen(ctx->base.screen)->have_load_at_vertex ||
                                                  sel_ctx->alternate_tri))
      return true;

   /* When transform feedback is enabled and the output is alternating (triangle strip or triangle
      strip with adjacency), we need to reorder vertices to get the order expected by OpenGL. This
      only works when there is no flat shading involved. In that scenario, we don't care about
      the provoking vertex. */
   if (xfb && !flat && sel_ctx->alternate_tri) {
      sel_ctx->provoking_vertex = 0;
      return true;
   }

   return false;
}

static nir_variable *
create_varying_from_info(nir_shader *nir, const struct d3d12_varying_info *info,
                         unsigned slot, unsigned slot_frac, nir_variable_mode mode, bool patch)
{
   nir_variable *var;
   char tmp[100];

   snprintf(tmp, ARRAY_SIZE(tmp),
            mode == nir_var_shader_in ? "in_%d" : "out_%d",
            info->slots[slot].vars[slot_frac].driver_location);
   var = nir_variable_create(nir, mode, info->slots[slot].types[slot_frac], tmp);
   var->data.location = slot;
   var->data.location_frac = slot_frac;
   var->data.driver_location = info->slots[slot].vars[slot_frac].driver_location;
   var->data.interpolation = info->slots[slot].vars[slot_frac].interpolation;
   var->data.patch = info->slots[slot].patch;
   var->data.compact = info->slots[slot].vars[slot_frac].compact;
   if (patch)
      var->data.location += VARYING_SLOT_PATCH0;

   if (mode == nir_var_shader_out)
      NIR_PASS_V(nir, d3d12_write_0_to_new_varying, var);

   return var;
}

void
create_varyings_from_info(nir_shader *nir, const struct d3d12_varying_info *info,
                          unsigned slot, nir_variable_mode mode, bool patch)
{
   unsigned mask = info->slots[slot].location_frac_mask;
   while (mask)
      create_varying_from_info(nir, info, slot, u_bit_scan(&mask), mode, patch);
}

static d3d12_varying_info*
fill_varyings(struct d3d12_context *ctx, const nir_shader *s,
              nir_variable_mode modes, uint64_t mask, bool patch)
{
   struct d3d12_varying_info info;

   info.max = 0;
   info.mask = 0;
   info.hash = 0;

   nir_foreach_variable_with_modes(var, s, modes) {
      unsigned slot = var->data.location;
      bool is_generic_patch = slot >= VARYING_SLOT_PATCH0;
      if (patch ^ is_generic_patch)
         continue;
      if (is_generic_patch)
         slot -= VARYING_SLOT_PATCH0;
      uint64_t slot_bit = BITFIELD64_BIT(slot);

      if (!(mask & slot_bit))
         continue;

      if ((info.mask & slot_bit) == 0) {
         memset(info.slots + slot, 0, sizeof(info.slots[0]));
         info.max = MAX2(info.max, slot);
      }

      const struct glsl_type *type = var->type;
      if ((s->info.stage == MESA_SHADER_GEOMETRY ||
           s->info.stage == MESA_SHADER_TESS_CTRL) &&
          (modes & nir_var_shader_in) &&
          glsl_type_is_array(type))
         type = glsl_get_array_element(type);
      info.slots[slot].types[var->data.location_frac] = type;

      info.slots[slot].patch = var->data.patch;
      auto& var_slot = info.slots[slot].vars[var->data.location_frac];
      var_slot.driver_location = var->data.driver_location;
      var_slot.interpolation = var->data.interpolation;
      var_slot.compact = var->data.compact;
      info.mask |= slot_bit;
      info.slots[slot].location_frac_mask |= (1 << var->data.location_frac);
   }

   for (uint32_t i = 0; i <= info.max; ++i) {
      if (((1llu << i) & info.mask) == 0)
         memset(info.slots + i, 0, sizeof(info.slots[0]));
      else
         info.hash = _mesa_hash_data_with_seed(info.slots + i, sizeof(info.slots[0]), info.hash);
   }
   info.hash = _mesa_hash_data_with_seed(&info.mask, sizeof(info.mask), info.hash);
   
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);

   mtx_lock(&screen->varying_info_mutex);
   set_entry *pentry = _mesa_set_search_pre_hashed(screen->varying_info_set, info.hash, &info);
   if (pentry != nullptr) {
      mtx_unlock(&screen->varying_info_mutex);
      return (d3d12_varying_info*)pentry->key;
   }
   else {
      d3d12_varying_info *key = MALLOC_STRUCT(d3d12_varying_info);
      *key = info;

      _mesa_set_add_pre_hashed(screen->varying_info_set, info.hash, key);

      mtx_unlock(&screen->varying_info_mutex);
      return key;
   }
}

static void
fill_flat_varyings(struct d3d12_gs_variant_key *key, d3d12_shader_selector *fs)
{
   if (!fs || !fs->current)
      return;

   nir_foreach_variable_with_modes(input, fs->current->nir,
                                   nir_var_shader_in) {
      if (input->data.interpolation == INTERP_MODE_FLAT)
         key->flat_varyings |= BITFIELD64_BIT(input->data.location);
   }
}

bool
d3d12_compare_varying_info(const d3d12_varying_info *expect, const d3d12_varying_info *have)
{
   if (expect == have)
      return true;

   if (expect == nullptr || have == nullptr)
      return false;

   if (expect->mask != have->mask
      || expect->max != have->max)
      return false;

   if (!expect->mask)
      return true;

   /* 6 is a rough (wild) guess for a bulk memcmp cross-over point.  When there
    * are a small number of slots present, individual   is much faster. */
   if (util_bitcount64(expect->mask) < 6) {
      uint64_t mask = expect->mask;
      while (mask) {
         int slot = u_bit_scan64(&mask);
         if (memcmp(&expect->slots[slot], &have->slots[slot], sizeof(have->slots[slot])))
            return false;
      }

      return true;
   }

   return !memcmp(expect->slots, have->slots, sizeof(expect->slots[0]) * expect->max);
}


uint32_t varying_info_hash(const void *info) {
   return ((d3d12_varying_info*)info)->hash;
}
bool varying_info_compare(const void *a, const void *b) {
   return d3d12_compare_varying_info((d3d12_varying_info*)a, (d3d12_varying_info*)b);
}
void varying_info_entry_destroy(set_entry *entry) {
   if (entry->key)
      free((void*)entry->key);
}

void
d3d12_varying_cache_init(struct d3d12_screen *screen) {
   screen->varying_info_set = _mesa_set_create(nullptr, varying_info_hash, varying_info_compare);
}

void
d3d12_varying_cache_destroy(struct d3d12_screen *screen) {
   _mesa_set_destroy(screen->varying_info_set, varying_info_entry_destroy);
}


static void
validate_geometry_shader_variant(struct d3d12_selection_context *sel_ctx)
{
   struct d3d12_context *ctx = sel_ctx->ctx;
   d3d12_shader_selector *gs = ctx->gfx_stages[PIPE_SHADER_GEOMETRY];

   /* Nothing to do if there is a user geometry shader bound */
   if (gs != NULL && !gs->is_variant)
      return;

   d3d12_shader_selector* vs = ctx->gfx_stages[PIPE_SHADER_VERTEX];
   d3d12_shader_selector* fs = ctx->gfx_stages[PIPE_SHADER_FRAGMENT];

   struct d3d12_gs_variant_key key;
   key.all = 0;
   key.flat_varyings = 0;

   /* Fill the geometry shader variant key */
   if (sel_ctx->fill_mode_lowered != PIPE_POLYGON_MODE_FILL) {
      key.fill_mode = sel_ctx->fill_mode_lowered;
      key.cull_mode = sel_ctx->cull_mode_lowered;
      key.has_front_face = BITSET_TEST(fs->initial->info.system_values_read, SYSTEM_VALUE_FRONT_FACE);
      if (key.cull_mode != PIPE_FACE_NONE || key.has_front_face)
         key.front_ccw = ctx->gfx_pipeline_state.rast->base.front_ccw ^ (ctx->flip_y < 0);
      key.edge_flag_fix = needs_edge_flag_fix(ctx->initial_api_prim);
      fill_flat_varyings(&key, fs);
      if (key.flat_varyings != 0)
         key.flatshade_first = ctx->gfx_pipeline_state.rast->base.flatshade_first;
   } else if (sel_ctx->needs_point_sprite_lowering) {
      key.passthrough = true;
   } else if (sel_ctx->needs_vertex_reordering) {
      /* TODO support cases where flat shading (pv != 0) and xfb are enabled */
      key.provoking_vertex = sel_ctx->provoking_vertex;
      key.alternate_tri = sel_ctx->alternate_tri;
   }

   if (vs->initial_output_vars == nullptr) {
      vs->initial_output_vars = fill_varyings(sel_ctx->ctx, vs->initial, nir_var_shader_out,
                                                vs->initial->info.outputs_written, false);
   }
   key.varyings = vs->initial_output_vars;
   gs = d3d12_get_gs_variant(ctx, &key);
   ctx->gfx_stages[PIPE_SHADER_GEOMETRY] = gs;
}

static void
validate_tess_ctrl_shader_variant(struct d3d12_selection_context *sel_ctx)
{
   struct d3d12_context *ctx = sel_ctx->ctx;
   d3d12_shader_selector *tcs = ctx->gfx_stages[PIPE_SHADER_TESS_CTRL];

   /* Nothing to do if there is a user tess ctrl shader bound */
   if (tcs != NULL && !tcs->is_variant)
      return;

   d3d12_shader_selector *vs = ctx->gfx_stages[PIPE_SHADER_VERTEX];
   d3d12_shader_selector *tes = ctx->gfx_stages[PIPE_SHADER_TESS_EVAL];
   struct d3d12_tcs_variant_key key = {0};

   bool variant_needed = tes != nullptr;

   /* Fill the variant key */
   if (variant_needed) {
      if (vs->initial_output_vars == nullptr) {
         vs->initial_output_vars = fill_varyings(sel_ctx->ctx, vs->initial, nir_var_shader_out,
                                                 vs->initial->info.outputs_written, false);
      }
      key.varyings = vs->initial_output_vars;
      key.vertices_out = ctx->patch_vertices;
   }

   /* Find/create the proper variant and bind it */
   tcs = variant_needed ? d3d12_get_tcs_variant(ctx, &key) : NULL;
   ctx->gfx_stages[PIPE_SHADER_TESS_CTRL] = tcs;
}

static bool
d3d12_compare_shader_keys(struct d3d12_selection_context* sel_ctx, const d3d12_shader_key *expect, const d3d12_shader_key *have)
{
   assert(expect->stage == have->stage);
   assert(expect);
   assert(have);

   if (expect->hash != have->hash)
      return false;

   switch (expect->stage) {
   case PIPE_SHADER_VERTEX:
      if (expect->vs.needs_format_emulation != have->vs.needs_format_emulation)
         return false;

      if (expect->vs.needs_format_emulation) {
         if (memcmp(expect->vs.format_conversion, have->vs.format_conversion,
            sel_ctx->ctx->gfx_pipeline_state.ves->num_elements * sizeof(enum pipe_format)))
            return false;
      }
      break;
   case PIPE_SHADER_GEOMETRY:
      if (expect->gs.all != have->gs.all)
         return false;
      break;
   case PIPE_SHADER_TESS_CTRL:
      if (expect->hs.all != have->hs.all ||
          expect->hs.required_patch_outputs != have->hs.required_patch_outputs)
         return false;
      break;
   case PIPE_SHADER_TESS_EVAL:
      if (expect->ds.tcs_vertices_out != have->ds.tcs_vertices_out ||
          expect->ds.prev_patch_outputs != have->ds.prev_patch_outputs ||
          expect->ds.required_patch_inputs != have->ds.required_patch_inputs)
         return false;
      break;
   case PIPE_SHADER_FRAGMENT:
      if (expect->fs.all != have->fs.all)
         return false;
      break;
   case PIPE_SHADER_COMPUTE:
      if (memcmp(expect->cs.workgroup_size, have->cs.workgroup_size,
                 sizeof(have->cs.workgroup_size)))
         return false;
      break;
   default:
      unreachable("invalid stage");
   }
   
   if (expect->n_texture_states != have->n_texture_states)
      return false;

   if (expect->n_images != have->n_images)
      return false;

   if (expect->n_texture_states > 0 && 
       memcmp(expect->tex_wrap_states, have->tex_wrap_states,
              expect->n_texture_states * sizeof(dxil_wrap_sampler_state)))
      return false;

   if (memcmp(expect->swizzle_state, have->swizzle_state,
              expect->n_texture_states * sizeof(dxil_texture_swizzle_state)))
      return false;

   if (memcmp(expect->sampler_compare_funcs, have->sampler_compare_funcs,
              expect->n_texture_states * sizeof(enum compare_func)))
      return false;

   if (memcmp(expect->image_format_conversion, have->image_format_conversion,
      expect->n_images * sizeof(struct d3d12_image_format_conversion_info)))
      return false;
   
   return
      expect->required_varying_inputs == have->required_varying_inputs &&
      expect->required_varying_outputs == have->required_varying_outputs &&
      expect->next_varying_inputs == have->next_varying_inputs &&
      expect->prev_varying_outputs == have->prev_varying_outputs &&
      expect->common_all == have->common_all &&
      expect->tex_saturate_s == have->tex_saturate_s &&
      expect->tex_saturate_r == have->tex_saturate_r &&
      expect->tex_saturate_t == have->tex_saturate_t;
}

static uint32_t
d3d12_shader_key_hash(const d3d12_shader_key *key)
{
   uint32_t hash;

   hash = (uint32_t)key->stage;
   hash += ((uint64_t)key->required_varying_inputs) +
            (((uint64_t)key->required_varying_inputs) >> 32);
   hash += ((uint64_t)key->required_varying_outputs) +
            (((uint64_t)key->required_varying_outputs) >> 32);

   hash += key->next_varying_inputs;
   hash += key->prev_varying_outputs;
   switch (key->stage) {
   case PIPE_SHADER_VERTEX:
      /* (Probably) not worth the bit extraction for needs_format_emulation and
       * the rest of the the format_conversion data is large.  Don't bother
       * hashing for now until this is shown to be worthwhile. */
       break;
   case PIPE_SHADER_GEOMETRY:
      hash += key->gs.all;
      break;
   case PIPE_SHADER_FRAGMENT:
      hash += key->fs.all;
      break;
   case PIPE_SHADER_COMPUTE:
      hash = _mesa_hash_data_with_seed(&key->cs, sizeof(key->cs), hash);
      break;
   case PIPE_SHADER_TESS_CTRL:
      hash += key->hs.all;
      hash += ((uint64_t)key->hs.required_patch_outputs) +
               (((uint64_t)key->hs.required_patch_outputs) >> 32);
      break;
   case PIPE_SHADER_TESS_EVAL:
      hash += key->ds.tcs_vertices_out;
      hash += key->ds.prev_patch_outputs;
      hash += ((uint64_t)key->ds.required_patch_inputs) +
               (((uint64_t)key->ds.required_patch_inputs) >> 32);
      break;
   default:
      /* No type specific information to hash for other stages. */
      break;
   }

   hash += key->n_texture_states;
   hash += key->n_images;
   return hash;
}

static void
d3d12_fill_shader_key(struct d3d12_selection_context *sel_ctx,
                      d3d12_shader_key *key, d3d12_shader_selector *sel,
                      d3d12_shader_selector *prev, d3d12_shader_selector *next)
{
   pipe_shader_type stage = sel->stage;

   uint64_t system_generated_in_values =
         VARYING_BIT_PNTC |
         VARYING_BIT_PRIMITIVE_ID;

   uint64_t system_out_values =
         VARYING_BIT_CLIP_DIST0 |
         VARYING_BIT_CLIP_DIST1;

   memset(key, 0, offsetof(d3d12_shader_key, vs));
   key->stage = stage;

   switch (stage)
   {
   case PIPE_SHADER_VERTEX:
      key->vs.needs_format_emulation = 0;
      break; 
   case PIPE_SHADER_FRAGMENT:
      key->fs.all = 0;
      break;
   case PIPE_SHADER_GEOMETRY:
      key->gs.all = 0;
      break;
   case PIPE_SHADER_TESS_CTRL:
      key->hs.all = 0;
      key->hs.required_patch_outputs = nullptr;
      break;
   case PIPE_SHADER_TESS_EVAL:
      key->ds.tcs_vertices_out = 0;
      key->ds.prev_patch_outputs = 0;
      key->ds.required_patch_inputs = nullptr;
      break;
   case PIPE_SHADER_COMPUTE:
      memset(key->cs.workgroup_size, 0, sizeof(key->cs.workgroup_size));
      break;
   default: unreachable("Invalid stage type");
   }

   key->n_texture_states = 0;
   key->tex_wrap_states = sel_ctx->ctx->tex_wrap_states_shader_key;
   key->n_images = 0;

   if (prev) {
      /* We require as inputs what the previous stage has written,
       * except certain system values */

      struct d3d12_varying_info **output_vars = nullptr;

      switch (stage) {
      case PIPE_SHADER_FRAGMENT:
         system_out_values |= VARYING_BIT_POS | VARYING_BIT_PSIZ | VARYING_BIT_VIEWPORT | VARYING_BIT_LAYER;
         output_vars = &prev->current->output_vars_fs;
         break;
      case PIPE_SHADER_GEOMETRY:
         system_out_values |= VARYING_BIT_POS;
         output_vars = &prev->current->output_vars_gs;
         break;
      default:
         output_vars = &prev->current->output_vars_default;
         break;
      }

      uint64_t mask = prev->current->nir->info.outputs_written & ~system_out_values;

      if (*output_vars == nullptr) {
         *output_vars = fill_varyings(sel_ctx->ctx, prev->current->nir,
                                      nir_var_shader_out, mask, false);
      }

      key->required_varying_inputs = *output_vars;

      key->prev_varying_outputs = prev->current->nir->info.outputs_written;

      if (stage == PIPE_SHADER_TESS_EVAL) {
         uint32_t patch_mask = prev->current->nir->info.patch_outputs_written;

         if (prev->current->tess_eval_output_vars == nullptr) {
            prev->current->tess_eval_output_vars = fill_varyings(sel_ctx->ctx, prev->current->nir,
                                                                 nir_var_shader_out, patch_mask, true);
         }

         key->ds.required_patch_inputs = prev->current->tess_eval_output_vars;
         key->ds.prev_patch_outputs = patch_mask;
      }

      /* Set the provoking vertex based on the previous shader output. Only set the
       * key value if the driver actually supports changing the provoking vertex though */
      if (stage == PIPE_SHADER_FRAGMENT && sel_ctx->ctx->gfx_pipeline_state.rast &&
          !sel_ctx->needs_vertex_reordering &&
          d3d12_screen(sel_ctx->ctx->base.screen)->have_load_at_vertex)
         key->fs.provoking_vertex = sel_ctx->provoking_vertex;

      /* Get the input clip distance size. The info's clip_distance_array_size corresponds
       * to the output, and in cases of TES or GS you could have differently-sized inputs
       * and outputs. For FS, there is no output, so it's repurposed to mean input.
       */
      if (stage != PIPE_SHADER_FRAGMENT)
         key->input_clip_size = prev->current->nir->info.clip_distance_array_size;
   }

   /* We require as outputs what the next stage reads,
    * except certain system values */
   if (next) {
      if (!next->is_variant) {

         struct d3d12_varying_info **input_vars = &next->current->input_vars_default;

         if (stage == PIPE_SHADER_VERTEX) {
            system_generated_in_values |= VARYING_BIT_POS;
            input_vars = &next->current->input_vars_vs;
         }
         uint64_t mask = next->current->nir->info.inputs_read & ~system_generated_in_values;

         
         if (*input_vars == nullptr) {
            *input_vars = fill_varyings(sel_ctx->ctx, next->current->nir,
                                        nir_var_shader_in, mask, false);
         }

         key->required_varying_outputs = *input_vars;


         if (stage == PIPE_SHADER_TESS_CTRL) {
            uint32_t patch_mask = next->current->nir->info.patch_outputs_read;

            if (prev->current->tess_ctrl_input_vars == nullptr){
               prev->current->tess_ctrl_input_vars = fill_varyings(sel_ctx->ctx, prev->current->nir,
                                                                   nir_var_shader_in, patch_mask, true);
            }

            key->hs.required_patch_outputs = prev->current->tess_ctrl_input_vars;
            key->hs.next_patch_inputs = patch_mask;
         }
      }
      key->next_varying_inputs = next->current->nir->info.inputs_read;

   }

   if (stage == PIPE_SHADER_GEOMETRY ||
       ((stage == PIPE_SHADER_VERTEX || stage == PIPE_SHADER_TESS_EVAL) &&
          (!next || next->stage == PIPE_SHADER_FRAGMENT))) {
      key->last_vertex_processing_stage = 1;
      key->invert_depth = sel_ctx->ctx->reverse_depth_range;
      key->halfz = sel_ctx->ctx->gfx_pipeline_state.rast ?
         sel_ctx->ctx->gfx_pipeline_state.rast->base.clip_halfz : false;
      if (sel_ctx->ctx->pstipple.enabled &&
         sel_ctx->ctx->gfx_pipeline_state.rast->base.poly_stipple_enable)
         key->next_varying_inputs |= VARYING_BIT_POS;
   }

   if (stage == PIPE_SHADER_GEOMETRY && sel_ctx->ctx->gfx_pipeline_state.rast) {
      struct pipe_rasterizer_state *rast = &sel_ctx->ctx->gfx_pipeline_state.rast->base;
      if (sel_ctx->needs_point_sprite_lowering) {
         key->gs.writes_psize = 1;
         key->gs.point_size_per_vertex = rast->point_size_per_vertex;
         key->gs.sprite_coord_enable = rast->sprite_coord_enable;
         key->gs.sprite_origin_upper_left = (rast->sprite_coord_mode != PIPE_SPRITE_COORD_LOWER_LEFT);
         if (sel_ctx->ctx->flip_y < 0)
            key->gs.sprite_origin_upper_left = !key->gs.sprite_origin_upper_left;
         key->gs.aa_point = rast->point_smooth;
         key->gs.stream_output_factor = 6;
      } else if (sel_ctx->fill_mode_lowered == PIPE_POLYGON_MODE_LINE) {
         key->gs.stream_output_factor = 2;
      } else if (sel_ctx->needs_vertex_reordering && !sel->is_variant) {
         key->gs.triangle_strip = 1;
      }

      if (sel->is_variant && next && next->initial->info.inputs_read & VARYING_BIT_PRIMITIVE_ID)
         key->gs.primitive_id = 1;
   } else if (stage == PIPE_SHADER_FRAGMENT) {
      key->fs.missing_dual_src_outputs = sel_ctx->missing_dual_src_outputs;
      key->fs.frag_result_color_lowering = sel_ctx->frag_result_color_lowering;
      key->fs.manual_depth_range = sel_ctx->manual_depth_range;
      key->fs.polygon_stipple = sel_ctx->ctx->pstipple.enabled &&
         sel_ctx->ctx->gfx_pipeline_state.rast->base.poly_stipple_enable;
      key->fs.multisample_disabled = sel_ctx->ctx->gfx_pipeline_state.rast &&
         !sel_ctx->ctx->gfx_pipeline_state.rast->desc.MultisampleEnable;
      if (sel_ctx->ctx->gfx_pipeline_state.blend &&
          sel_ctx->ctx->gfx_pipeline_state.blend->desc.RenderTarget[0].LogicOpEnable &&
          !sel_ctx->ctx->gfx_pipeline_state.has_float_rtv) {
         key->fs.cast_to_uint = util_format_is_unorm(sel_ctx->ctx->fb.cbufs[0]->format);
         key->fs.cast_to_int = !key->fs.cast_to_uint;
      }
   } else if (stage == PIPE_SHADER_TESS_CTRL) {
      if (next && next->current->nir->info.stage == MESA_SHADER_TESS_EVAL) {
         key->hs.primitive_mode = next->current->nir->info.tess._primitive_mode;
         key->hs.ccw = next->current->nir->info.tess.ccw;
         key->hs.point_mode = next->current->nir->info.tess.point_mode;
         key->hs.spacing = next->current->nir->info.tess.spacing;
      } else {
         key->hs.primitive_mode = TESS_PRIMITIVE_QUADS;
         key->hs.ccw = true;
         key->hs.point_mode = false;
         key->hs.spacing = TESS_SPACING_EQUAL;
      }
      key->hs.patch_vertices_in = MAX2(sel_ctx->ctx->patch_vertices, 1);
   } else if (stage == PIPE_SHADER_TESS_EVAL) {
      if (prev && prev->current->nir->info.stage == MESA_SHADER_TESS_CTRL)
         key->ds.tcs_vertices_out = prev->current->nir->info.tess.tcs_vertices_out;
      else
         key->ds.tcs_vertices_out = 32;
   }

   if (sel->samples_int_textures) {
      key->samples_int_textures = sel->samples_int_textures;
      key->n_texture_states = sel_ctx->ctx->num_sampler_views[stage];
      /* Copy only states with integer textures */
      for(int i = 0; i < key->n_texture_states; ++i) {
         auto& wrap_state = sel_ctx->ctx->tex_wrap_states[stage][i];
         if (wrap_state.is_int_sampler) {
            memcpy(&key->tex_wrap_states[i], &wrap_state, sizeof(wrap_state));
            key->swizzle_state[i] = sel_ctx->ctx->tex_swizzle_state[stage][i];
         } else {
            memset(&key->tex_wrap_states[i], 0, sizeof(key->tex_wrap_states[i]));
            key->swizzle_state[i] = { PIPE_SWIZZLE_X,  PIPE_SWIZZLE_Y,  PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W };
         }
      }
   }

   for (unsigned i = 0, e = sel_ctx->ctx->num_samplers[stage]; i < e; ++i) {
      if (!sel_ctx->ctx->samplers[stage][i] ||
          sel_ctx->ctx->samplers[stage][i]->filter == PIPE_TEX_FILTER_NEAREST)
         continue;

      if (sel_ctx->ctx->samplers[stage][i]->wrap_r == PIPE_TEX_WRAP_CLAMP)
         key->tex_saturate_r |= 1 << i;
      if (sel_ctx->ctx->samplers[stage][i]->wrap_s == PIPE_TEX_WRAP_CLAMP)
         key->tex_saturate_s |= 1 << i;
      if (sel_ctx->ctx->samplers[stage][i]->wrap_t == PIPE_TEX_WRAP_CLAMP)
         key->tex_saturate_t |= 1 << i;
   }

   if (sel->compare_with_lod_bias_grad) {
      key->n_texture_states = sel_ctx->ctx->num_sampler_views[stage];
      memcpy(key->sampler_compare_funcs, sel_ctx->ctx->tex_compare_func[stage],
             key->n_texture_states * sizeof(enum compare_func));
      memcpy(key->swizzle_state, sel_ctx->ctx->tex_swizzle_state[stage],
             key->n_texture_states * sizeof(dxil_texture_swizzle_state));
      if (!sel->samples_int_textures) 
         memset(key->tex_wrap_states, 0, sizeof(key->tex_wrap_states[0]) * key->n_texture_states);
   }

   if (stage == PIPE_SHADER_VERTEX && sel_ctx->ctx->gfx_pipeline_state.ves) {
      key->vs.needs_format_emulation = sel_ctx->ctx->gfx_pipeline_state.ves->needs_format_emulation;
      if (key->vs.needs_format_emulation) {
         unsigned num_elements = sel_ctx->ctx->gfx_pipeline_state.ves->num_elements;

         memset(key->vs.format_conversion + num_elements,
                  0, 
                  sizeof(key->vs.format_conversion) - (num_elements * sizeof(enum pipe_format)));

         memcpy(key->vs.format_conversion, sel_ctx->ctx->gfx_pipeline_state.ves->format_conversion,
                  num_elements * sizeof(enum pipe_format));
      }
   }

   if (stage == PIPE_SHADER_FRAGMENT &&
       sel_ctx->ctx->gfx_stages[PIPE_SHADER_GEOMETRY] &&
       sel_ctx->ctx->gfx_stages[PIPE_SHADER_GEOMETRY]->is_variant &&
       sel_ctx->ctx->gfx_stages[PIPE_SHADER_GEOMETRY]->gs_key.has_front_face) {
      key->fs.remap_front_facing = 1;
   }

   if (stage == PIPE_SHADER_COMPUTE && sel_ctx->variable_workgroup_size) {
      memcpy(key->cs.workgroup_size, sel_ctx->variable_workgroup_size, sizeof(key->cs.workgroup_size));
   }

   key->n_images = sel_ctx->ctx->num_image_views[stage];
   for (int i = 0; i < key->n_images; ++i) {
      key->image_format_conversion[i].emulated_format = sel_ctx->ctx->image_view_emulation_formats[stage][i];
      if (key->image_format_conversion[i].emulated_format != PIPE_FORMAT_NONE)
         key->image_format_conversion[i].view_format = sel_ctx->ctx->image_views[stage][i].format;
   }

   key->hash = d3d12_shader_key_hash(key);
}

static void
select_shader_variant(struct d3d12_selection_context *sel_ctx, d3d12_shader_selector *sel,
                     d3d12_shader_selector *prev, d3d12_shader_selector *next)
{
   struct d3d12_context *ctx = sel_ctx->ctx;
   d3d12_shader_key key;
   nir_shader *new_nir_variant;
   unsigned pstipple_binding = UINT32_MAX;

   d3d12_fill_shader_key(sel_ctx, &key, sel, prev, next);

   /* Check for an existing variant */
   for (d3d12_shader *variant = sel->first; variant;
        variant = variant->next_variant) {

      if (d3d12_compare_shader_keys(sel_ctx, &key, &variant->key)) {
         sel->current = variant;
         return;
      }
   }

   /* Clone the NIR shader */
   new_nir_variant = nir_shader_clone(sel, sel->initial);

   /* Apply any needed lowering passes */
   if (key.stage == PIPE_SHADER_GEOMETRY) {
      if (key.gs.writes_psize) {
         NIR_PASS_V(new_nir_variant, d3d12_lower_point_sprite,
                    !key.gs.sprite_origin_upper_left,
                    key.gs.point_size_per_vertex,
                    key.gs.sprite_coord_enable,
                    key.next_varying_inputs);

         nir_function_impl *impl = nir_shader_get_entrypoint(new_nir_variant);
         nir_shader_gather_info(new_nir_variant, impl);
      }

      if (key.gs.primitive_id) {
         NIR_PASS_V(new_nir_variant, d3d12_lower_primitive_id);

         nir_function_impl *impl = nir_shader_get_entrypoint(new_nir_variant);
         nir_shader_gather_info(new_nir_variant, impl);
      }

      if (key.gs.triangle_strip)
         NIR_PASS_V(new_nir_variant, d3d12_lower_triangle_strip);
   }
   else if (key.stage == PIPE_SHADER_FRAGMENT)
   {
      if (key.fs.polygon_stipple) {
         NIR_PASS_V(new_nir_variant, nir_lower_pstipple_fs,
                    &pstipple_binding, 0, false, nir_type_bool1);

         nir_function_impl *impl = nir_shader_get_entrypoint(new_nir_variant);
         nir_shader_gather_info(new_nir_variant, impl);
      }

      if (key.fs.remap_front_facing) {
         dxil_nir_forward_front_face(new_nir_variant);

         nir_function_impl *impl = nir_shader_get_entrypoint(new_nir_variant);
         nir_shader_gather_info(new_nir_variant, impl);
      }

      if (key.fs.missing_dual_src_outputs) {
         NIR_PASS_V(new_nir_variant, d3d12_add_missing_dual_src_target,
                    key.fs.missing_dual_src_outputs);
      } else if (key.fs.frag_result_color_lowering) {
         NIR_PASS_V(new_nir_variant, nir_lower_fragcolor,
                    key.fs.frag_result_color_lowering);
      }

      if (key.fs.manual_depth_range)
         NIR_PASS_V(new_nir_variant, d3d12_lower_depth_range);
   }


   if (sel->compare_with_lod_bias_grad) {
      STATIC_ASSERT(sizeof(dxil_texture_swizzle_state) ==
                    sizeof(nir_lower_tex_shadow_swizzle));

      NIR_PASS_V(new_nir_variant, nir_lower_tex_shadow, key.n_texture_states,
                 key.sampler_compare_funcs, (nir_lower_tex_shadow_swizzle *)key.swizzle_state);
   }

   if (key.stage == PIPE_SHADER_FRAGMENT) {
      if (key.fs.cast_to_uint)
         NIR_PASS_V(new_nir_variant, d3d12_lower_uint_cast, false);
      if (key.fs.cast_to_int)
         NIR_PASS_V(new_nir_variant, d3d12_lower_uint_cast, true);
   }

   if (key.n_images) {
      d3d12_image_format_conversion_info_arr image_format_arr = { key.n_images, key.image_format_conversion };
      NIR_PASS_V(new_nir_variant, d3d12_lower_image_casts, &image_format_arr);
   }

   if (key.stage == PIPE_SHADER_COMPUTE && sel->workgroup_size_variable) {
      new_nir_variant->info.workgroup_size[0] = key.cs.workgroup_size[0];
      new_nir_variant->info.workgroup_size[1] = key.cs.workgroup_size[1];
      new_nir_variant->info.workgroup_size[2] = key.cs.workgroup_size[2];
   }

   if (new_nir_variant->info.stage == MESA_SHADER_TESS_CTRL) {
      new_nir_variant->info.tess._primitive_mode = (tess_primitive_mode)key.hs.primitive_mode;
      new_nir_variant->info.tess.ccw = key.hs.ccw;
      new_nir_variant->info.tess.point_mode = key.hs.point_mode;
      new_nir_variant->info.tess.spacing = key.hs.spacing;

      NIR_PASS_V(new_nir_variant, dxil_nir_set_tcs_patches_in, key.hs.patch_vertices_in);
   } else if (new_nir_variant->info.stage == MESA_SHADER_TESS_EVAL) {
      new_nir_variant->info.tess.tcs_vertices_out = key.ds.tcs_vertices_out;
   }

   {
      struct nir_lower_tex_options tex_options = { };
      tex_options.lower_txp = ~0u; /* No equivalent for textureProj */
      tex_options.lower_rect = true;
      tex_options.lower_rect_offset = true;
      tex_options.saturate_s = key.tex_saturate_s;
      tex_options.saturate_r = key.tex_saturate_r;
      tex_options.saturate_t = key.tex_saturate_t;
      tex_options.lower_invalid_implicit_lod = true;
      tex_options.lower_tg4_offsets = true;

      NIR_PASS_V(new_nir_variant, nir_lower_tex, &tex_options);
   }

   /* Add the needed in and outputs, and re-sort */
   if (prev) {
      if (key.required_varying_inputs != nullptr) {
         uint64_t mask = key.required_varying_inputs->mask & ~new_nir_variant->info.inputs_read;
         new_nir_variant->info.inputs_read |= mask;
         while (mask) {
            int slot = u_bit_scan64(&mask);
            create_varyings_from_info(new_nir_variant, key.required_varying_inputs, slot, nir_var_shader_in, false);
         }
      }

      if (sel->stage == PIPE_SHADER_TESS_EVAL) {
         uint32_t patch_mask = (uint32_t)key.ds.required_patch_inputs->mask & ~new_nir_variant->info.patch_inputs_read;
         new_nir_variant->info.patch_inputs_read |= patch_mask;
         while (patch_mask) {
            int slot = u_bit_scan(&patch_mask);
            create_varyings_from_info(new_nir_variant, key.ds.required_patch_inputs, slot, nir_var_shader_in, true);
         }
      }
      dxil_reassign_driver_locations(new_nir_variant, nir_var_shader_in,
                                      key.prev_varying_outputs);
   }


   if (next) {
      if (key.required_varying_outputs != nullptr) {
         uint64_t mask = key.required_varying_outputs->mask & ~new_nir_variant->info.outputs_written;
         new_nir_variant->info.outputs_written |= mask;
         while (mask) {
            int slot = u_bit_scan64(&mask);
            create_varyings_from_info(new_nir_variant, key.required_varying_outputs, slot, nir_var_shader_out, false);
         }
      }

      if (sel->stage == PIPE_SHADER_TESS_CTRL &&
            key.hs.required_patch_outputs != nullptr) {
         uint32_t patch_mask = (uint32_t)key.hs.required_patch_outputs->mask & ~new_nir_variant->info.patch_outputs_written;
         new_nir_variant->info.patch_outputs_written |= patch_mask;
         while (patch_mask) {
            int slot = u_bit_scan(&patch_mask);
            create_varyings_from_info(new_nir_variant, key.hs.required_patch_outputs, slot, nir_var_shader_out, true);
         }
      }
      dxil_reassign_driver_locations(new_nir_variant, nir_var_shader_out,
                                     key.next_varying_inputs);
   }

   d3d12_shader *new_variant = compile_nir(ctx, sel, &key, new_nir_variant);
   assert(new_variant);

   /* keep track of polygon stipple texture binding */
   new_variant->pstipple_binding = pstipple_binding;

   /* prepend the new shader in the selector chain and pick it */
   new_variant->next_variant = sel->first;
   sel->current = sel->first = new_variant;
}

static d3d12_shader_selector *
get_prev_shader(struct d3d12_context *ctx, pipe_shader_type current)
{
   switch (current) {
   case PIPE_SHADER_VERTEX:
      return NULL;
   case PIPE_SHADER_FRAGMENT:
      if (ctx->gfx_stages[PIPE_SHADER_GEOMETRY])
         return ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
      FALLTHROUGH;
   case PIPE_SHADER_GEOMETRY:
      if (ctx->gfx_stages[PIPE_SHADER_TESS_EVAL])
         return ctx->gfx_stages[PIPE_SHADER_TESS_EVAL];
      FALLTHROUGH;
   case PIPE_SHADER_TESS_EVAL:
      if (ctx->gfx_stages[PIPE_SHADER_TESS_CTRL])
         return ctx->gfx_stages[PIPE_SHADER_TESS_CTRL];
      FALLTHROUGH;
   case PIPE_SHADER_TESS_CTRL:
      return ctx->gfx_stages[PIPE_SHADER_VERTEX];
   default:
      unreachable("shader type not supported");
   }
}

static d3d12_shader_selector *
get_next_shader(struct d3d12_context *ctx, pipe_shader_type current)
{
   switch (current) {
   case PIPE_SHADER_VERTEX:
      if (ctx->gfx_stages[PIPE_SHADER_TESS_CTRL])
         return ctx->gfx_stages[PIPE_SHADER_TESS_CTRL];
      FALLTHROUGH;
   case PIPE_SHADER_TESS_CTRL:
      if (ctx->gfx_stages[PIPE_SHADER_TESS_EVAL])
         return ctx->gfx_stages[PIPE_SHADER_TESS_EVAL];
      FALLTHROUGH;
   case PIPE_SHADER_TESS_EVAL:
      if (ctx->gfx_stages[PIPE_SHADER_GEOMETRY])
         return ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
      FALLTHROUGH;
   case PIPE_SHADER_GEOMETRY:
      return ctx->gfx_stages[PIPE_SHADER_FRAGMENT];
   case PIPE_SHADER_FRAGMENT:
      return NULL;
   default:
      unreachable("shader type not supported");
   }
}

enum tex_scan_flags {
   TEX_SAMPLE_INTEGER_TEXTURE = 1 << 0,
   TEX_CMP_WITH_LOD_BIAS_GRAD = 1 << 1,
   TEX_SCAN_ALL_FLAGS         = (1 << 2) - 1
};

static unsigned
scan_texture_use(nir_shader *nir)
{
   unsigned result = 0;
   nir_foreach_function_impl(impl, nir) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type == nir_instr_type_tex) {
               auto tex = nir_instr_as_tex(instr);
               switch (tex->op) {
               case nir_texop_txb:
               case nir_texop_txl:
               case nir_texop_txd:
                  if (tex->is_shadow)
                     result |= TEX_CMP_WITH_LOD_BIAS_GRAD;
                  FALLTHROUGH;
               case nir_texop_tex:
                  if (tex->dest_type & (nir_type_int | nir_type_uint))
                     result |= TEX_SAMPLE_INTEGER_TEXTURE;
               default:
                  ;
               }
            }
            if (TEX_SCAN_ALL_FLAGS == result)
               return result;
         }
      }
   }
   return result;
}

static uint64_t
update_so_info(struct pipe_stream_output_info *so_info,
               uint64_t outputs_written)
{
   uint64_t so_outputs = 0;
   uint8_t reverse_map[64] = {0};
   unsigned slot = 0;

   while (outputs_written)
      reverse_map[slot++] = u_bit_scan64(&outputs_written);

   for (unsigned i = 0; i < so_info->num_outputs; i++) {
      struct pipe_stream_output *output = &so_info->output[i];

      /* Map Gallium's condensed "slots" back to real VARYING_SLOT_* enums */
      output->register_index = reverse_map[output->register_index];

      so_outputs |= 1ull << output->register_index;
   }

   return so_outputs;
}

static struct d3d12_shader_selector *
d3d12_create_shader_impl(struct d3d12_context *ctx,
                         struct d3d12_shader_selector *sel,
                         struct nir_shader *nir,
                         struct d3d12_shader_selector *prev,
                         struct d3d12_shader_selector *next)
{
   unsigned tex_scan_result = scan_texture_use(nir);
   sel->samples_int_textures = (tex_scan_result & TEX_SAMPLE_INTEGER_TEXTURE) != 0;
   sel->compare_with_lod_bias_grad = (tex_scan_result & TEX_CMP_WITH_LOD_BIAS_GRAD) != 0;
   sel->workgroup_size_variable = nir->info.workgroup_size_variable;
   
   /* Integer cube maps are not supported in DirectX because sampling is not supported
    * on integer textures and TextureLoad is not supported for cube maps, so we have to
    * lower integer cube maps to be handled like 2D textures arrays*/
   NIR_PASS_V(nir, dxil_nir_lower_int_cubemaps, true);

   /* Keep this initial shader as the blue print for possible variants */
   sel->initial = nir;
   sel->initial_output_vars = nullptr;
   sel->gs_key.varyings = nullptr;
   sel->tcs_key.varyings = nullptr;

   /*
    * We must compile some shader here, because if the previous or a next shaders exists later
    * when the shaders are bound, then the key evaluation in the shader selector will access
    * the current variant of these  prev and next shader, and we can only assign
    * a current variant when it has been successfully compiled.
    *
    * For shaders that require lowering because certain instructions are not available
    * and their emulation is state depended (like sampling an integer texture that must be
    * emulated and needs handling of boundary conditions, or shadow compare sampling with LOD),
    * we must go through the shader selector here to create a compilable variant.
    * For shaders that are not depended on the state this is just compiling the original
    * shader.
    *
    * TODO: get rid of having to compiling the shader here if it can be forseen that it will
    * be thrown away (i.e. it depends on states that are likely to change before the shader is
    * used for the first time)
    */
   struct d3d12_selection_context sel_ctx = {0};
   sel_ctx.ctx = ctx;
   select_shader_variant(&sel_ctx, sel, prev, next);

   if (!sel->current) {
      ralloc_free(sel);
      return NULL;
   }

   return sel;
}

struct d3d12_shader_selector *
d3d12_create_shader(struct d3d12_context *ctx,
                    pipe_shader_type stage,
                    const struct pipe_shader_state *shader)
{
   struct d3d12_shader_selector *sel = rzalloc(nullptr, d3d12_shader_selector);
   sel->stage = stage;

   struct nir_shader *nir = NULL;

   if (shader->type == PIPE_SHADER_IR_NIR) {
      nir = (nir_shader *)shader->ir.nir;
   } else {
      assert(shader->type == PIPE_SHADER_IR_TGSI);
      nir = tgsi_to_nir(shader->tokens, ctx->base.screen, false);
   }

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));
   memcpy(&sel->so_info, &shader->stream_output, sizeof(sel->so_info));
   update_so_info(&sel->so_info, nir->info.outputs_written);

   assert(nir != NULL);
   d3d12_shader_selector *prev = get_prev_shader(ctx, sel->stage);
   d3d12_shader_selector *next = get_next_shader(ctx, sel->stage);

   NIR_PASS_V(nir, dxil_nir_split_clip_cull_distance);
   NIR_PASS_V(nir, d3d12_split_needed_varyings);

   if (nir->info.stage != MESA_SHADER_VERTEX) {
      nir->info.inputs_read =
      dxil_reassign_driver_locations(nir, nir_var_shader_in,
                                     prev ? prev->current->nir->info.outputs_written : 0);
   } else {
      nir->info.inputs_read = dxil_sort_by_driver_location(nir, nir_var_shader_in);

      uint32_t driver_loc = 0;
      nir_foreach_variable_with_modes(var, nir, nir_var_shader_in) {
         var->data.driver_location = driver_loc;
         driver_loc += glsl_count_attribute_slots(var->type, false);
      }
   }

   if (nir->info.stage != MESA_SHADER_FRAGMENT) {
      nir->info.outputs_written =
            dxil_reassign_driver_locations(nir, nir_var_shader_out,
                                            next ? next->current->nir->info.inputs_read : 0);
   } else {
      NIR_PASS_V(nir, nir_lower_fragcoord_wtrans);
      NIR_PASS_V(nir, dxil_nir_lower_sample_pos);
      dxil_sort_ps_outputs(nir);
   }

   return d3d12_create_shader_impl(ctx, sel, nir, prev, next);
}

struct d3d12_shader_selector *
d3d12_create_compute_shader(struct d3d12_context *ctx,
                            const struct pipe_compute_state *shader)
{
   struct d3d12_shader_selector *sel = rzalloc(nullptr, d3d12_shader_selector);
   sel->stage = PIPE_SHADER_COMPUTE;

   struct nir_shader *nir = NULL;

   if (shader->ir_type == PIPE_SHADER_IR_NIR) {
      nir = (nir_shader *)shader->prog;
   } else {
      assert(shader->ir_type == PIPE_SHADER_IR_TGSI);
      nir = tgsi_to_nir(shader->prog, ctx->base.screen, false);
   }

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   NIR_PASS_V(nir, d3d12_lower_compute_state_vars);

   return d3d12_create_shader_impl(ctx, sel, nir, nullptr, nullptr);
}

void
d3d12_select_shader_variants(struct d3d12_context *ctx, const struct pipe_draw_info *dinfo)
{
   struct d3d12_selection_context sel_ctx;

   sel_ctx.ctx = ctx;
   sel_ctx.needs_point_sprite_lowering = needs_point_sprite_lowering(ctx, dinfo);
   sel_ctx.fill_mode_lowered = fill_mode_lowered(ctx, dinfo);
   sel_ctx.cull_mode_lowered = cull_mode_lowered(ctx, sel_ctx.fill_mode_lowered);
   sel_ctx.provoking_vertex = get_provoking_vertex(&sel_ctx, &sel_ctx.alternate_tri, dinfo);
   sel_ctx.needs_vertex_reordering = needs_vertex_reordering(&sel_ctx, dinfo);
   sel_ctx.missing_dual_src_outputs = ctx->missing_dual_src_outputs;
   sel_ctx.frag_result_color_lowering = frag_result_color_lowering(ctx);
   sel_ctx.manual_depth_range = ctx->manual_depth_range;

   d3d12_shader_selector* gs = ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
   if (gs == nullptr || gs->is_variant) {
      if (sel_ctx.fill_mode_lowered != PIPE_POLYGON_MODE_FILL || sel_ctx.needs_point_sprite_lowering || sel_ctx.needs_vertex_reordering)
         validate_geometry_shader_variant(&sel_ctx);
      else if (gs != nullptr) {
         ctx->gfx_stages[PIPE_SHADER_GEOMETRY] = NULL;
      }
   }

   validate_tess_ctrl_shader_variant(&sel_ctx);

   auto* stages = ctx->gfx_stages;
   d3d12_shader_selector* prev;
   d3d12_shader_selector* next;
   if (stages[PIPE_SHADER_VERTEX]) {
      next = get_next_shader(ctx, PIPE_SHADER_VERTEX);
      select_shader_variant(&sel_ctx, stages[PIPE_SHADER_VERTEX], nullptr, next);
   }
   if (stages[PIPE_SHADER_TESS_CTRL]) {
      prev = get_prev_shader(ctx, PIPE_SHADER_TESS_CTRL);
      next = get_next_shader(ctx, PIPE_SHADER_TESS_CTRL);
      select_shader_variant(&sel_ctx, stages[PIPE_SHADER_TESS_CTRL], prev, next);
   }
   if (stages[PIPE_SHADER_TESS_EVAL]) {
      prev = get_prev_shader(ctx, PIPE_SHADER_TESS_EVAL);
      next = get_next_shader(ctx, PIPE_SHADER_TESS_EVAL);
      select_shader_variant(&sel_ctx, stages[PIPE_SHADER_TESS_EVAL], prev, next);
   }
   if (stages[PIPE_SHADER_GEOMETRY]) {
      prev = get_prev_shader(ctx, PIPE_SHADER_GEOMETRY);
      next = get_next_shader(ctx, PIPE_SHADER_GEOMETRY);
      select_shader_variant(&sel_ctx, stages[PIPE_SHADER_GEOMETRY], prev, next);
   }
   if (stages[PIPE_SHADER_FRAGMENT]) {
      prev = get_prev_shader(ctx, PIPE_SHADER_FRAGMENT);
      select_shader_variant(&sel_ctx, stages[PIPE_SHADER_FRAGMENT], prev, nullptr);
   }
}

static const unsigned *
workgroup_size_variable(struct d3d12_context *ctx,
                        const struct pipe_grid_info *info)
{
   if (ctx->compute_state->workgroup_size_variable)
      return info->block;
   return nullptr;
}

void
d3d12_select_compute_shader_variants(struct d3d12_context *ctx, const struct pipe_grid_info *info)
{
   struct d3d12_selection_context sel_ctx = {};

   sel_ctx.ctx = ctx;
   sel_ctx.variable_workgroup_size = workgroup_size_variable(ctx, info);

   select_shader_variant(&sel_ctx, ctx->compute_state, nullptr, nullptr);
}

void
d3d12_shader_free(struct d3d12_shader_selector *sel)
{
   auto shader = sel->first;
   while (shader) {
      free(shader->bytecode);
      shader = shader->next_variant;
   }

   ralloc_free((void*)sel->initial);
   ralloc_free(sel);
}
