/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_cmd_buffer.h"
#include "nvk_physical_device.h"
#include "nvk_shader.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_xfb_info.h"

#include "nv50_ir_driver.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"

#include "nvk_cl9097.h"

uint64_t
nvk_cg_get_prog_debug(void)
{
   return debug_get_num_option("NV50_PROG_DEBUG", 0);
}

uint64_t
nvk_cg_get_prog_optimize(void)
{
   return debug_get_num_option("NV50_PROG_OPTIMIZE", 3);
}

const nir_shader_compiler_options *
nvk_cg_nir_options(const struct nvk_physical_device *pdev,
                   gl_shader_stage stage)
{
   return nv50_ir_nir_shader_compiler_options(pdev->info.chipset, stage);
}

static nir_variable *
find_or_create_input(nir_builder *b, const struct glsl_type *type,
                     const char *name, unsigned location)
{
   nir_foreach_shader_in_variable(in, b->shader) {
      if (in->data.location == location)
         return in;
   }
   nir_variable *in = nir_variable_create(b->shader, nir_var_shader_in,
                                          type, name);
   in->data.location = location;
   if (glsl_type_is_integer(type))
      in->data.interpolation = INTERP_MODE_FLAT;
   else
      in->data.interpolation = INTERP_MODE_NOPERSPECTIVE;

   return in;
}

static bool
lower_fragcoord_instr(nir_builder *b, nir_instr *instr, UNUSED void *_data)
{
   assert(b->shader->info.stage == MESA_SHADER_FRAGMENT);
   nir_variable *var;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *val;
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_frag_coord:
      var = find_or_create_input(b, glsl_vec4_type(),
                                 "gl_FragCoord",
                                 VARYING_SLOT_POS);
      val = nir_load_var(b, var);
      break;
   case nir_intrinsic_load_point_coord:
      var = find_or_create_input(b, glsl_vector_type(GLSL_TYPE_FLOAT, 2),
                                 "gl_PointCoord",
                                 VARYING_SLOT_PNTC);
      val = nir_load_var(b, var);
      break;
   case nir_intrinsic_load_sample_pos:
      var = find_or_create_input(b, glsl_vec4_type(),
                                 "gl_FragCoord",
                                 VARYING_SLOT_POS);
      val = nir_ffract(b, nir_trim_vector(b, nir_load_var(b, var), 2));
      break;
   case nir_intrinsic_load_layer_id:
      var = find_or_create_input(b, glsl_int_type(),
                                 "gl_Layer", VARYING_SLOT_LAYER);
      val = nir_load_var(b, var);
      break;

   default:
      return false;
   }

   nir_def_rewrite_uses(&intrin->def, val);

   return true;
}

void
nvk_cg_preprocess_nir(nir_shader *nir)
{
   NIR_PASS(_, nir, nir_split_struct_vars, nir_var_function_temp);
   NIR_PASS(_, nir, nir_lower_vars_to_ssa);

   NIR_PASS(_, nir, nir_split_var_copies);
   NIR_PASS(_, nir, nir_lower_vars_to_ssa);

   NIR_PASS(_, nir, nir_lower_global_vars_to_local);
   NIR_PASS(_, nir, nir_remove_dead_variables, nir_var_function_temp, NULL);

   NIR_PASS(_, nir, nir_lower_system_values);

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS(_, nir, nir_shader_instructions_pass, lower_fragcoord_instr,
               nir_metadata_block_index | nir_metadata_dominance, NULL);
   }

   nvk_cg_optimize_nir(nir);

   NIR_PASS(_, nir, nir_lower_var_copies);
}

void
nvk_cg_optimize_nir(nir_shader *nir)
{
   bool progress;

   do {
      progress = false;

      NIR_PASS(progress, nir, nir_split_array_vars, nir_var_function_temp);
      NIR_PASS(progress, nir, nir_shrink_vec_array_vars, nir_var_function_temp);

      if (!nir->info.var_copies_lowered) {
         /* Only run this pass if nir_lower_var_copies was not called
          * yet. That would lower away any copy_deref instructions and we
          * don't want to introduce any more.
          */
         NIR_PASS(progress, nir, nir_opt_find_array_copies);
      }
      NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
      NIR_PASS(progress, nir, nir_opt_dead_write_vars);
      NIR_PASS(progress, nir, nir_lower_vars_to_ssa);
      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_opt_dce);
      if (nir_opt_loop(nir)) {
         progress = true;
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_remove_phis);
         NIR_PASS(progress, nir, nir_opt_dce);
      }
      NIR_PASS(progress, nir, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      /*
       * this should be fine, likely a backend problem,
       * but a bunch of tessellation shaders blow up.
       * we should revisit this when NAK is merged.
       */
      NIR_PASS(progress, nir, nir_opt_peephole_select, 2, true, true);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_opt_algebraic);

      NIR_PASS(progress, nir, nir_opt_undef);

      if (nir->options->max_unroll_iterations) {
         NIR_PASS(progress, nir, nir_opt_loop_unroll);
      }
   } while (progress);

   NIR_PASS(progress, nir, nir_opt_shrink_vectors);
   NIR_PASS(progress, nir, nir_remove_dead_variables,
            nir_var_function_temp | nir_var_shader_in | nir_var_shader_out, NULL);
}

static bool
lower_image_size_to_txs(nir_builder *b, nir_intrinsic_instr *intrin,
                        UNUSED void *_data)
{
   if (intrin->intrinsic != nir_intrinsic_image_deref_size)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);

   nir_deref_instr *img = nir_src_as_deref(intrin->src[0]);
   nir_def *lod = nir_tex_type_has_lod(img->type) ?
                      intrin->src[1].ssa : NULL;
   nir_def *size = nir_txs_deref(b, img, lod);

   if (glsl_get_sampler_dim(img->type) == GLSL_SAMPLER_DIM_CUBE) {
      /* Cube image descriptors are set up as simple arrays but SPIR-V wants
       * the number of cubes.
       */
      if (glsl_sampler_type_is_array(img->type)) {
         size = nir_vec3(b, nir_channel(b, size, 0),
                            nir_channel(b, size, 1),
                            nir_udiv_imm(b, nir_channel(b, size, 2), 6));
      } else {
         size = nir_vec3(b, nir_channel(b, size, 0),
                            nir_channel(b, size, 1),
                            nir_imm_int(b, 1));
      }
   }

   nir_def_rewrite_uses(&intrin->def, size);

   return true;
}

static int
count_location_slots(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

static void
assign_io_locations(nir_shader *nir)
{
   if (nir->info.stage != MESA_SHADER_VERTEX) {
      unsigned location = 0;
      nir_foreach_variable_with_modes(var, nir, nir_var_shader_in) {
         var->data.driver_location = location;
         if (nir_is_arrayed_io(var, nir->info.stage)) {
            location += glsl_count_attribute_slots(glsl_get_array_element(var->type), false);
         } else {
            location += glsl_count_attribute_slots(var->type, false);
         }
      }
      nir->num_inputs = location;
   } else {
      nir_foreach_shader_in_variable(var, nir) {
         assert(var->data.location >= VERT_ATTRIB_GENERIC0);
         var->data.driver_location = var->data.location - VERT_ATTRIB_GENERIC0;
      }
   }

   {
      unsigned location = 0;
      nir_foreach_variable_with_modes(var, nir, nir_var_shader_out) {
         var->data.driver_location = location;
         if (nir_is_arrayed_io(var, nir->info.stage)) {
            location += glsl_count_attribute_slots(glsl_get_array_element(var->type), false);
         } else {
            location += glsl_count_attribute_slots(var->type, false);
         }
      }
      nir->num_outputs = location;
   }
}

static void
nak_cg_postprocess_nir(nir_shader *nir)
{
   NIR_PASS(_, nir, nir_shader_intrinsics_pass, lower_image_size_to_txs,
            nir_metadata_block_index | nir_metadata_dominance, NULL);

   uint32_t indirect_mask = nir_var_function_temp;

   NIR_PASS(_, nir, nir_lower_indirect_derefs, indirect_mask, 16);

   nvk_cg_optimize_nir(nir);
   if (nir->info.stage != MESA_SHADER_COMPUTE)
      assign_io_locations(nir);

   NIR_PASS(_, nir, nir_lower_int64);

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));
}

/* NOTE: Using a[0x270] in FP may cause an error even if we're using less than
 * 124 scalar varying values.
 */
static uint32_t
nvc0_shader_input_address(unsigned sn, unsigned si)
{
   switch (sn) {
   case TGSI_SEMANTIC_TESSOUTER:    return 0x000 + si * 0x4;
   case TGSI_SEMANTIC_TESSINNER:    return 0x010 + si * 0x4;
   case TGSI_SEMANTIC_PATCH:        return 0x020 + si * 0x10;
   case TGSI_SEMANTIC_PRIMID:       return 0x060;
   case TGSI_SEMANTIC_LAYER:        return 0x064;
   case TGSI_SEMANTIC_VIEWPORT_INDEX:return 0x068;
   case TGSI_SEMANTIC_PSIZE:        return 0x06c;
   case TGSI_SEMANTIC_POSITION:     return 0x070;
   case TGSI_SEMANTIC_GENERIC:      return 0x080 + si * 0x10;
   case TGSI_SEMANTIC_FOG:          return 0x2e8;
   case TGSI_SEMANTIC_COLOR:        return 0x280 + si * 0x10;
   case TGSI_SEMANTIC_BCOLOR:       return 0x2a0 + si * 0x10;
   case TGSI_SEMANTIC_CLIPDIST:     return 0x2c0 + si * 0x10;
   case TGSI_SEMANTIC_CLIPVERTEX:   return 0x270;
   case TGSI_SEMANTIC_PCOORD:       return 0x2e0;
   case TGSI_SEMANTIC_TESSCOORD:    return 0x2f0;
   case TGSI_SEMANTIC_INSTANCEID:   return 0x2f8;
   case TGSI_SEMANTIC_VERTEXID:     return 0x2fc;
   case TGSI_SEMANTIC_TEXCOORD:     return 0x300 + si * 0x10;
   default:
      assert(!"invalid TGSI input semantic");
      return ~0;
   }
}

static uint32_t
nvc0_shader_output_address(unsigned sn, unsigned si)
{
   switch (sn) {
   case TGSI_SEMANTIC_TESSOUTER:     return 0x000 + si * 0x4;
   case TGSI_SEMANTIC_TESSINNER:     return 0x010 + si * 0x4;
   case TGSI_SEMANTIC_PATCH:         return 0x020 + si * 0x10;
   case TGSI_SEMANTIC_PRIMID:        return 0x060;
   case TGSI_SEMANTIC_LAYER:         return 0x064;
   case TGSI_SEMANTIC_VIEWPORT_INDEX:return 0x068;
   case TGSI_SEMANTIC_PSIZE:         return 0x06c;
   case TGSI_SEMANTIC_POSITION:      return 0x070;
   case TGSI_SEMANTIC_GENERIC:       return 0x080 + si * 0x10;
   case TGSI_SEMANTIC_FOG:           return 0x2e8;
   case TGSI_SEMANTIC_COLOR:         return 0x280 + si * 0x10;
   case TGSI_SEMANTIC_BCOLOR:        return 0x2a0 + si * 0x10;
   case TGSI_SEMANTIC_CLIPDIST:      return 0x2c0 + si * 0x10;
   case TGSI_SEMANTIC_CLIPVERTEX:    return 0x270;
   case TGSI_SEMANTIC_TEXCOORD:      return 0x300 + si * 0x10;
   case TGSI_SEMANTIC_VIEWPORT_MASK: return 0x3a0;
   case TGSI_SEMANTIC_EDGEFLAG:      return ~0;
   default:
      assert(!"invalid TGSI output semantic");
      return ~0;
   }
}

static int
nvc0_vp_assign_input_slots(struct nv50_ir_prog_info_out *info)
{
   unsigned i, c, n;

   for (n = 0, i = 0; i < info->numInputs; ++i) {
      switch (info->in[i].sn) {
      case TGSI_SEMANTIC_INSTANCEID: /* for SM4 only, in TGSI they're SVs */
      case TGSI_SEMANTIC_VERTEXID:
         info->in[i].mask = 0x1;
         info->in[i].slot[0] =
            nvc0_shader_input_address(info->in[i].sn, 0) / 4;
         continue;
      default:
         break;
      }
      for (c = 0; c < 4; ++c)
         info->in[i].slot[c] = (0x80 + n * 0x10 + c * 0x4) / 4;
      ++n;
   }

   return 0;
}

static int
nvc0_sp_assign_input_slots(struct nv50_ir_prog_info_out *info)
{
   unsigned offset;
   unsigned i, c;

   for (i = 0; i < info->numInputs; ++i) {
      offset = nvc0_shader_input_address(info->in[i].sn, info->in[i].si);

      for (c = 0; c < 4; ++c)
         info->in[i].slot[c] = (offset + c * 0x4) / 4;
   }

   return 0;
}

static int
nvc0_fp_assign_output_slots(struct nv50_ir_prog_info_out *info)
{
   unsigned count = info->prop.fp.numColourResults * 4;
   unsigned i, c;

   /* Compute the relative position of each color output, since skipped MRT
    * positions will not have registers allocated to them.
    */
   unsigned colors[8] = {0};
   for (i = 0; i < info->numOutputs; ++i)
      if (info->out[i].sn == TGSI_SEMANTIC_COLOR)
         colors[info->out[i].si] = 1;
   for (i = 0, c = 0; i < 8; i++)
      if (colors[i])
         colors[i] = c++;
   for (i = 0; i < info->numOutputs; ++i)
      if (info->out[i].sn == TGSI_SEMANTIC_COLOR)
         for (c = 0; c < 4; ++c)
            info->out[i].slot[c] = colors[info->out[i].si] * 4 + c;

   if (info->io.sampleMask < NV50_CODEGEN_MAX_VARYINGS)
      info->out[info->io.sampleMask].slot[0] = count++;
   else
   if (info->target >= 0xe0)
      count++; /* on Kepler, depth is always last colour reg + 2 */

   if (info->io.fragDepth < NV50_CODEGEN_MAX_VARYINGS)
      info->out[info->io.fragDepth].slot[2] = count;

   return 0;
}

static int
nvc0_sp_assign_output_slots(struct nv50_ir_prog_info_out *info)
{
   unsigned offset;
   unsigned i, c;

   for (i = 0; i < info->numOutputs; ++i) {
      offset = nvc0_shader_output_address(info->out[i].sn, info->out[i].si);

      for (c = 0; c < 4; ++c)
         info->out[i].slot[c] = (offset + c * 0x4) / 4;
   }

   return 0;
}

static int
nvc0_program_assign_varying_slots(struct nv50_ir_prog_info_out *info)
{
   int ret;

   if (info->type == PIPE_SHADER_VERTEX)
      ret = nvc0_vp_assign_input_slots(info);
   else
      ret = nvc0_sp_assign_input_slots(info);
   if (ret)
      return ret;

   if (info->type == PIPE_SHADER_FRAGMENT)
      ret = nvc0_fp_assign_output_slots(info);
   else
      ret = nvc0_sp_assign_output_slots(info);
   return ret;
}

static inline void
nvk_vtgs_hdr_update_oread(struct nvk_shader *vs, uint8_t slot)
{
   uint8_t min = (vs->info.hdr[4] >> 12) & 0xff;
   uint8_t max = (vs->info.hdr[4] >> 24);

   min = MIN2(min, slot);
   max = MAX2(max, slot);

   vs->info.hdr[4] = (max << 24) | (min << 12);
}

static int
nvk_vtgp_gen_header(struct nvk_shader *vs, struct nv50_ir_prog_info_out *info)
{
   unsigned i, c, a;

   for (i = 0; i < info->numInputs; ++i) {
      if (info->in[i].patch)
         continue;
      for (c = 0; c < 4; ++c) {
         a = info->in[i].slot[c];
         if (info->in[i].mask & (1 << c))
            vs->info.hdr[5 + a / 32] |= 1 << (a % 32);
      }
   }

   for (i = 0; i < info->numOutputs; ++i) {
      if (info->out[i].patch)
         continue;
      for (c = 0; c < 4; ++c) {
         if (!(info->out[i].mask & (1 << c)))
            continue;
         assert(info->out[i].slot[c] >= 0x40 / 4);
         a = info->out[i].slot[c] - 0x40 / 4;
         vs->info.hdr[13 + a / 32] |= 1 << (a % 32);
         if (info->out[i].oread)
            nvk_vtgs_hdr_update_oread(vs, info->out[i].slot[c]);
      }
   }

   for (i = 0; i < info->numSysVals; ++i) {
      switch (info->sv[i].sn) {
      case SYSTEM_VALUE_PRIMITIVE_ID:
         vs->info.hdr[5] |= 1 << 24;
         break;
      case SYSTEM_VALUE_INSTANCE_ID:
         vs->info.hdr[10] |= 1 << 30;
         break;
      case SYSTEM_VALUE_VERTEX_ID:
         vs->info.hdr[10] |= 1 << 31;
         break;
      case SYSTEM_VALUE_TESS_COORD:
         /* We don't have the mask, nor the slots populated. While this could
          * be achieved, the vast majority of the time if either of the coords
          * are read, then both will be read.
          */
         nvk_vtgs_hdr_update_oread(vs, 0x2f0 / 4);
         nvk_vtgs_hdr_update_oread(vs, 0x2f4 / 4);
         break;
      default:
         break;
      }
   }

   vs->info.vtg.writes_layer = (vs->info.hdr[13] & (1 << 9)) != 0;
   vs->info.vtg.clip_enable = (1 << info->io.clipDistances) - 1;
   vs->info.vtg.cull_enable =
      ((1 << info->io.cullDistances) - 1) << info->io.clipDistances;

   return 0;
}

static int
nvk_vs_gen_header(struct nvk_shader *vs, struct nv50_ir_prog_info_out *info)
{
   vs->info.hdr[0] = 0x20061 | (1 << 10);
   vs->info.hdr[4] = 0xff000;

   return nvk_vtgp_gen_header(vs, info);
}

static int
nvk_gs_gen_header(struct nvk_shader *gs,
                  const struct nir_shader *nir,
                  struct nv50_ir_prog_info_out *info)
{
   gs->info.hdr[0] = 0x20061 | (4 << 10);

   gs->info.hdr[2] = MIN2(info->prop.gp.instanceCount, 32) << 24;

   switch (info->prop.gp.outputPrim) {
   case MESA_PRIM_POINTS:
      gs->info.hdr[3] = 0x01000000;
      break;
   case MESA_PRIM_LINE_STRIP:
      gs->info.hdr[3] = 0x06000000;
      break;
   case MESA_PRIM_TRIANGLE_STRIP:
      gs->info.hdr[3] = 0x07000000;
      break;
   default:
      assert(0);
      break;
   }

   gs->info.hdr[4] = CLAMP(info->prop.gp.maxVertices, 1, 1024);

   gs->info.hdr[0] |= nir->info.gs.active_stream_mask << 28;

   return nvk_vtgp_gen_header(gs, info);
}

static void
nvk_generate_tessellation_parameters(const struct nv50_ir_prog_info_out *info,
                                     struct nvk_shader *shader)
{
   // TODO: this is a little confusing because nouveau codegen uses
   // MESA_PRIM_POINTS for unspecified domain and
   // MESA_PRIM_POINTS = 0, the same as NV9097 ISOLINE enum
   switch (info->prop.tp.domain) {
   case MESA_PRIM_LINES:
      shader->info.ts.domain = NAK_TS_DOMAIN_ISOLINE;
      break;
   case MESA_PRIM_TRIANGLES:
      shader->info.ts.domain = NAK_TS_DOMAIN_TRIANGLE;
      break;
   case MESA_PRIM_QUADS:
      shader->info.ts.domain = NAK_TS_DOMAIN_QUAD;
      break;
   default:
      return;
   }

   switch (info->prop.tp.partitioning) {
   case PIPE_TESS_SPACING_EQUAL:
      shader->info.ts.spacing = NAK_TS_SPACING_INTEGER;
      break;
   case PIPE_TESS_SPACING_FRACTIONAL_ODD:
      shader->info.ts.spacing = NAK_TS_SPACING_FRACT_ODD;
      break;
   case PIPE_TESS_SPACING_FRACTIONAL_EVEN:
      shader->info.ts.spacing = NAK_TS_SPACING_FRACT_EVEN;
      break;
   default:
      assert(!"invalid tessellator partitioning");
      break;
   }

   if (info->prop.tp.outputPrim == MESA_PRIM_POINTS) { // point_mode
      shader->info.ts.prims = NAK_TS_PRIMS_POINTS;
   } else if (info->prop.tp.domain == MESA_PRIM_LINES) { // isoline domain
      shader->info.ts.prims = NAK_TS_PRIMS_LINES;
   } else {  // triangle/quad domain
      if (info->prop.tp.winding > 0) {
         shader->info.ts.prims = NAK_TS_PRIMS_TRIANGLES_CW;
      } else {
         shader->info.ts.prims = NAK_TS_PRIMS_TRIANGLES_CCW;
      }
   }
}

static int
nvk_tcs_gen_header(struct nvk_shader *tcs, struct nv50_ir_prog_info_out *info)
{
   unsigned opcs = 6; /* output patch constants (at least the TessFactors) */

   if (info->numPatchConstants)
      opcs = 8 + info->numPatchConstants * 4;

   tcs->info.hdr[0] = 0x20061 | (2 << 10);

   tcs->info.hdr[1] = opcs << 24;
   tcs->info.hdr[2] = info->prop.tp.outputPatchSize << 24;

   tcs->info.hdr[4] = 0xff000; /* initial min/max parallel output read address */

   nvk_vtgp_gen_header(tcs, info);

   if (info->target >= NVISA_GM107_CHIPSET) {
      /* On GM107+, the number of output patch components has moved in the TCP
       * header, but it seems like blob still also uses the old position.
       * Also, the high 8-bits are located in between the min/max parallel
       * field and has to be set after updating the outputs. */
      tcs->info.hdr[3] = (opcs & 0x0f) << 28;
      tcs->info.hdr[4] |= (opcs & 0xf0) << 16;
   }

   nvk_generate_tessellation_parameters(info, tcs);

   return 0;
}

static int
nvk_tes_gen_header(struct nvk_shader *tes, struct nv50_ir_prog_info_out *info)
{
   tes->info.hdr[0] = 0x20061 | (3 << 10);
   tes->info.hdr[4] = 0xff000;

   nvk_vtgp_gen_header(tes, info);

   nvk_generate_tessellation_parameters(info, tes);

   tes->info.hdr[18] |= 0x3 << 12; /* ? */

   return 0;
}

#define NVC0_INTERP_FLAT          (1 << 0)
#define NVC0_INTERP_PERSPECTIVE   (2 << 0)
#define NVC0_INTERP_LINEAR        (3 << 0)
#define NVC0_INTERP_CENTROID      (1 << 2)

static uint8_t
nvk_hdr_interp_mode(const struct nv50_ir_varying *var)
{
   if (var->linear)
      return NVC0_INTERP_LINEAR;
   if (var->flat)
      return NVC0_INTERP_FLAT;
   return NVC0_INTERP_PERSPECTIVE;
}


static int
nvk_fs_gen_header(struct nvk_shader *fs, const struct nak_fs_key *key,
                  struct nv50_ir_prog_info_out *info)
{
   unsigned i, c, a, m;

   /* just 00062 on Kepler */
   fs->info.hdr[0] = 0x20062 | (5 << 10);
   fs->info.hdr[5] = 0x80000000; /* getting a trap if FRAG_COORD_UMASK.w = 0 */

   if (info->prop.fp.usesDiscard || key->zs_self_dep)
      fs->info.hdr[0] |= 0x8000;
   if (!info->prop.fp.separateFragData)
      fs->info.hdr[0] |= 0x4000;
   if (info->io.sampleMask < 80 /* PIPE_MAX_SHADER_OUTPUTS */)
      fs->info.hdr[19] |= 0x1;
   if (info->prop.fp.writesDepth) {
      fs->info.hdr[19] |= 0x2;
      fs->info.fs.writes_depth = true;
   }

   for (i = 0; i < info->numInputs; ++i) {
      m = nvk_hdr_interp_mode(&info->in[i]);
      for (c = 0; c < 4; ++c) {
         if (!(info->in[i].mask & (1 << c)))
            continue;
         a = info->in[i].slot[c];
         if (info->in[i].slot[0] >= (0x060 / 4) &&
             info->in[i].slot[0] <= (0x07c / 4)) {
            fs->info.hdr[5] |= 1 << (24 + (a - 0x060 / 4));
         } else
         if (info->in[i].slot[0] >= (0x2c0 / 4) &&
             info->in[i].slot[0] <= (0x2fc / 4)) {
            fs->info.hdr[14] |= (1 << (a - 0x280 / 4)) & 0x07ff0000;
         } else {
            if (info->in[i].slot[c] < (0x040 / 4) ||
                info->in[i].slot[c] > (0x380 / 4))
               continue;
            a *= 2;
            if (info->in[i].slot[0] >= (0x300 / 4))
               a -= 32;
            fs->info.hdr[4 + a / 32] |= m << (a % 32);
         }
      }
   }
   /* GM20x+ needs TGSI_SEMANTIC_POSITION to access sample locations */
   if (info->prop.fp.readsSampleLocations && info->target >= NVISA_GM200_CHIPSET)
      fs->info.hdr[5] |= 0x30000000;

   for (i = 0; i < info->numOutputs; ++i) {
      if (info->out[i].sn == TGSI_SEMANTIC_COLOR)
         fs->info.hdr[18] |= 0xf << (4 * info->out[i].si);
   }

   /* There are no "regular" attachments, but the shader still needs to be
    * executed. It seems like it wants to think that it has some color
    * outputs in order to actually run.
    */
   if (info->prop.fp.numColourResults == 0 &&
       !info->prop.fp.writesDepth &&
       info->io.sampleMask >= 80 /* PIPE_MAX_SHADER_OUTPUTS */)
      fs->info.hdr[18] |= 0xf;

   fs->info.fs.early_fragment_tests = info->prop.fp.earlyFragTests;
   fs->info.fs.reads_sample_mask = info->prop.fp.usesSampleMaskIn;
   fs->info.fs.post_depth_coverage = info->prop.fp.postDepthCoverage;

   return 0;
}

static uint8_t find_register_index_for_xfb_output(const struct nir_shader *nir,
                                                  nir_xfb_output_info output)
{
   nir_foreach_shader_out_variable(var, nir) {
      uint32_t slots = glsl_count_vec4_slots(var->type, false, false);
      for (uint32_t i = 0; i < slots; ++i) {
         if (output.location == (var->data.location+i)) {
            return var->data.driver_location+i;
         }
      }
   }
   // should not be reached
   return 0;
}

static void
nvk_fill_transform_feedback_state(struct nak_xfb_info *xfb,
                                  struct nir_shader *nir,
                                  const struct nv50_ir_prog_info_out *info)
{
   const uint8_t max_buffers = 4;
   const uint8_t dw_bytes = 4;
   const struct nir_xfb_info *nx = nir->xfb_info;
   //nir_print_xfb_info(nx, stdout);

   memset(xfb, 0, sizeof(*xfb));

   for (uint8_t b = 0; b < max_buffers; ++b) {
      xfb->stride[b] = b < nx->buffers_written ? nx->buffers[b].stride : 0;
      xfb->attr_count[b] = 0;
      xfb->stream[b] = nx->buffer_to_stream[b];
   }
   memset(xfb->attr_index, 0xff, sizeof(xfb->attr_index)); /* = skip */

   if (info->numOutputs == 0)
      return;

   for (uint32_t i = 0; i < nx->output_count; ++i) {
      const nir_xfb_output_info output = nx->outputs[i];
      const uint8_t b = output.buffer;
      const uint8_t r = find_register_index_for_xfb_output(nir, output);
      uint32_t p = output.offset / dw_bytes;

      assert(r < info->numOutputs && p < ARRAY_SIZE(xfb->attr_index[b]));

      u_foreach_bit(c, nx->outputs[i].component_mask)
         xfb->attr_index[b][p++] = info->out[r].slot[c];

      xfb->attr_count[b] = MAX2(xfb->attr_count[b], p);
   }

   /* zero unused indices */
   for (uint8_t b = 0; b < 4; ++b)
      for (uint32_t c = xfb->attr_count[b]; c & 3; ++c)
         xfb->attr_index[b][c] = 0;
}

VkResult
nvk_cg_compile_nir(struct nvk_physical_device *pdev, nir_shader *nir,
                   const struct nak_fs_key *fs_key,
                   struct nvk_shader *shader)
{
   struct nv50_ir_prog_info *info;
   struct nv50_ir_prog_info_out info_out = {};
   int ret;

   nak_cg_postprocess_nir(nir);

   info = CALLOC_STRUCT(nv50_ir_prog_info);
   if (!info)
      return false;

   info->type = nir->info.stage;
   info->target = pdev->info.chipset;
   info->bin.nir = nir;

   for (unsigned i = 0; i < 3; i++)
      shader->info.cs.local_size[i] = nir->info.workgroup_size[i];

   info->dbgFlags = nvk_cg_get_prog_debug();
   info->optLevel = nvk_cg_get_prog_optimize();
   info->io.auxCBSlot = 1;
   info->io.uboInfoBase = 0;
   info->io.drawInfoBase = nvk_root_descriptor_offset(draw.base_vertex);
   if (nir->info.stage == MESA_SHADER_COMPUTE) {
      info->prop.cp.gridInfoBase = 0;
   } else {
      info->assignSlots = nvc0_program_assign_varying_slots;
   }
   ret = nv50_ir_generate_code(info, &info_out);
   if (ret)
      return VK_ERROR_UNKNOWN;

   if (info_out.bin.fixupData) {
      nv50_ir_apply_fixups(info_out.bin.fixupData, info_out.bin.code,
                           fs_key && fs_key->force_sample_shading,
                           false /* flatshade */, false /* alphatest */,
                           fs_key && fs_key->force_sample_shading);
   }

   shader->info.stage = nir->info.stage;
   shader->code_ptr = (uint8_t *)info_out.bin.code;
   shader->code_size = info_out.bin.codeSize;

   if (info_out.target >= NVISA_GV100_CHIPSET)
      shader->info.num_gprs = MAX2(4, info_out.bin.maxGPR + 3);
   else
      shader->info.num_gprs = MAX2(4, info_out.bin.maxGPR + 1);
   shader->info.num_barriers = info_out.numBarriers;

   if (info_out.bin.tlsSpace) {
      assert(info_out.bin.tlsSpace < (1 << 24));
      shader->info.hdr[0] |= 1 << 26;
      shader->info.hdr[1] |= align(info_out.bin.tlsSpace, 0x10); /* l[] size */
      shader->info.slm_size = info_out.bin.tlsSpace;
   }

   switch (info->type) {
   case PIPE_SHADER_VERTEX:
      ret = nvk_vs_gen_header(shader, &info_out);
      break;
   case PIPE_SHADER_FRAGMENT:
      ret = nvk_fs_gen_header(shader, fs_key, &info_out);
      shader->info.fs.uses_sample_shading = nir->info.fs.uses_sample_shading;
      break;
   case PIPE_SHADER_GEOMETRY:
      ret = nvk_gs_gen_header(shader, nir, &info_out);
      break;
   case PIPE_SHADER_TESS_CTRL:
      ret = nvk_tcs_gen_header(shader, &info_out);
      break;
   case PIPE_SHADER_TESS_EVAL:
      ret = nvk_tes_gen_header(shader, &info_out);
      break;
   case PIPE_SHADER_COMPUTE:
      shader->info.cs.smem_size = info_out.bin.smemSize;
      break;
   default:
      unreachable("Invalid shader stage");
      break;
   }
   assert(ret == 0);

   if (info_out.io.globalAccess)
      shader->info.hdr[0] |= 1 << 26;
   if (info_out.io.globalAccess & 0x2)
      shader->info.hdr[0] |= 1 << 16;
   if (info_out.io.fp64)
      shader->info.hdr[0] |= 1 << 27;

   if (nir->xfb_info)
      nvk_fill_transform_feedback_state(&shader->info.vtg.xfb, nir, &info_out);

   return VK_SUCCESS;
}
