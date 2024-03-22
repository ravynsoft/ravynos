/*
 * Copyright © 2014-2015 Broadcom
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/blob.h"
#include "util/u_debug.h"
#include "util/disk_cache.h"
#include "util/u_memory.h"
#include "util/perf/cpu_trace.h"
#include "util/ralloc.h"
#include "pipe/p_screen.h"

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_control_flow.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_serialize.h"
#include "compiler/shader_enums.h"

#include "tgsi_to_nir.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_from_mesa.h"

#define SWIZ(X, Y, Z, W) (unsigned[4]){      \
      TGSI_SWIZZLE_##X,                      \
      TGSI_SWIZZLE_##Y,                      \
      TGSI_SWIZZLE_##Z,                      \
      TGSI_SWIZZLE_##W,                      \
   }

struct ttn_reg_info {
   /** nir register handle containing this TGSI index. */
   nir_def *reg;
   nir_variable *var;
   /** Offset (in vec4s) from the start of var for this TGSI index. */
   int offset;
};

struct ttn_compile {
   union tgsi_full_token *token;
   nir_builder build;
   struct tgsi_shader_info *scan;

   struct ttn_reg_info *output_regs;
   struct ttn_reg_info *temp_regs;
   nir_def **imm_defs;

   unsigned num_samp_types;
   nir_alu_type *samp_types;

   nir_def *addr_reg;

   nir_variable **inputs;
   nir_variable **outputs;
   nir_variable *samplers[PIPE_MAX_SAMPLERS];
   nir_variable *images[PIPE_MAX_SHADER_IMAGES];
   nir_variable *ssbo[PIPE_MAX_SHADER_BUFFERS];
   uint32_t ubo_sizes[PIPE_MAX_CONSTANT_BUFFERS];

   unsigned num_samplers;
   unsigned num_images;
   unsigned num_msaa_images;

   nir_variable *input_var_face;
   nir_variable *input_var_position;
   nir_variable *input_var_point;
   nir_variable *clipdist;

   /* How many TGSI_FILE_IMMEDIATE vec4s have been parsed so far. */
   unsigned next_imm;

   bool cap_face_is_sysval;
   bool cap_position_is_sysval;
   bool cap_point_is_sysval;
   bool cap_samplers_as_deref;
   bool cap_integers;
   bool cap_compact_arrays;
};

#define ttn_swizzle(b, src, x, y, z, w) \
   nir_swizzle(b, src, SWIZ(x, y, z, w), 4)
#define ttn_channel(b, src, swiz) \
   nir_channel(b, src, TGSI_SWIZZLE_##swiz)

static gl_varying_slot
tgsi_varying_semantic_to_slot(unsigned semantic, unsigned index)
{
   switch (semantic) {
   case TGSI_SEMANTIC_POSITION:
      return VARYING_SLOT_POS;
   case TGSI_SEMANTIC_COLOR:
      if (index == 0)
         return VARYING_SLOT_COL0;
      else
         return VARYING_SLOT_COL1;
   case TGSI_SEMANTIC_BCOLOR:
      if (index == 0)
         return VARYING_SLOT_BFC0;
      else
         return VARYING_SLOT_BFC1;
   case TGSI_SEMANTIC_FOG:
      return VARYING_SLOT_FOGC;
   case TGSI_SEMANTIC_PSIZE:
      return VARYING_SLOT_PSIZ;
   case TGSI_SEMANTIC_GENERIC:
      assert(index < 32);
      return VARYING_SLOT_VAR0 + index;
   case TGSI_SEMANTIC_FACE:
      return VARYING_SLOT_FACE;
   case TGSI_SEMANTIC_EDGEFLAG:
      return VARYING_SLOT_EDGE;
   case TGSI_SEMANTIC_PRIMID:
      return VARYING_SLOT_PRIMITIVE_ID;
   case TGSI_SEMANTIC_CLIPDIST:
      if (index == 0)
         return VARYING_SLOT_CLIP_DIST0;
      else
         return VARYING_SLOT_CLIP_DIST1;
   case TGSI_SEMANTIC_CLIPVERTEX:
      return VARYING_SLOT_CLIP_VERTEX;
   case TGSI_SEMANTIC_TEXCOORD:
      assert(index < 8);
      return VARYING_SLOT_TEX0 + index;
   case TGSI_SEMANTIC_PCOORD:
      return VARYING_SLOT_PNTC;
   case TGSI_SEMANTIC_VIEWPORT_INDEX:
      return VARYING_SLOT_VIEWPORT;
   case TGSI_SEMANTIC_LAYER:
      return VARYING_SLOT_LAYER;
   case TGSI_SEMANTIC_TESSINNER:
      return VARYING_SLOT_TESS_LEVEL_INNER;
   case TGSI_SEMANTIC_TESSOUTER:
      return VARYING_SLOT_TESS_LEVEL_OUTER;
   default:
      fprintf(stderr, "Bad TGSI semantic: %d/%d\n", semantic, index);
      abort();
   }
}

static enum gl_frag_depth_layout
ttn_get_depth_layout(unsigned tgsi_fs_depth_layout)
{
   switch (tgsi_fs_depth_layout) {
   case TGSI_FS_DEPTH_LAYOUT_NONE:
      return FRAG_DEPTH_LAYOUT_NONE;
   case TGSI_FS_DEPTH_LAYOUT_ANY:
      return FRAG_DEPTH_LAYOUT_ANY;
   case TGSI_FS_DEPTH_LAYOUT_GREATER:
      return FRAG_DEPTH_LAYOUT_GREATER;
   case TGSI_FS_DEPTH_LAYOUT_LESS:
      return FRAG_DEPTH_LAYOUT_LESS;
   case TGSI_FS_DEPTH_LAYOUT_UNCHANGED:
      return FRAG_DEPTH_LAYOUT_UNCHANGED;
   default:
      unreachable("bad TGSI FS depth layout");
   }
}

static enum glsl_interp_mode
ttn_translate_interp_mode(unsigned tgsi_interp)
{
   switch (tgsi_interp) {
   case TGSI_INTERPOLATE_CONSTANT:
      return INTERP_MODE_FLAT;
   case TGSI_INTERPOLATE_LINEAR:
      return INTERP_MODE_NOPERSPECTIVE;
   case TGSI_INTERPOLATE_PERSPECTIVE:
      return INTERP_MODE_SMOOTH;
   case TGSI_INTERPOLATE_COLOR:
      return INTERP_MODE_NONE;
   default:
      unreachable("bad TGSI interpolation mode");
   }
}

static void
ttn_emit_declaration(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_declaration *decl = &c->token->FullDeclaration;
   unsigned array_size = decl->Range.Last - decl->Range.First + 1;
   unsigned file = decl->Declaration.File;
   unsigned i;

   if (file == TGSI_FILE_TEMPORARY) {
      if (decl->Declaration.Array) {
         /* for arrays, we create variables instead of registers: */
         nir_variable *var =
            nir_variable_create(b->shader, nir_var_shader_temp,
                                glsl_array_type(glsl_vec4_type(), array_size, 0),
                                ralloc_asprintf(b->shader, "arr_%d",
                                                decl->Array.ArrayID));

         for (i = 0; i < array_size; i++) {
            /* point all the matching slots to the same var,
             * with appropriate offset set, mostly just so
             * we know what to do when tgsi does a non-indirect
             * access
             */
            c->temp_regs[decl->Range.First + i].reg = NULL;
            c->temp_regs[decl->Range.First + i].var = var;
            c->temp_regs[decl->Range.First + i].offset = i;
         }
      } else {
         for (i = 0; i < array_size; i++) {
            nir_def *reg = nir_decl_reg(b, 4, 32, 0);
            c->temp_regs[decl->Range.First + i].reg = reg;
            c->temp_regs[decl->Range.First + i].var = NULL;
            c->temp_regs[decl->Range.First + i].offset = 0;
         }
      }
   } else if (file == TGSI_FILE_ADDRESS) {
      c->addr_reg = nir_decl_reg(b, 4, 32, 0);
   } else if (file == TGSI_FILE_SYSTEM_VALUE) {
      /* Nothing to record for system values. */
   } else if (file == TGSI_FILE_BUFFER) {
      /* Nothing to record for buffers. */
   } else if (file == TGSI_FILE_IMAGE) {
      /* Nothing to record for images. */
   } else if (file == TGSI_FILE_SAMPLER) {
      /* Nothing to record for samplers. */
   } else if (file == TGSI_FILE_SAMPLER_VIEW) {
      struct tgsi_declaration_sampler_view *sview = &decl->SamplerView;
      nir_alu_type type;

      assert((sview->ReturnTypeX == sview->ReturnTypeY) &&
             (sview->ReturnTypeX == sview->ReturnTypeZ) &&
             (sview->ReturnTypeX == sview->ReturnTypeW));

      switch (sview->ReturnTypeX) {
      case TGSI_RETURN_TYPE_SINT:
         type = nir_type_int32;
         break;
      case TGSI_RETURN_TYPE_UINT:
         type = nir_type_uint32;
         break;
      case TGSI_RETURN_TYPE_FLOAT:
      default:
         type = nir_type_float32;
         break;
      }

      for (i = 0; i < array_size; i++) {
         c->samp_types[decl->Range.First + i] = type;
      }
   } else {
      bool is_array = (array_size > 1);

      assert(file == TGSI_FILE_INPUT ||
             file == TGSI_FILE_OUTPUT ||
             file == TGSI_FILE_CONSTANT);

      /* nothing to do for UBOs: */
      if ((file == TGSI_FILE_CONSTANT) && decl->Declaration.Dimension &&
          decl->Dim.Index2D != 0) {
         b->shader->info.num_ubos =
            MAX2(b->shader->info.num_ubos, decl->Dim.Index2D);
         c->ubo_sizes[decl->Dim.Index2D] =
            MAX2(c->ubo_sizes[decl->Dim.Index2D], decl->Range.Last * 16);
         return;
      }

      if ((file == TGSI_FILE_INPUT) || (file == TGSI_FILE_OUTPUT)) {
         is_array = (is_array && decl->Declaration.Array &&
                     (decl->Array.ArrayID != 0));
      }

      for (i = 0; i < array_size; i++) {
         unsigned idx = decl->Range.First + i;
         nir_variable *var = rzalloc(b->shader, nir_variable);

         var->data.driver_location = idx;

         var->type = glsl_vec4_type();
         if (is_array)
            var->type = glsl_array_type(var->type, array_size, 0);

         switch (file) {
         case TGSI_FILE_INPUT:
            var->data.read_only = true;
            var->data.mode = nir_var_shader_in;
            var->name = ralloc_asprintf(var, "in_%d", idx);

            if (c->scan->processor == PIPE_SHADER_FRAGMENT) {
               if (decl->Semantic.Name == TGSI_SEMANTIC_FACE) {
                  var->type = glsl_bool_type();
                  if (c->cap_face_is_sysval) {
                     var->data.mode = nir_var_system_value;
                     var->data.location = SYSTEM_VALUE_FRONT_FACE;
                  } else {
                     var->data.location = VARYING_SLOT_FACE;
                  }
                  c->input_var_face = var;
               } else if (decl->Semantic.Name == TGSI_SEMANTIC_POSITION) {
                  if (c->cap_position_is_sysval) {
                     var->data.mode = nir_var_system_value;
                     var->data.location = SYSTEM_VALUE_FRAG_COORD;
                  } else {
                     var->data.location = VARYING_SLOT_POS;
                  }
                  c->input_var_position = var;
               } else if (decl->Semantic.Name == TGSI_SEMANTIC_PCOORD) {
                  if (c->cap_point_is_sysval) {
                     var->data.mode = nir_var_system_value;
                     var->data.location = SYSTEM_VALUE_POINT_COORD;
                  } else {
                     var->data.location = VARYING_SLOT_PNTC;
                  }
                  c->input_var_point = var;
               } else {
                  var->data.location =
                     tgsi_varying_semantic_to_slot(decl->Semantic.Name,
                                                   decl->Semantic.Index);
               }
            } else {
               assert(!decl->Declaration.Semantic);
               var->data.location = VERT_ATTRIB_GENERIC0 + idx;
            }
            var->data.index = 0;
            var->data.interpolation =
               ttn_translate_interp_mode(decl->Interp.Interpolate);

            c->inputs[idx] = var;

            for (int i = 0; i < array_size; i++)
               b->shader->info.inputs_read |= 1ull << (var->data.location + i);

            break;
         case TGSI_FILE_OUTPUT: {
            int semantic_name = decl->Semantic.Name;
            int semantic_index = decl->Semantic.Index;
            /* Since we can't load from outputs in the IR, we make temporaries
             * for the outputs and emit stores to the real outputs at the end of
             * the shader.
             */
            nir_def *reg = nir_decl_reg(b, 4, 32,
                                            is_array ? array_size : 0);

            var->data.mode = nir_var_shader_out;
            var->name = ralloc_asprintf(var, "out_%d", idx);
            var->data.index = 0;
            var->data.interpolation =
               ttn_translate_interp_mode(decl->Interp.Interpolate);
            var->data.patch = semantic_name == TGSI_SEMANTIC_TESSINNER ||
                              semantic_name == TGSI_SEMANTIC_TESSOUTER ||
                              semantic_name == TGSI_SEMANTIC_PATCH;

            if (c->scan->processor == PIPE_SHADER_FRAGMENT) {
               switch (semantic_name) {
               case TGSI_SEMANTIC_COLOR: {
                  /* TODO tgsi loses some information, so we cannot
                   * actually differentiate here between DSB and MRT
                   * at this point.  But so far no drivers using tgsi-
                   * to-nir support dual source blend:
                   */
                  bool dual_src_blend = false;
                  if (dual_src_blend && (semantic_index == 1)) {
                     var->data.location = FRAG_RESULT_DATA0;
                     var->data.index = 1;
                  } else {
                     if (c->scan->properties[TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS])
                        var->data.location = FRAG_RESULT_COLOR;
                     else
                        var->data.location = FRAG_RESULT_DATA0 + semantic_index;
                  }
                  break;
               }
               case TGSI_SEMANTIC_POSITION:
                  var->data.location = FRAG_RESULT_DEPTH;
                  var->type = glsl_float_type();
                  break;
               case TGSI_SEMANTIC_STENCIL:
                  var->data.location = FRAG_RESULT_STENCIL;
                  var->type = glsl_int_type();
                  break;
               case TGSI_SEMANTIC_SAMPLEMASK:
                  var->data.location = FRAG_RESULT_SAMPLE_MASK;
                  var->type = glsl_int_type();
                  break;

               default:
                  fprintf(stderr, "Bad TGSI semantic: %d/%d\n",
                          decl->Semantic.Name, decl->Semantic.Index);
                  abort();
               }
            } else {
               var->data.location =
                  tgsi_varying_semantic_to_slot(semantic_name, semantic_index);
               if (var->data.location == VARYING_SLOT_FOGC ||
                   var->data.location == VARYING_SLOT_PSIZ) {
                  var->type = glsl_float_type();
               } else if (var->data.location == VARYING_SLOT_LAYER) {
                  var->type = glsl_int_type();
               } else if (c->cap_compact_arrays &&
                          var->data.location == VARYING_SLOT_CLIP_DIST0) {
                  var->type = glsl_array_type(glsl_float_type(),
                                              b->shader->info.clip_distance_array_size,
                                              sizeof(float));
                  c->clipdist = var;
               }
            }

            if (is_array) {
               unsigned j;
               for (j = 0; j < array_size; j++) {
                  c->output_regs[idx + j].offset = i + j;
                  c->output_regs[idx + j].reg = reg;
               }
            } else {
               c->output_regs[idx].offset = i;
               c->output_regs[idx].reg = reg;
            }

            c->outputs[idx] = var;

            if (c->cap_compact_arrays && var->data.location == VARYING_SLOT_CLIP_DIST1) {
               /* ignore this entirely */
               continue;
            }

            for (int i = 0; i < array_size; i++)
               b->shader->info.outputs_written |= 1ull << (var->data.location + i);
         }
            break;
         case TGSI_FILE_CONSTANT:
            var->data.mode = nir_var_uniform;
            var->name = ralloc_asprintf(var, "uniform_%d", idx);
            var->data.location = idx;
            break;
         default:
            unreachable("bad declaration file");
            return;
         }

         nir_shader_add_variable(b->shader, var);

         if (is_array)
            break;
      }

   }
}

static void
ttn_emit_immediate(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_immediate *tgsi_imm = &c->token->FullImmediate;
   nir_load_const_instr *load_const;
   int i;

   load_const = nir_load_const_instr_create(b->shader, 4, 32);
   c->imm_defs[c->next_imm] = &load_const->def;
   c->next_imm++;

   for (i = 0; i < load_const->def.num_components; i++)
      load_const->value[i].u32 = tgsi_imm->u[i].Uint;

   nir_builder_instr_insert(b, &load_const->instr);
}

static nir_def *
ttn_src_for_indirect(struct ttn_compile *c, struct tgsi_ind_register *indirect);

/* generate either a constant or indirect deref chain for accessing an
 * array variable.
 */
static nir_deref_instr *
ttn_array_deref(struct ttn_compile *c, nir_variable *var, unsigned offset,
                struct tgsi_ind_register *indirect)
{
   nir_deref_instr *deref = nir_build_deref_var(&c->build, var);
   nir_def *index = nir_imm_int(&c->build, offset);
   if (indirect)
      index = nir_iadd(&c->build, index, ttn_src_for_indirect(c, indirect));
   return nir_build_deref_array(&c->build, deref, index);
}

/* Special case: Turn the frontface varying into a load of the
 * frontface variable, and create the vector as required by TGSI.
 */
static nir_def *
ttn_emulate_tgsi_front_face(struct ttn_compile *c)
{
   nir_def *tgsi_frontface[4];

   if (c->cap_face_is_sysval) {
      /* When it's a system value, it should be an integer vector: (F, 0, 0, 1)
       * F is 0xffffffff if front-facing, 0 if not.
       */

      nir_def *frontface = nir_load_front_face(&c->build, 1);

      tgsi_frontface[0] = nir_bcsel(&c->build,
                             frontface,
                             nir_imm_int(&c->build, 0xffffffff),
                             nir_imm_int(&c->build, 0));
      tgsi_frontface[1] = nir_imm_int(&c->build, 0);
      tgsi_frontface[2] = nir_imm_int(&c->build, 0);
      tgsi_frontface[3] = nir_imm_int(&c->build, 1);
   } else {
      /* When it's an input, it should be a float vector: (F, 0.0, 0.0, 1.0)
       * F is positive if front-facing, negative if not.
       */

      assert(c->input_var_face);
      nir_def *frontface = nir_load_var(&c->build, c->input_var_face);

      tgsi_frontface[0] = nir_bcsel(&c->build,
                             frontface,
                             nir_imm_float(&c->build, 1.0),
                             nir_imm_float(&c->build, -1.0));
      tgsi_frontface[1] = nir_imm_float(&c->build, 0.0);
      tgsi_frontface[2] = nir_imm_float(&c->build, 0.0);
      tgsi_frontface[3] = nir_imm_float(&c->build, 1.0);
   }

   return nir_vec(&c->build, tgsi_frontface, 4);
}

static nir_src
ttn_src_for_file_and_index(struct ttn_compile *c, unsigned file, unsigned index,
                           struct tgsi_ind_register *indirect,
                           struct tgsi_dimension *dim,
                           struct tgsi_ind_register *dimind,
                           bool src_is_float)
{
   nir_builder *b = &c->build;
   nir_src src;

   memset(&src, 0, sizeof(src));

   switch (file) {
   case TGSI_FILE_TEMPORARY:
      if (c->temp_regs[index].var) {
         unsigned offset = c->temp_regs[index].offset;
         nir_variable *var = c->temp_regs[index].var;
         nir_def *load = nir_load_deref(&c->build,
               ttn_array_deref(c, var, offset, indirect));

         src = nir_src_for_ssa(load);
      } else {
         assert(!indirect);
         src = nir_src_for_ssa(nir_load_reg(b, c->temp_regs[index].reg));
      }
      assert(!dim);
      break;

   case TGSI_FILE_ADDRESS:
      src = nir_src_for_ssa(nir_load_reg(b, c->addr_reg));
      assert(!dim);
      break;

   case TGSI_FILE_IMMEDIATE:
      src = nir_src_for_ssa(c->imm_defs[index]);
      assert(!indirect);
      assert(!dim);
      break;

   case TGSI_FILE_SYSTEM_VALUE: {
      nir_def *load;

      assert(!indirect);
      assert(!dim);

      switch (c->scan->system_value_semantic_name[index]) {
      case TGSI_SEMANTIC_VERTEXID_NOBASE:
         load = nir_load_vertex_id_zero_base(b);
         break;
      case TGSI_SEMANTIC_VERTEXID:
         load = nir_load_vertex_id(b);
         break;
      case TGSI_SEMANTIC_BASEVERTEX:
         load = nir_load_base_vertex(b);
         break;
      case TGSI_SEMANTIC_INSTANCEID:
         load = nir_load_instance_id(b);
         break;
      case TGSI_SEMANTIC_FACE:
         assert(c->cap_face_is_sysval);
         load = ttn_emulate_tgsi_front_face(c);
         break;
      case TGSI_SEMANTIC_POSITION:
         assert(c->cap_position_is_sysval);
         load = nir_load_frag_coord(b);
         break;
      case TGSI_SEMANTIC_PCOORD:
         assert(c->cap_point_is_sysval);
         load = nir_load_point_coord(b);
         break;
      case TGSI_SEMANTIC_THREAD_ID:
         load = nir_load_local_invocation_id(b);
         break;
      case TGSI_SEMANTIC_BLOCK_ID:
         load = nir_load_workgroup_id(b);
         break;
      case TGSI_SEMANTIC_BLOCK_SIZE:
         load = nir_load_workgroup_size(b);
         break;
      case TGSI_SEMANTIC_CS_USER_DATA_AMD:
         load = nir_load_user_data_amd(b);
         break;
      case TGSI_SEMANTIC_TESS_DEFAULT_INNER_LEVEL:
         load = nir_load_tess_level_inner_default(b);
         break;
      case TGSI_SEMANTIC_TESS_DEFAULT_OUTER_LEVEL:
         load = nir_load_tess_level_outer_default(b);
         break;
      case TGSI_SEMANTIC_SAMPLEID:
         load = nir_load_sample_id(b);
         b->shader->info.fs.uses_sample_shading = true;
         break;
      default:
         unreachable("bad system value");
      }

      if (load->num_components == 2)
         load = nir_swizzle(b, load, SWIZ(X, Y, Y, Y), 4);
      else if (load->num_components == 3)
         load = nir_swizzle(b, load, SWIZ(X, Y, Z, Z), 4);

      src = nir_src_for_ssa(load);
      break;
   }

   case TGSI_FILE_INPUT:
      if (c->scan->processor == PIPE_SHADER_FRAGMENT &&
          c->scan->input_semantic_name[index] == TGSI_SEMANTIC_FACE) {
         assert(!c->cap_face_is_sysval && c->input_var_face);
         return nir_src_for_ssa(ttn_emulate_tgsi_front_face(c));
      } else if (c->scan->processor == PIPE_SHADER_FRAGMENT &&
          c->scan->input_semantic_name[index] == TGSI_SEMANTIC_POSITION) {
         assert(!c->cap_position_is_sysval && c->input_var_position);
         return nir_src_for_ssa(nir_load_var(&c->build, c->input_var_position));
      } else if (c->scan->processor == PIPE_SHADER_FRAGMENT &&
          c->scan->input_semantic_name[index] == TGSI_SEMANTIC_PCOORD) {
         assert(!c->cap_point_is_sysval && c->input_var_point);
         return nir_src_for_ssa(nir_load_var(&c->build, c->input_var_point));
      } else {
         /* Indirection on input arrays isn't supported by TTN. */
         assert(!dim);
         nir_deref_instr *deref = nir_build_deref_var(&c->build,
                                                      c->inputs[index]);
         return nir_src_for_ssa(nir_load_deref(&c->build, deref));
      }
      break;

   case TGSI_FILE_OUTPUT:
      if (c->scan->processor == PIPE_SHADER_FRAGMENT) {
         c->outputs[index]->data.fb_fetch_output = 1;
         nir_deref_instr *deref = nir_build_deref_var(&c->build,
                                                      c->outputs[index]);
         return nir_src_for_ssa(nir_load_deref(&c->build, deref));
      }
      unreachable("unsupported output read");
      break;

   case TGSI_FILE_CONSTANT: {
      nir_intrinsic_instr *load;
      nir_intrinsic_op op;
      unsigned srcn = 0;

      if (dim && (dim->Index > 0 || dim->Indirect)) {
         op = nir_intrinsic_load_ubo;
      } else {
         op = nir_intrinsic_load_uniform;
      }

      load = nir_intrinsic_instr_create(b->shader, op);
      if (op == nir_intrinsic_load_uniform) {
         nir_intrinsic_set_dest_type(load, src_is_float ? nir_type_float :
                                                          nir_type_int);
      }

      load->num_components = 4;
      if (dim && (dim->Index > 0 || dim->Indirect)) {
         if (dimind) {
            load->src[srcn] =
               ttn_src_for_file_and_index(c, dimind->File, dimind->Index,
                                          NULL, NULL, NULL, false);
         } else {
            /* UBOs start at index 1 in TGSI: */
            load->src[srcn] =
               nir_src_for_ssa(nir_imm_int(b, dim->Index - 1));
         }
         srcn++;
      }

      nir_def *offset;
      if (op == nir_intrinsic_load_ubo) {
         /* UBO loads don't have a base offset. */
         offset = nir_imm_int(b, index);
         if (indirect) {
            offset = nir_iadd(b, offset, ttn_src_for_indirect(c, indirect));
         }
         /* UBO offsets are in bytes, but TGSI gives them to us in vec4's */
         offset = nir_ishl_imm(b, offset, 4);
         nir_intrinsic_set_align(load, 16, 0);

         /* Set a very conservative base/range of the access: 16 bytes if not
          * indirect at all, offset to the end of the UBO if the offset is
          * indirect, and totally unknown if the block number is indirect.
          */
         uint32_t base = index * 16;
         nir_intrinsic_set_range_base(load, base);
         if (dimind)
            nir_intrinsic_set_range(load, ~0);
         else if (indirect)
            nir_intrinsic_set_range(load, c->ubo_sizes[dim->Index] - base);
         else
            nir_intrinsic_set_range(load, base + 16);
      } else {
         nir_intrinsic_set_base(load, index);
         if (indirect) {
            offset = ttn_src_for_indirect(c, indirect);
            nir_intrinsic_set_range(load, c->build.shader->num_uniforms * 16 - index);
         } else {
            offset = nir_imm_int(b, 0);
            nir_intrinsic_set_range(load, 1);
         }
      }
      load->src[srcn++] = nir_src_for_ssa(offset);

      nir_def_init(&load->instr, &load->def, 4, 32);
      nir_builder_instr_insert(b, &load->instr);

      src = nir_src_for_ssa(&load->def);
      break;
   }

   default:
      unreachable("bad src file");
   }


   return src;
}

static nir_def *
ttn_src_for_indirect(struct ttn_compile *c, struct tgsi_ind_register *indirect)
{
   nir_builder *b = &c->build;
   nir_alu_src src;
   memset(&src, 0, sizeof(src));
   for (int i = 0; i < 4; i++)
      src.swizzle[i] = indirect->Swizzle;
   src.src = ttn_src_for_file_and_index(c,
                                        indirect->File,
                                        indirect->Index,
                                        NULL, NULL, NULL,
                                        false);
   return nir_mov_alu(b, src, 1);
}

static nir_variable *
ttn_get_var(struct ttn_compile *c, struct tgsi_full_dst_register *tgsi_fdst)
{
   struct tgsi_dst_register *tgsi_dst = &tgsi_fdst->Register;
   unsigned index = tgsi_dst->Index;

   if (tgsi_dst->File == TGSI_FILE_TEMPORARY) {
      /* we should not have an indirect when there is no var! */
      if (!c->temp_regs[index].var)
         assert(!tgsi_dst->Indirect);
      return c->temp_regs[index].var;
   }

   return NULL;
}

static nir_def *
ttn_get_src(struct ttn_compile *c, struct tgsi_full_src_register *tgsi_fsrc,
            int src_idx)
{
   nir_builder *b = &c->build;
   struct tgsi_src_register *tgsi_src = &tgsi_fsrc->Register;
   enum tgsi_opcode opcode = c->token->FullInstruction.Instruction.Opcode;
   unsigned tgsi_src_type = tgsi_opcode_infer_src_type(opcode, src_idx);
   bool src_is_float = (tgsi_src_type == TGSI_TYPE_FLOAT ||
                        tgsi_src_type == TGSI_TYPE_DOUBLE ||
                        tgsi_src_type == TGSI_TYPE_UNTYPED);
   nir_alu_src src;

   memset(&src, 0, sizeof(src));

   if (tgsi_src->File == TGSI_FILE_NULL) {
      return nir_imm_float(b, 0.0);
   } else if (tgsi_src->File == TGSI_FILE_SAMPLER ||
              tgsi_src->File == TGSI_FILE_IMAGE ||
              tgsi_src->File == TGSI_FILE_BUFFER) {
      /* Only the index of the resource gets used in texturing, and it will
       * handle looking that up on its own instead of using the nir_alu_src.
       */
      assert(!tgsi_src->Indirect);
      return NULL;
   } else {
      struct tgsi_ind_register *ind = NULL;
      struct tgsi_dimension *dim = NULL;
      struct tgsi_ind_register *dimind = NULL;
      if (tgsi_src->Indirect)
         ind = &tgsi_fsrc->Indirect;
      if (tgsi_src->Dimension) {
         dim = &tgsi_fsrc->Dimension;
         if (dim->Indirect)
            dimind = &tgsi_fsrc->DimIndirect;
      }
      src.src = ttn_src_for_file_and_index(c,
                                           tgsi_src->File,
                                           tgsi_src->Index,
                                           ind, dim, dimind,
                                           src_is_float);
   }

   src.swizzle[0] = tgsi_src->SwizzleX;
   src.swizzle[1] = tgsi_src->SwizzleY;
   src.swizzle[2] = tgsi_src->SwizzleZ;
   src.swizzle[3] = tgsi_src->SwizzleW;

   nir_def *def = nir_mov_alu(b, src, 4);

   if (tgsi_type_is_64bit(tgsi_src_type))
      def = nir_bitcast_vector(b, def, 64);

   if (tgsi_src->Absolute) {
      assert(src_is_float);
      def = nir_fabs(b, def);
   }

   if (tgsi_src->Negate) {
      if (src_is_float)
         def = nir_fneg(b, def);
      else
         def = nir_ineg(b, def);
   }

   return def;
}

static nir_def *
ttn_alu(nir_builder *b, nir_op op, unsigned dest_bitsize, nir_def **src)
{
   nir_def *def = nir_build_alu_src_arr(b, op, src);
   if (def->bit_size == 1)
      def = nir_ineg(b, nir_b2iN(b, def, dest_bitsize));
   assert(def->bit_size == dest_bitsize);
   if (dest_bitsize == 64) {
      /* Replicate before bitcasting, so we end up with 4x32 at the end */
      if (def->num_components == 1)
         def = nir_replicate(b, def, 2);

      if (def->num_components > 2) {
         /* 32 -> 64 bit conversion ops are supposed to only convert the first
          * two components, and we need to truncate here to avoid creating a
          * vec8 after bitcasting the destination.
          */
         def = nir_trim_vector(b, def, 2);
      }
      def = nir_bitcast_vector(b, def, 32);
   }
   return def;
}

/* EXP - Approximate Exponential Base 2
 *  dst.x = 2^{\lfloor src.x\rfloor}
 *  dst.y = src.x - \lfloor src.x\rfloor
 *  dst.z = 2^{src.x}
 *  dst.w = 1.0
 */
static nir_def *
ttn_exp(nir_builder *b, nir_def **src)
{
   nir_def *srcx = ttn_channel(b, src[0], X);

   return nir_vec4(b, nir_fexp2(b, nir_ffloor(b, srcx)),
                      nir_fsub(b, srcx, nir_ffloor(b, srcx)),
                      nir_fexp2(b, srcx),
                      nir_imm_float(b, 1.0));
}

/* LOG - Approximate Logarithm Base 2
 *  dst.x = \lfloor\log_2{|src.x|}\rfloor
 *  dst.y = \frac{|src.x|}{2^{\lfloor\log_2{|src.x|}\rfloor}}
 *  dst.z = \log_2{|src.x|}
 *  dst.w = 1.0
 */
static nir_def *
ttn_log(nir_builder *b, nir_def **src)
{
   nir_def *abs_srcx = nir_fabs(b, ttn_channel(b, src[0], X));
   nir_def *log2 = nir_flog2(b, abs_srcx);

   return nir_vec4(b, nir_ffloor(b, log2),
                      nir_fdiv(b, abs_srcx, nir_fexp2(b, nir_ffloor(b, log2))),
                      nir_flog2(b, abs_srcx),
                      nir_imm_float(b, 1.0));
}

/* DST - Distance Vector
 *   dst.x = 1.0
 *   dst.y = src0.y \times src1.y
 *   dst.z = src0.z
 *   dst.w = src1.w
 */
static nir_def *
ttn_dst(nir_builder *b, nir_def **src)
{
   return nir_vec4(b, nir_imm_float(b, 1.0),
                      nir_fmul(b, ttn_channel(b, src[0], Y),
                                  ttn_channel(b, src[1], Y)),
                      ttn_channel(b, src[0], Z),
                      ttn_channel(b, src[1], W));
}

/* LIT - Light Coefficients
 *  dst.x = 1.0
 *  dst.y = max(src.x, 0.0)
 *  dst.z = (src.x > 0.0) ? max(src.y, 0.0)^{clamp(src.w, -128.0, 128.0))} : 0
 *  dst.w = 1.0
 */
static nir_def *
ttn_lit(nir_builder *b, nir_def **src)
{
   nir_def *src0_y = ttn_channel(b, src[0], Y);
   nir_def *wclamp = nir_fmax(b, nir_fmin(b, ttn_channel(b, src[0], W),
                                              nir_imm_float(b, 128.0)),
                                  nir_imm_float(b, -128.0));
   nir_def *pow = nir_fpow(b, nir_fmax(b, src0_y, nir_imm_float(b, 0.0)),
                               wclamp);
   nir_def *z = nir_bcsel(b, nir_flt_imm(b, ttn_channel(b, src[0], X), 0.0),
                                 nir_imm_float(b, 0.0), pow);

   return nir_vec4(b, nir_imm_float(b, 1.0),
                      nir_fmax(b, ttn_channel(b, src[0], X),
                                  nir_imm_float(b, 0.0)),
                      z, nir_imm_float(b, 1.0));
}

static void
ttn_barrier(nir_builder *b)
{
   nir_barrier(b, .execution_scope = SCOPE_WORKGROUP);
}

static void
ttn_kill(nir_builder *b)
{
   nir_discard(b);
   b->shader->info.fs.uses_discard = true;
}

static void
ttn_kill_if(nir_builder *b, nir_def **src)
{
   /* flt must be exact, because NaN shouldn't discard. (apps rely on this) */
   b->exact = true;
   nir_def *cmp = nir_bany(b, nir_flt_imm(b, src[0], 0.0));
   b->exact = false;

   nir_discard_if(b, cmp);
   b->shader->info.fs.uses_discard = true;
}

static void
get_texture_info(unsigned texture,
                 enum glsl_sampler_dim *dim,
                 bool *is_shadow,
                 bool *is_array)
{
   assert(is_array);
   *is_array = false;

   if (is_shadow)
      *is_shadow = false;

   switch (texture) {
   case TGSI_TEXTURE_BUFFER:
      *dim = GLSL_SAMPLER_DIM_BUF;
      break;
   case TGSI_TEXTURE_1D:
      *dim = GLSL_SAMPLER_DIM_1D;
      break;
   case TGSI_TEXTURE_1D_ARRAY:
      *dim = GLSL_SAMPLER_DIM_1D;
      *is_array = true;
      break;
   case TGSI_TEXTURE_SHADOW1D:
      *dim = GLSL_SAMPLER_DIM_1D;
      *is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      *dim = GLSL_SAMPLER_DIM_1D;
      *is_shadow = true;
      *is_array = true;
      break;
   case TGSI_TEXTURE_2D:
      *dim = GLSL_SAMPLER_DIM_2D;
      break;
   case TGSI_TEXTURE_2D_ARRAY:
      *dim = GLSL_SAMPLER_DIM_2D;
      *is_array = true;
      break;
   case TGSI_TEXTURE_2D_MSAA:
      *dim = GLSL_SAMPLER_DIM_MS;
      break;
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      *dim = GLSL_SAMPLER_DIM_MS;
      *is_array = true;
      break;
   case TGSI_TEXTURE_SHADOW2D:
      *dim = GLSL_SAMPLER_DIM_2D;
      *is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
      *dim = GLSL_SAMPLER_DIM_2D;
      *is_shadow = true;
      *is_array = true;
      break;
   case TGSI_TEXTURE_3D:
      *dim = GLSL_SAMPLER_DIM_3D;
      break;
   case TGSI_TEXTURE_CUBE:
      *dim = GLSL_SAMPLER_DIM_CUBE;
      break;
   case TGSI_TEXTURE_CUBE_ARRAY:
      *dim = GLSL_SAMPLER_DIM_CUBE;
      *is_array = true;
      break;
   case TGSI_TEXTURE_SHADOWCUBE:
      *dim = GLSL_SAMPLER_DIM_CUBE;
      *is_shadow = true;
      break;
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      *dim = GLSL_SAMPLER_DIM_CUBE;
      *is_shadow = true;
      *is_array = true;
      break;
   case TGSI_TEXTURE_RECT:
      *dim = GLSL_SAMPLER_DIM_RECT;
      break;
   case TGSI_TEXTURE_SHADOWRECT:
      *dim = GLSL_SAMPLER_DIM_RECT;
      *is_shadow = true;
      break;
   default:
      fprintf(stderr, "Unknown TGSI texture target %d\n", texture);
      abort();
   }
}

static enum glsl_base_type
base_type_for_alu_type(nir_alu_type type)
{
   type = nir_alu_type_get_base_type(type);

   switch (type) {
   case nir_type_float:
      return GLSL_TYPE_FLOAT;
   case nir_type_int:
      return GLSL_TYPE_INT;
   case nir_type_uint:
      return GLSL_TYPE_UINT;
   default:
      unreachable("invalid type");
   }
}

static nir_variable *
get_sampler_var(struct ttn_compile *c, int binding,
                enum glsl_sampler_dim dim,
                bool is_shadow,
                bool is_array,
                enum glsl_base_type base_type,
                nir_texop op)
{
   nir_variable *var = c->samplers[binding];
   if (!var) {
      const struct glsl_type *type =
         glsl_sampler_type(dim, is_shadow, is_array, base_type);
      var = nir_variable_create(c->build.shader, nir_var_uniform, type,
                                "sampler");
      var->data.binding = binding;
      var->data.explicit_binding = true;

      c->samplers[binding] = var;
      c->num_samplers = MAX2(c->num_samplers, binding + 1);

      /* Record textures used */
      BITSET_SET(c->build.shader->info.textures_used, binding);
      if (op == nir_texop_txf || op == nir_texop_txf_ms)
         BITSET_SET(c->build.shader->info.textures_used_by_txf, binding);
      BITSET_SET(c->build.shader->info.samplers_used, binding);
   }

   return var;
}

static nir_variable *
get_image_var(struct ttn_compile *c, int binding,
              enum glsl_sampler_dim dim,
              bool is_array,
              enum glsl_base_type base_type,
              enum gl_access_qualifier access,
              enum pipe_format format)
{
   nir_variable *var = c->images[binding];

   if (!var) {
      const struct glsl_type *type = glsl_image_type(dim, is_array, base_type);

      var = nir_variable_create(c->build.shader, nir_var_image, type, "image");
      var->data.binding = binding;
      var->data.explicit_binding = true;
      var->data.access = access;
      var->data.image.format = format;

      c->images[binding] = var;
      c->num_images = MAX2(c->num_images, binding + 1);
      if (dim == GLSL_SAMPLER_DIM_MS)
         c->num_msaa_images = c->num_images;
   }

   return var;
}

static void
add_ssbo_var(struct ttn_compile *c, int binding)
{
   nir_variable *var = c->ssbo[binding];

   if (!var) {
      /* A length of 0 is used to denote unsized arrays */
      const struct glsl_type *type = glsl_array_type(glsl_uint_type(), 0, 0);

      struct glsl_struct_field field = {
            .type = type,
            .name = "data",
            .location = -1,
      };

      var = nir_variable_create(c->build.shader, nir_var_mem_ssbo, type, "ssbo");
      var->data.binding = binding;
      var->interface_type =
         glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                             false, "data");
      c->ssbo[binding] = var;
   }
}

static nir_def *
ttn_tex(struct ttn_compile *c, nir_def **src)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   nir_tex_instr *instr;
   nir_texop op;
   unsigned num_srcs, samp = 1, sview, i;

   switch (tgsi_inst->Instruction.Opcode) {
   case TGSI_OPCODE_TEX:
      op = nir_texop_tex;
      num_srcs = 1;
      break;
   case TGSI_OPCODE_TEX2:
      op = nir_texop_tex;
      num_srcs = 1;
      samp = 2;
      break;
   case TGSI_OPCODE_TXP:
      op = nir_texop_tex;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXB:
      op = nir_texop_txb;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXB2:
      op = nir_texop_txb;
      num_srcs = 2;
      samp = 2;
      break;
   case TGSI_OPCODE_TXL:
   case TGSI_OPCODE_TEX_LZ:
      op = nir_texop_txl;
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXL2:
      op = nir_texop_txl;
      num_srcs = 2;
      samp = 2;
      break;
   case TGSI_OPCODE_TXF:
   case TGSI_OPCODE_TXF_LZ:
      if (tgsi_inst->Texture.Texture == TGSI_TEXTURE_2D_MSAA ||
          tgsi_inst->Texture.Texture == TGSI_TEXTURE_2D_ARRAY_MSAA) {
         op = nir_texop_txf_ms;
      } else {
         op = nir_texop_txf;
      }
      num_srcs = 2;
      break;
   case TGSI_OPCODE_TXD:
      op = nir_texop_txd;
      num_srcs = 3;
      samp = 3;
      break;
   case TGSI_OPCODE_LODQ:
      op = nir_texop_lod;
      num_srcs = 1;
      break;

   default:
      fprintf(stderr, "unknown TGSI tex op %d\n", tgsi_inst->Instruction.Opcode);
      abort();
   }

   if (tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW1D ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW1D_ARRAY ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW2D ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOW2D_ARRAY ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWRECT ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWCUBE ||
       tgsi_inst->Texture.Texture == TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
      num_srcs++;
   }

   /* Deref sources */
   num_srcs += 2;

   num_srcs += tgsi_inst->Texture.NumOffsets;

   instr = nir_tex_instr_create(b->shader, num_srcs);
   instr->op = op;

   get_texture_info(tgsi_inst->Texture.Texture,
                    &instr->sampler_dim, &instr->is_shadow, &instr->is_array);

   instr->coord_components =
      glsl_get_sampler_dim_coordinate_components(instr->sampler_dim);

   if (instr->is_array)
      instr->coord_components++;

   assert(tgsi_inst->Src[samp].Register.File == TGSI_FILE_SAMPLER);

   /* TODO if we supported any opc's which take an explicit SVIEW
    * src, we would use that here instead.  But for the "legacy"
    * texture opc's the SVIEW index is same as SAMP index:
    */
   sview = tgsi_inst->Src[samp].Register.Index;

   nir_alu_type sampler_type =
      sview < c->num_samp_types ? c->samp_types[sview] : nir_type_float32;

   if (op == nir_texop_lod) {
      instr->dest_type = nir_type_float32;
   } else {
      instr->dest_type = sampler_type;
   }

   nir_variable *var =
      get_sampler_var(c, sview, instr->sampler_dim,
                      instr->is_shadow,
                      instr->is_array,
                      base_type_for_alu_type(sampler_type),
                      op);

   nir_deref_instr *deref = nir_build_deref_var(b, var);

   unsigned src_number = 0;

   instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                                &deref->def);
   src_number++;
   instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref,
                                                &deref->def);
   src_number++;

   instr->src[src_number] =
      nir_tex_src_for_ssa(nir_tex_src_coord,
                          nir_trim_vector(b, src[0], instr->coord_components));
   src_number++;

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXP) {
      instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_projector,
                                                   ttn_channel(b, src[0], W));
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXB) {
      instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_bias,
                                                   ttn_channel(b, src[0], W));
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXB2) {
      instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_bias,
                                                   ttn_channel(b, src[1], X));
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXL ||
       tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TEX_LZ) {
      if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TEX_LZ)
         instr->src[src_number].src = nir_src_for_ssa(nir_imm_int(b, 0));
      else
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      instr->src[src_number].src_type = nir_tex_src_lod;
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXL2) {
      instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_lod,
                                                   ttn_channel(b, src[1], X));
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXF ||
       tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXF_LZ) {
      if (op == nir_texop_txf_ms) {
         instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_ms_index,
                                                      ttn_channel(b, src[0], W));
      } else {
         if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXF_LZ)
            instr->src[src_number].src = nir_src_for_ssa(nir_imm_int(b, 0));
         else
            instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
         instr->src[src_number].src_type = nir_tex_src_lod;
      }
      src_number++;
   }

   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_TXD) {
      instr->src[src_number] =
         nir_tex_src_for_ssa(nir_tex_src_ddx,
               nir_trim_vector(b, src[1], nir_tex_instr_src_size(instr, src_number)));
      src_number++;
      instr->src[src_number] =
         nir_tex_src_for_ssa(nir_tex_src_ddy,
               nir_trim_vector(b, src[2], nir_tex_instr_src_size(instr, src_number)));
      src_number++;
   }

   if (instr->is_shadow) {
      if (instr->coord_components == 4)
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[1], X));
      else if (instr->coord_components == 3)
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], W));
      else
         instr->src[src_number].src = nir_src_for_ssa(ttn_channel(b, src[0], Z));

      instr->src[src_number].src_type = nir_tex_src_comparator;
      src_number++;
   }

   for (i = 0; i < tgsi_inst->Texture.NumOffsets; i++) {
      struct tgsi_texture_offset *tex_offset = &tgsi_inst->TexOffsets[i];
      /* since TexOffset ins't using tgsi_full_src_register we get to
       * do some extra gymnastics:
       */
      nir_alu_src src;

      memset(&src, 0, sizeof(src));

      src.src = ttn_src_for_file_and_index(c,
                                           tex_offset->File,
                                           tex_offset->Index,
                                           NULL, NULL, NULL,
                                           true);

      src.swizzle[0] = tex_offset->SwizzleX;
      src.swizzle[1] = tex_offset->SwizzleY;
      src.swizzle[2] = tex_offset->SwizzleZ;
      src.swizzle[3] = TGSI_SWIZZLE_W;

      instr->src[src_number] = nir_tex_src_for_ssa(nir_tex_src_offset,
                                                   nir_mov_alu(b, src, nir_tex_instr_src_size(instr, src_number)));
      src_number++;
   }

   assert(src_number == num_srcs);
   assert(src_number == instr->num_srcs);

   nir_def_init(&instr->instr, &instr->def,
                nir_tex_instr_dest_size(instr), 32);
   nir_builder_instr_insert(b, &instr->instr);
   return nir_pad_vector_imm_int(b, &instr->def, 0, 4);
}

/* TGSI_OPCODE_TXQ is actually two distinct operations:
 *
 *     dst.x = texture\_width(unit, lod)
 *     dst.y = texture\_height(unit, lod)
 *     dst.z = texture\_depth(unit, lod)
 *     dst.w = texture\_levels(unit)
 *
 * dst.xyz map to NIR txs opcode, and dst.w maps to query_levels
 */
static nir_def *
ttn_txq(struct ttn_compile *c, nir_def **src)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   nir_tex_instr *txs, *qlv;

   txs = nir_tex_instr_create(b->shader, 2);
   txs->op = nir_texop_txs;
   txs->dest_type = nir_type_uint32;
   get_texture_info(tgsi_inst->Texture.Texture,
                    &txs->sampler_dim, &txs->is_shadow, &txs->is_array);

   qlv = nir_tex_instr_create(b->shader, 1);
   qlv->op = nir_texop_query_levels;
   qlv->dest_type = nir_type_uint32;
   get_texture_info(tgsi_inst->Texture.Texture,
                    &qlv->sampler_dim, &qlv->is_shadow, &qlv->is_array);

   assert(tgsi_inst->Src[1].Register.File == TGSI_FILE_SAMPLER);
   int sview = tgsi_inst->Src[1].Register.Index;

   nir_alu_type sampler_type =
      sview < c->num_samp_types ? c->samp_types[sview] : nir_type_float32;

   nir_variable *var =
      get_sampler_var(c, sview, txs->sampler_dim,
                      txs->is_shadow,
                      txs->is_array,
                      base_type_for_alu_type(sampler_type),
                      nir_texop_txs);

   nir_deref_instr *deref = nir_build_deref_var(b, var);

   txs->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                     &deref->def);

   qlv->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                     &deref->def);

   /* lod: */
   txs->src[1] = nir_tex_src_for_ssa(nir_tex_src_lod,
                                     ttn_channel(b, src[0], X));

   nir_def_init(&txs->instr, &txs->def, nir_tex_instr_dest_size(txs), 32);
   nir_builder_instr_insert(b, &txs->instr);

   nir_def_init(&qlv->instr, &qlv->def, 1, 32);
   nir_builder_instr_insert(b, &qlv->instr);

   return nir_vector_insert_imm(b,
                                nir_pad_vector_imm_int(b, &txs->def, 0, 4),
                                &qlv->def, 3);
}

static enum glsl_base_type
get_image_base_type(struct tgsi_full_instruction *tgsi_inst)
{
   const struct util_format_description *desc =
      util_format_description(tgsi_inst->Memory.Format);

   if (desc->channel[0].pure_integer) {
      if (desc->channel[0].type == UTIL_FORMAT_TYPE_SIGNED)
         return GLSL_TYPE_INT;
      else
         return GLSL_TYPE_UINT;
   }
   return GLSL_TYPE_FLOAT;
}

static enum gl_access_qualifier
get_mem_qualifier(struct tgsi_full_instruction *tgsi_inst)
{
   enum gl_access_qualifier access = 0;

   if (tgsi_inst->Memory.Qualifier & TGSI_MEMORY_COHERENT)
      access |= ACCESS_COHERENT;
   if (tgsi_inst->Memory.Qualifier & TGSI_MEMORY_RESTRICT)
      access |= ACCESS_RESTRICT;
   if (tgsi_inst->Memory.Qualifier & TGSI_MEMORY_VOLATILE)
      access |= ACCESS_VOLATILE;
   if (tgsi_inst->Memory.Qualifier & TGSI_MEMORY_STREAM_CACHE_POLICY)
      access |= ACCESS_NON_TEMPORAL;

   return access;
}

static nir_def *
ttn_mem(struct ttn_compile *c, nir_def **src)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   nir_intrinsic_instr *instr = NULL;
   unsigned resource_index, addr_src_index, file;

   switch (tgsi_inst->Instruction.Opcode) {
   case TGSI_OPCODE_LOAD:
      assert(!tgsi_inst->Src[0].Register.Indirect);
      resource_index = tgsi_inst->Src[0].Register.Index;
      file = tgsi_inst->Src[0].Register.File;
      addr_src_index = 1;
      break;
   case TGSI_OPCODE_STORE:
      assert(!tgsi_inst->Dst[0].Register.Indirect);
      resource_index = tgsi_inst->Dst[0].Register.Index;
      file = tgsi_inst->Dst[0].Register.File;
      addr_src_index = 0;
      break;
   default:
      unreachable("unexpected memory opcode");
   }

   if (file == TGSI_FILE_BUFFER) {
      nir_intrinsic_op op;

      switch (tgsi_inst->Instruction.Opcode) {
      case TGSI_OPCODE_LOAD:
         op = nir_intrinsic_load_ssbo;
         break;
      case TGSI_OPCODE_STORE:
         op = nir_intrinsic_store_ssbo;
         break;
      default:
         unreachable("unexpected buffer opcode");
      }

      add_ssbo_var(c, resource_index);

      instr = nir_intrinsic_instr_create(b->shader, op);
      instr->num_components = util_last_bit(tgsi_inst->Dst[0].Register.WriteMask);
      nir_intrinsic_set_access(instr, get_mem_qualifier(tgsi_inst));
      nir_intrinsic_set_align(instr, 4, 0);

      unsigned i = 0;
      if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_STORE)
         instr->src[i++] = nir_src_for_ssa(nir_swizzle(b, src[1], SWIZ(X, Y, Z, W),
                                                       instr->num_components));
      instr->src[i++] = nir_src_for_ssa(nir_imm_int(b, resource_index));
      instr->src[i++] = nir_src_for_ssa(ttn_channel(b, src[addr_src_index], X));

      if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_STORE)
         nir_intrinsic_set_write_mask(instr, tgsi_inst->Dst[0].Register.WriteMask);

   } else if (file == TGSI_FILE_IMAGE) {
      nir_intrinsic_op op;

      switch (tgsi_inst->Instruction.Opcode) {
      case TGSI_OPCODE_LOAD:
         op = nir_intrinsic_image_deref_load;
         break;
      case TGSI_OPCODE_STORE:
         op = nir_intrinsic_image_deref_store;
         break;
      default:
         unreachable("unexpected file opcode");
      }

      instr = nir_intrinsic_instr_create(b->shader, op);

      /* Set the image variable dereference. */
      enum glsl_sampler_dim dim;
      bool is_array;
      get_texture_info(tgsi_inst->Memory.Texture, &dim, NULL, &is_array);

      enum glsl_base_type base_type = get_image_base_type(tgsi_inst);
      enum gl_access_qualifier access = get_mem_qualifier(tgsi_inst);

      nir_variable *image =
         get_image_var(c, resource_index,
                       dim, is_array, base_type, access,
                       tgsi_inst->Memory.Format);
      nir_deref_instr *image_deref = nir_build_deref_var(b, image);
      const struct glsl_type *type = image_deref->type;

      nir_intrinsic_set_access(instr, image_deref->var->data.access);

      instr->src[0] = nir_src_for_ssa(&image_deref->def);
      instr->src[1] = nir_src_for_ssa(src[addr_src_index]);

      /* Set the sample argument, which is undefined for single-sample images. */
      if (glsl_get_sampler_dim(type) == GLSL_SAMPLER_DIM_MS) {
         instr->src[2] = nir_src_for_ssa(ttn_channel(b, src[addr_src_index], W));
      } else {
         instr->src[2] = nir_src_for_ssa(nir_undef(b, 1, 32));
      }

      if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_LOAD) {
         instr->src[3] = nir_src_for_ssa(nir_imm_int(b, 0)); /* LOD */
      }

      unsigned num_components = util_last_bit(tgsi_inst->Dst[0].Register.WriteMask);

      if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_STORE) {
         instr->src[3] = nir_src_for_ssa(nir_swizzle(b, src[1], SWIZ(X, Y, Z, W),
                                                     num_components));
         instr->src[4] = nir_src_for_ssa(nir_imm_int(b, 0)); /* LOD */
      }

      instr->num_components = num_components;
   } else {
      unreachable("unexpected file");
   }


   if (tgsi_inst->Instruction.Opcode == TGSI_OPCODE_LOAD) {
      nir_def_init(&instr->instr, &instr->def, instr->num_components, 32);
      nir_builder_instr_insert(b, &instr->instr);
      return nir_pad_vector_imm_int(b, &instr->def, 0, 4);
   } else {
      nir_builder_instr_insert(b, &instr->instr);
      return NULL;
   }
}

static const nir_op op_trans[TGSI_OPCODE_LAST] = {
   [TGSI_OPCODE_ARL] = 0,
   [TGSI_OPCODE_MOV] = nir_op_mov,
   [TGSI_OPCODE_FBFETCH] = nir_op_mov,
   [TGSI_OPCODE_LIT] = 0,
   [TGSI_OPCODE_RCP] = nir_op_frcp,
   [TGSI_OPCODE_RSQ] = nir_op_frsq,
   [TGSI_OPCODE_EXP] = 0,
   [TGSI_OPCODE_LOG] = 0,
   [TGSI_OPCODE_MUL] = nir_op_fmul,
   [TGSI_OPCODE_ADD] = nir_op_fadd,
   [TGSI_OPCODE_DP3] = 0,
   [TGSI_OPCODE_DP4] = 0,
   [TGSI_OPCODE_DST] = 0,
   [TGSI_OPCODE_MIN] = nir_op_fmin,
   [TGSI_OPCODE_MAX] = nir_op_fmax,
   [TGSI_OPCODE_SLT] = nir_op_slt,
   [TGSI_OPCODE_SGE] = nir_op_sge,
   [TGSI_OPCODE_MAD] = nir_op_ffma,
   [TGSI_OPCODE_TEX_LZ] = 0,
   [TGSI_OPCODE_LRP] = 0,
   [TGSI_OPCODE_SQRT] = nir_op_fsqrt,
   [TGSI_OPCODE_FRC] = nir_op_ffract,
   [TGSI_OPCODE_TXF_LZ] = 0,
   [TGSI_OPCODE_FLR] = nir_op_ffloor,
   [TGSI_OPCODE_ROUND] = nir_op_fround_even,
   [TGSI_OPCODE_EX2] = nir_op_fexp2,
   [TGSI_OPCODE_LG2] = nir_op_flog2,
   [TGSI_OPCODE_POW] = nir_op_fpow,
   [TGSI_OPCODE_COS] = nir_op_fcos,
   [TGSI_OPCODE_DDX] = nir_op_fddx,
   [TGSI_OPCODE_DDY] = nir_op_fddy,
   [TGSI_OPCODE_KILL] = 0,
   [TGSI_OPCODE_PK2H] = 0, /* XXX */
   [TGSI_OPCODE_PK2US] = 0, /* XXX */
   [TGSI_OPCODE_PK4B] = 0, /* XXX */
   [TGSI_OPCODE_PK4UB] = 0, /* XXX */
   [TGSI_OPCODE_SEQ] = nir_op_seq,
   [TGSI_OPCODE_SGT] = 0,
   [TGSI_OPCODE_SIN] = nir_op_fsin,
   [TGSI_OPCODE_SNE] = nir_op_sne,
   [TGSI_OPCODE_SLE] = 0,
   [TGSI_OPCODE_TEX] = 0,
   [TGSI_OPCODE_TXD] = 0,
   [TGSI_OPCODE_TXP] = 0,
   [TGSI_OPCODE_UP2H] = 0, /* XXX */
   [TGSI_OPCODE_UP2US] = 0, /* XXX */
   [TGSI_OPCODE_UP4B] = 0, /* XXX */
   [TGSI_OPCODE_UP4UB] = 0, /* XXX */
   [TGSI_OPCODE_ARR] = 0,

   /* No function calls, yet. */
   [TGSI_OPCODE_CAL] = 0, /* XXX */
   [TGSI_OPCODE_RET] = 0, /* XXX */

   [TGSI_OPCODE_SSG] = nir_op_fsign,
   [TGSI_OPCODE_CMP] = 0,
   [TGSI_OPCODE_TXB] = 0,
   [TGSI_OPCODE_DIV] = nir_op_fdiv,
   [TGSI_OPCODE_DP2] = 0,
   [TGSI_OPCODE_TXL] = 0,

   [TGSI_OPCODE_BRK] = 0,
   [TGSI_OPCODE_IF] = 0,
   [TGSI_OPCODE_UIF] = 0,
   [TGSI_OPCODE_ELSE] = 0,
   [TGSI_OPCODE_ENDIF] = 0,

   [TGSI_OPCODE_DDX_FINE] = nir_op_fddx_fine,
   [TGSI_OPCODE_DDY_FINE] = nir_op_fddy_fine,

   [TGSI_OPCODE_CEIL] = nir_op_fceil,
   [TGSI_OPCODE_I2F] = nir_op_i2f32,
   [TGSI_OPCODE_NOT] = nir_op_inot,
   [TGSI_OPCODE_TRUNC] = nir_op_ftrunc,
   [TGSI_OPCODE_SHL] = nir_op_ishl,
   [TGSI_OPCODE_AND] = nir_op_iand,
   [TGSI_OPCODE_OR] = nir_op_ior,
   [TGSI_OPCODE_MOD] = nir_op_umod,
   [TGSI_OPCODE_XOR] = nir_op_ixor,
   [TGSI_OPCODE_TXF] = 0,
   [TGSI_OPCODE_TXQ] = 0,

   [TGSI_OPCODE_CONT] = 0,

   [TGSI_OPCODE_EMIT] = 0, /* XXX */
   [TGSI_OPCODE_ENDPRIM] = 0, /* XXX */

   [TGSI_OPCODE_BGNLOOP] = 0,
   [TGSI_OPCODE_BGNSUB] = 0, /* XXX: no function calls */
   [TGSI_OPCODE_ENDLOOP] = 0,
   [TGSI_OPCODE_ENDSUB] = 0, /* XXX: no function calls */

   [TGSI_OPCODE_NOP] = 0,
   [TGSI_OPCODE_FSEQ] = nir_op_feq,
   [TGSI_OPCODE_FSGE] = nir_op_fge,
   [TGSI_OPCODE_FSLT] = nir_op_flt,
   [TGSI_OPCODE_FSNE] = nir_op_fneu,

   [TGSI_OPCODE_KILL_IF] = 0,

   [TGSI_OPCODE_END] = 0,

   [TGSI_OPCODE_F2I] = nir_op_f2i32,
   [TGSI_OPCODE_IDIV] = nir_op_idiv,
   [TGSI_OPCODE_IMAX] = nir_op_imax,
   [TGSI_OPCODE_IMIN] = nir_op_imin,
   [TGSI_OPCODE_INEG] = nir_op_ineg,
   [TGSI_OPCODE_ISGE] = nir_op_ige,
   [TGSI_OPCODE_ISHR] = nir_op_ishr,
   [TGSI_OPCODE_ISLT] = nir_op_ilt,
   [TGSI_OPCODE_F2U] = nir_op_f2u32,
   [TGSI_OPCODE_U2F] = nir_op_u2f32,
   [TGSI_OPCODE_UADD] = nir_op_iadd,
   [TGSI_OPCODE_UDIV] = nir_op_udiv,
   [TGSI_OPCODE_UMAD] = 0,
   [TGSI_OPCODE_UMAX] = nir_op_umax,
   [TGSI_OPCODE_UMIN] = nir_op_umin,
   [TGSI_OPCODE_UMOD] = nir_op_umod,
   [TGSI_OPCODE_UMUL] = nir_op_imul,
   [TGSI_OPCODE_USEQ] = nir_op_ieq,
   [TGSI_OPCODE_USGE] = nir_op_uge,
   [TGSI_OPCODE_USHR] = nir_op_ushr,
   [TGSI_OPCODE_USLT] = nir_op_ult,
   [TGSI_OPCODE_USNE] = nir_op_ine,

   [TGSI_OPCODE_SWITCH] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_CASE] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_DEFAULT] = 0, /* not emitted by glsl_to_tgsi.cpp */
   [TGSI_OPCODE_ENDSWITCH] = 0, /* not emitted by glsl_to_tgsi.cpp */

   /* XXX: SAMPLE opcodes */

   [TGSI_OPCODE_UARL] = nir_op_mov,
   [TGSI_OPCODE_UCMP] = 0,
   [TGSI_OPCODE_IABS] = nir_op_iabs,
   [TGSI_OPCODE_ISSG] = nir_op_isign,

   [TGSI_OPCODE_LOAD] = 0,
   [TGSI_OPCODE_STORE] = 0,

   /* XXX: atomics */

   [TGSI_OPCODE_TEX2] = 0,
   [TGSI_OPCODE_TXB2] = 0,
   [TGSI_OPCODE_TXL2] = 0,

   [TGSI_OPCODE_IMUL_HI] = nir_op_imul_high,
   [TGSI_OPCODE_UMUL_HI] = nir_op_umul_high,

   [TGSI_OPCODE_TG4] = 0,
   [TGSI_OPCODE_LODQ] = 0,

   [TGSI_OPCODE_IBFE] = nir_op_ibitfield_extract,
   [TGSI_OPCODE_UBFE] = nir_op_ubitfield_extract,
   [TGSI_OPCODE_BFI] = nir_op_bitfield_insert,
   [TGSI_OPCODE_BREV] = nir_op_bitfield_reverse,
   [TGSI_OPCODE_POPC] = nir_op_bit_count,
   [TGSI_OPCODE_LSB] = nir_op_find_lsb,
   [TGSI_OPCODE_IMSB] = nir_op_ifind_msb,
   [TGSI_OPCODE_UMSB] = nir_op_ufind_msb,

   [TGSI_OPCODE_INTERP_CENTROID] = 0, /* XXX */
   [TGSI_OPCODE_INTERP_SAMPLE] = 0, /* XXX */
   [TGSI_OPCODE_INTERP_OFFSET] = 0, /* XXX */

   [TGSI_OPCODE_F2D] = nir_op_f2f64,
   [TGSI_OPCODE_D2F] = nir_op_f2f32,
   [TGSI_OPCODE_DMUL] = nir_op_fmul,
   [TGSI_OPCODE_D2U] = nir_op_f2u32,
   [TGSI_OPCODE_U2D] = nir_op_u2f64,

   [TGSI_OPCODE_U64ADD] = nir_op_iadd,
   [TGSI_OPCODE_U64MUL] = nir_op_imul,
   [TGSI_OPCODE_U64DIV] = nir_op_udiv,
   [TGSI_OPCODE_U64SNE] = nir_op_ine,
   [TGSI_OPCODE_I64NEG] = nir_op_ineg,
   [TGSI_OPCODE_I64ABS] = nir_op_iabs,
};

static void
ttn_emit_instruction(struct ttn_compile *c)
{
   nir_builder *b = &c->build;
   struct tgsi_full_instruction *tgsi_inst = &c->token->FullInstruction;
   unsigned i;
   unsigned tgsi_op = tgsi_inst->Instruction.Opcode;
   struct tgsi_full_dst_register *tgsi_dst = &tgsi_inst->Dst[0];

   if (tgsi_op == TGSI_OPCODE_END)
      return;

   nir_def *src[TGSI_FULL_MAX_SRC_REGISTERS];
   for (i = 0; i < tgsi_inst->Instruction.NumSrcRegs; i++) {
      src[i] = ttn_get_src(c, &tgsi_inst->Src[i], i);
   }

   unsigned tgsi_dst_type = tgsi_opcode_infer_dst_type(tgsi_op, 0);

   /* The destination bitsize of the NIR opcode (not TGSI, where it's always
    * 32 bits). This needs to be passed into ttn_alu() because it can't be
    * inferred for comparison opcodes.
    */
   unsigned dst_bitsize = tgsi_type_is_64bit(tgsi_dst_type) ? 64 : 32;

   /* If this is non-NULL after the switch, it will be written to the
    * corresponding register/variable/etc after.
    */
   nir_def *dst = NULL;

   switch (tgsi_op) {
   case TGSI_OPCODE_RSQ:
      dst = nir_frsq(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_SQRT:
      dst = nir_fsqrt(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_RCP:
      dst = nir_frcp(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_EX2:
      dst = nir_fexp2(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_LG2:
      dst = nir_flog2(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_POW:
      dst = nir_fpow(b, ttn_channel(b, src[0], X), ttn_channel(b, src[1], X));
      break;

   case TGSI_OPCODE_COS:
      dst = nir_fcos(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_SIN:
      dst = nir_fsin(b, ttn_channel(b, src[0], X));
      break;

   case TGSI_OPCODE_ARL:
      dst = nir_f2i32(b, nir_ffloor(b, src[0]));
      break;

   case TGSI_OPCODE_EXP:
      dst = ttn_exp(b, src);
      break;

   case TGSI_OPCODE_LOG:
      dst = ttn_log(b, src);
      break;

   case TGSI_OPCODE_DST:
      dst = ttn_dst(b, src);
      break;

   case TGSI_OPCODE_LIT:
      dst = ttn_lit(b, src);
      break;

   case TGSI_OPCODE_DP2:
      dst = nir_fdot2(b, src[0], src[1]);
      break;

   case TGSI_OPCODE_DP3:
      dst = nir_fdot3(b, src[0], src[1]);
      break;

   case TGSI_OPCODE_DP4:
      dst = nir_fdot4(b, src[0], src[1]);
      break;

   case TGSI_OPCODE_UMAD:
      dst = nir_iadd(b, nir_imul(b, src[0], src[1]), src[2]);
      break;

   case TGSI_OPCODE_LRP:
      dst = nir_flrp(b, src[2], src[1], src[0]);
      break;

   case TGSI_OPCODE_KILL:
      ttn_kill(b);
      break;

   case TGSI_OPCODE_ARR:
      dst = nir_f2i32(b, nir_fround_even(b, src[0]));
      break;

   case TGSI_OPCODE_CMP:
      dst = nir_bcsel(b, nir_flt(b, src[0], nir_imm_float(b, 0.0)),
                      src[1], src[2]);
      break;

   case TGSI_OPCODE_UCMP:
      dst = nir_bcsel(b, nir_ine(b, src[0], nir_imm_int(b, 0)),
                      src[1], src[2]);
      break;

   case TGSI_OPCODE_SGT:
      dst = nir_slt(b, src[1], src[0]);
      break;

   case TGSI_OPCODE_SLE:
      dst = nir_sge(b, src[1], src[0]);
      break;

   case TGSI_OPCODE_KILL_IF:
      ttn_kill_if(b, src);
      break;

   case TGSI_OPCODE_TEX:
   case TGSI_OPCODE_TEX_LZ:
   case TGSI_OPCODE_TXP:
   case TGSI_OPCODE_TXL:
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXD:
   case TGSI_OPCODE_TEX2:
   case TGSI_OPCODE_TXL2:
   case TGSI_OPCODE_TXB2:
   case TGSI_OPCODE_TXF:
   case TGSI_OPCODE_TXF_LZ:
   case TGSI_OPCODE_TG4:
   case TGSI_OPCODE_LODQ:
      dst = ttn_tex(c, src);
      break;

   case TGSI_OPCODE_TXQ:
      dst = ttn_txq(c, src);
      break;

   case TGSI_OPCODE_LOAD:
   case TGSI_OPCODE_STORE:
      dst = ttn_mem(c, src);
      break;

   case TGSI_OPCODE_NOP:
      break;

   case TGSI_OPCODE_IF:
      nir_push_if(b, nir_fneu_imm(b, nir_channel(b, src[0], 0), 0.0));
      break;

   case TGSI_OPCODE_UIF:
      nir_push_if(b, nir_ine_imm(b, nir_channel(b, src[0], 0), 0));
      break;

   case TGSI_OPCODE_ELSE:
      nir_push_else(&c->build, NULL);
      break;

   case TGSI_OPCODE_ENDIF:
      nir_pop_if(&c->build, NULL);
      break;

   case TGSI_OPCODE_BGNLOOP:
      nir_push_loop(&c->build);
      break;

   case TGSI_OPCODE_BRK:
      nir_jump(b, nir_jump_break);
      break;

   case TGSI_OPCODE_CONT:
      nir_jump(b, nir_jump_continue);
      break;

   case TGSI_OPCODE_ENDLOOP:
      nir_pop_loop(&c->build, NULL);
      break;

   case TGSI_OPCODE_BARRIER:
      ttn_barrier(b);
      break;

   default:
      if (op_trans[tgsi_op] != 0 || tgsi_op == TGSI_OPCODE_MOV) {
         dst = ttn_alu(b, op_trans[tgsi_op], dst_bitsize, src);
      } else {
         fprintf(stderr, "unknown TGSI opcode: %s\n",
                 tgsi_get_opcode_name(tgsi_op));
         abort();
      }
      break;
   }

   if (dst == NULL)
      return;

   if (tgsi_inst->Instruction.Saturate)
      dst = nir_fsat(b, dst);

   if (dst->num_components == 1)
      dst = nir_replicate(b, dst, 4);
   else if (dst->num_components == 2)
      dst = nir_pad_vector_imm_int(b, dst, 0, 4); /* for 64->32 conversions */

   assert(dst->num_components == 4);

   /* Finally, copy the SSA def to the NIR variable/register */
   nir_variable *var = ttn_get_var(c, tgsi_dst);
   if (var) {
      unsigned index = tgsi_dst->Register.Index;
      unsigned offset = c->temp_regs[index].offset;
      struct tgsi_ind_register *indirect = tgsi_dst->Register.Indirect ?
                                           &tgsi_dst->Indirect : NULL;
      nir_store_deref(b, ttn_array_deref(c, var, offset, indirect), dst,
                      tgsi_dst->Register.WriteMask);
   } else {
      unsigned index = tgsi_dst->Register.Index;
      nir_def *reg = NULL;
      unsigned base_offset = 0;

      if (tgsi_dst->Register.File == TGSI_FILE_TEMPORARY) {
         assert(!c->temp_regs[index].var && "handled above");
         assert(!tgsi_dst->Register.Indirect);

         reg = c->temp_regs[index].reg;
         base_offset = c->temp_regs[index].offset;
      } else if (tgsi_dst->Register.File == TGSI_FILE_OUTPUT) {
         reg = c->output_regs[index].reg;
         base_offset = c->output_regs[index].offset;
      } else if (tgsi_dst->Register.File == TGSI_FILE_ADDRESS) {
         assert(index == 0);
         reg = c->addr_reg;
      }

      if (tgsi_dst->Register.Indirect) {
         nir_def *indirect = ttn_src_for_indirect(c, &tgsi_dst->Indirect);
         nir_store_reg_indirect(b, dst, reg, indirect, .base = base_offset,
                                .write_mask = tgsi_dst->Register.WriteMask);
      } else {
         nir_build_store_reg(b, dst, reg, .base = base_offset,
                             .write_mask = tgsi_dst->Register.WriteMask);
      }
   }
}

/**
 * Puts a NIR intrinsic to store of each TGSI_FILE_OUTPUT value to the output
 * variables at the end of the shader.
 *
 * We don't generate these incrementally as the TGSI_FILE_OUTPUT values are
 * written, because there's no output load intrinsic, which means we couldn't
 * handle writemasks.
 */
static void
ttn_add_output_stores(struct ttn_compile *c)
{
   nir_builder *b = &c->build;

   for (int i = 0; i < c->build.shader->num_outputs; i++) {
      nir_variable *var = c->outputs[i];
      if (!var)
         continue;

      nir_def *store_value =
         nir_build_load_reg(b, 4, 32, c->output_regs[i].reg,
                            .base = c->output_regs[i].offset);

      uint32_t store_mask = BITFIELD_MASK(store_value->num_components);
      if (c->build.shader->info.stage == MESA_SHADER_FRAGMENT) {
         /* TGSI uses TGSI_SEMANTIC_POSITION.z for the depth output
          * and TGSI_SEMANTIC_STENCIL.y for the stencil output,
          * while NIR uses a single-component output.
          */
         if (var->data.location == FRAG_RESULT_DEPTH)
            store_value = nir_channel(b, store_value, 2);
         else if (var->data.location == FRAG_RESULT_STENCIL)
            store_value = nir_channel(b, store_value, 1);
         else if (var->data.location == FRAG_RESULT_SAMPLE_MASK)
            store_value = nir_channel(b, store_value, 0);
      } else {
         /* FOGC, LAYER, and PSIZ are scalar values */
         if (var->data.location == VARYING_SLOT_FOGC ||
             var->data.location == VARYING_SLOT_LAYER ||
             var->data.location == VARYING_SLOT_PSIZ) {
            store_value = nir_channel(b, store_value, 0);
         }
         if (var->data.location == VARYING_SLOT_CLIP_DIST0)
            store_mask = BITFIELD_MASK(MIN2(c->build.shader->info.clip_distance_array_size, 4));
         else if (var->data.location == VARYING_SLOT_CLIP_DIST1) {
            if (c->build.shader->info.clip_distance_array_size > 4)
               store_mask = BITFIELD_MASK(c->build.shader->info.clip_distance_array_size - 4);
            else
               store_mask = 0;
         }
      }

      if (c->cap_compact_arrays &&
          (var->data.location == VARYING_SLOT_CLIP_DIST0 ||
           var->data.location == VARYING_SLOT_CLIP_DIST1)) {
         if (!store_mask)
            continue;

         nir_deref_instr *deref = nir_build_deref_var(b, c->clipdist);
         nir_def *zero = nir_imm_zero(b, 1, 32);
         unsigned offset = var->data.location == VARYING_SLOT_CLIP_DIST1 ? 4 : 0;
         unsigned size = var->data.location == VARYING_SLOT_CLIP_DIST1 ?
                          b->shader->info.clip_distance_array_size :
                          MIN2(4, b->shader->info.clip_distance_array_size);
         for (unsigned i = offset; i < size; i++) {
            /* deref the array member and store each component */
            nir_deref_instr *component_deref = nir_build_deref_array_imm(b, deref, i);
            nir_def *val = zero;
            if (store_mask & BITFIELD_BIT(i - offset))
               val = nir_channel(b, store_value, i - offset);
            nir_store_deref(b, component_deref, val, 0x1);
         }
      } else {
         nir_store_deref(b, nir_build_deref_var(b, var), store_value, store_mask);
      }
   }
}

/**
 * Parses the given TGSI tokens.
 */
static void
ttn_parse_tgsi(struct ttn_compile *c, const void *tgsi_tokens)
{
   struct tgsi_parse_context parser;
   ASSERTED int ret;

   ret = tgsi_parse_init(&parser, tgsi_tokens);
   assert(ret == TGSI_PARSE_OK);

   while (!tgsi_parse_end_of_tokens(&parser)) {
      tgsi_parse_token(&parser);
      c->token = &parser.FullToken;

      switch (parser.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_DECLARATION:
         ttn_emit_declaration(c);
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:
         ttn_emit_instruction(c);
         break;

      case TGSI_TOKEN_TYPE_IMMEDIATE:
         ttn_emit_immediate(c);
         break;
      }
   }

   tgsi_parse_free(&parser);
}

static void
ttn_read_pipe_caps(struct ttn_compile *c,
                   struct pipe_screen *screen)
{
   c->cap_samplers_as_deref = screen->get_param(screen, PIPE_CAP_NIR_SAMPLERS_AS_DEREF);
   c->cap_face_is_sysval = screen->get_param(screen, PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL);
   c->cap_position_is_sysval = screen->get_param(screen, PIPE_CAP_FS_POSITION_IS_SYSVAL);
   c->cap_point_is_sysval = screen->get_param(screen, PIPE_CAP_FS_POINT_IS_SYSVAL);
   c->cap_integers = screen->get_shader_param(screen, c->scan->processor, PIPE_SHADER_CAP_INTEGERS);
   c->cap_compact_arrays = screen->get_param(screen, PIPE_CAP_NIR_COMPACT_ARRAYS);
}

#define BITSET_SET32(bitset, u32_mask) do { \
   STATIC_ASSERT(sizeof((bitset)[0]) >= sizeof(u32_mask)); \
   BITSET_ZERO(bitset); \
   (bitset)[0] = (u32_mask); \
} while (0)

/**
 * Initializes a TGSI-to-NIR compiler.
 */
static struct ttn_compile *
ttn_compile_init(const void *tgsi_tokens,
                 const nir_shader_compiler_options *options,
                 struct pipe_screen *screen)
{
   struct ttn_compile *c;
   struct nir_shader *s;
   struct tgsi_shader_info scan;

   assert(options || screen);
   c = rzalloc(NULL, struct ttn_compile);

   tgsi_scan_shader(tgsi_tokens, &scan);
   c->scan = &scan;

   if (!options) {
      options =
         screen->get_compiler_options(screen, PIPE_SHADER_IR_NIR, scan.processor);
   }

   c->build = nir_builder_init_simple_shader(tgsi_processor_to_shader_stage(scan.processor),
                                             options, "TTN");

   s = c->build.shader;

   if (screen) {
      ttn_read_pipe_caps(c, screen);
   } else {
      /* TTN used to be hard coded to always make FACE a sysval,
       * so it makes sense to preserve that behavior so users don't break. */
      c->cap_face_is_sysval = true;
   }

   s->info.subgroup_size = SUBGROUP_SIZE_UNIFORM;

   if (s->info.stage == MESA_SHADER_FRAGMENT)
      s->info.fs.untyped_color_outputs = true;

   s->num_inputs = scan.file_max[TGSI_FILE_INPUT] + 1;
   s->num_uniforms = scan.const_file_max[0] + 1;
   s->num_outputs = scan.file_max[TGSI_FILE_OUTPUT] + 1;
   s->info.num_ssbos = util_last_bit(scan.shader_buffers_declared);
   s->info.num_ubos = util_last_bit(scan.const_buffers_declared >> 1);
   s->info.num_images = util_last_bit(scan.images_declared);
   BITSET_SET32(s->info.images_used, scan.images_declared);
   BITSET_SET32(s->info.image_buffers, scan.images_buffers);
   BITSET_SET32(s->info.msaa_images, scan.msaa_images_declared);
   s->info.num_textures = util_last_bit(scan.samplers_declared);
   BITSET_SET32(s->info.textures_used, scan.samplers_declared);
   BITSET_ZERO(s->info.textures_used_by_txf); /* No scan information yet */
   BITSET_SET32(s->info.samplers_used, scan.samplers_declared);
   s->info.internal = false;

   /* Default for TGSI is separate, this is assumed throughout the tree */
   s->info.separate_shader = true;

   for (unsigned i = 0; i < TGSI_PROPERTY_COUNT; i++) {
      unsigned value = scan.properties[i];

      switch (i) {
      case TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS:
         break; /* handled in ttn_emit_declaration */
      case TGSI_PROPERTY_FS_COORD_ORIGIN:
         if (s->info.stage == MESA_SHADER_FRAGMENT)
            s->info.fs.origin_upper_left = value == TGSI_FS_COORD_ORIGIN_UPPER_LEFT;
         break;
      case TGSI_PROPERTY_FS_COORD_PIXEL_CENTER:
         if (s->info.stage == MESA_SHADER_FRAGMENT)
            s->info.fs.pixel_center_integer = value == TGSI_FS_COORD_PIXEL_CENTER_INTEGER;
         break;
      case TGSI_PROPERTY_FS_DEPTH_LAYOUT:
         if (s->info.stage == MESA_SHADER_FRAGMENT)
            s->info.fs.depth_layout = ttn_get_depth_layout(value);
         break;
      case TGSI_PROPERTY_VS_WINDOW_SPACE_POSITION:
         if (s->info.stage == MESA_SHADER_VERTEX)
            s->info.vs.window_space_position = value;
         break;
      case TGSI_PROPERTY_NEXT_SHADER:
         s->info.next_stage = tgsi_processor_to_shader_stage(value);
         break;
      case TGSI_PROPERTY_VS_BLIT_SGPRS_AMD:
         if (s->info.stage == MESA_SHADER_VERTEX)
            s->info.vs.blit_sgprs_amd = value;
         break;
      case TGSI_PROPERTY_CS_FIXED_BLOCK_WIDTH:
         if (s->info.stage == MESA_SHADER_COMPUTE)
            s->info.workgroup_size[0] = value;
         break;
      case TGSI_PROPERTY_CS_FIXED_BLOCK_HEIGHT:
         if (s->info.stage == MESA_SHADER_COMPUTE)
            s->info.workgroup_size[1] = value;
         break;
      case TGSI_PROPERTY_CS_FIXED_BLOCK_DEPTH:
         if (s->info.stage == MESA_SHADER_COMPUTE)
            s->info.workgroup_size[2] = value;
         break;
      case TGSI_PROPERTY_CS_USER_DATA_COMPONENTS_AMD:
         if (s->info.stage == MESA_SHADER_COMPUTE)
            s->info.cs.user_data_components_amd = value;
         break;
      case TGSI_PROPERTY_NUM_CLIPDIST_ENABLED:
         s->info.clip_distance_array_size = value;
         break;
      case TGSI_PROPERTY_LEGACY_MATH_RULES:
         s->info.use_legacy_math_rules = value;
         break;
      default:
         if (value) {
            fprintf(stderr, "tgsi_to_nir: unhandled TGSI property %u = %u\n",
                    i, value);
            unreachable("unhandled TGSI property");
         }
      }
   }

   if (s->info.stage == MESA_SHADER_COMPUTE &&
       (!s->info.workgroup_size[0] ||
        !s->info.workgroup_size[1] ||
        !s->info.workgroup_size[2]))
      s->info.workgroup_size_variable = true;

   c->inputs = rzalloc_array(c, struct nir_variable *, s->num_inputs);
   c->outputs = rzalloc_array(c, struct nir_variable *, s->num_outputs);

   c->output_regs = rzalloc_array(c, struct ttn_reg_info,
                                  scan.file_max[TGSI_FILE_OUTPUT] + 1);
   c->temp_regs = rzalloc_array(c, struct ttn_reg_info,
                                scan.file_max[TGSI_FILE_TEMPORARY] + 1);
   c->imm_defs = rzalloc_array(c, nir_def *,
                               scan.file_max[TGSI_FILE_IMMEDIATE] + 1);

   c->num_samp_types = scan.file_max[TGSI_FILE_SAMPLER_VIEW] + 1;
   c->samp_types = rzalloc_array(c, nir_alu_type, c->num_samp_types);

   ttn_parse_tgsi(c, tgsi_tokens);
   ttn_add_output_stores(c);

   nir_validate_shader(c->build.shader, "TTN: after parsing TGSI and creating the NIR shader");

   return c;
}

static void
ttn_optimize_nir(nir_shader *nir)
{
   bool progress;

   do {
      progress = false;

      NIR_PASS_V(nir, nir_lower_vars_to_ssa);

      /* Linking deals with unused inputs/outputs, but here we can remove
       * things local to the shader in the hopes that we can cleanup other
       * things. This pass will also remove variables with only stores, so we
       * might be able to make progress after it.
       */
      NIR_PASS(progress, nir, nir_remove_dead_variables,
               nir_var_function_temp | nir_var_shader_temp |
               nir_var_mem_shared,
               NULL);

      NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
      NIR_PASS(progress, nir, nir_opt_dead_write_vars);

      if (nir->options->lower_to_scalar) {
         NIR_PASS_V(nir, nir_lower_alu_to_scalar,
                    nir->options->lower_to_scalar_filter, NULL);
         NIR_PASS_V(nir, nir_lower_phis_to_scalar, false);
      }

      NIR_PASS_V(nir, nir_lower_alu);
      NIR_PASS_V(nir, nir_lower_pack);
      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_opt_dce);
      if (nir_opt_loop(nir)) {
         progress = true;
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_dce);
      }
      NIR_PASS(progress, nir, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_peephole_select, 8, true, true);

      NIR_PASS(progress, nir, nir_opt_phi_precision);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);

      if (!nir->info.flrp_lowered) {
         unsigned lower_flrp =
            (nir->options->lower_flrp16 ? 16 : 0) |
            (nir->options->lower_flrp32 ? 32 : 0) |
            (nir->options->lower_flrp64 ? 64 : 0);

         if (lower_flrp) {
            bool lower_flrp_progress = false;

            NIR_PASS(lower_flrp_progress, nir, nir_lower_flrp,
                     lower_flrp,
                     false /* always_precise */);
            if (lower_flrp_progress) {
               NIR_PASS(progress, nir,
                        nir_opt_constant_folding);
               progress = true;
            }
         }

         /* Nothing should rematerialize any flrps, so we only need to do this
          * lowering once.
          */
         nir->info.flrp_lowered = true;
      }

      NIR_PASS(progress, nir, nir_opt_undef);
      NIR_PASS(progress, nir, nir_opt_conditional_discard);
      if (nir->options->max_unroll_iterations) {
         NIR_PASS(progress, nir, nir_opt_loop_unroll);
      }
   } while (progress);
}

static bool
lower_clipdistance_to_array(nir_shader *nir)
{
   bool progress = false;
   nir_variable *dist0 = nir_find_variable_with_location(nir, nir_var_shader_out, VARYING_SLOT_CLIP_DIST0);
   nir_variable *dist1 = nir_find_variable_with_location(nir, nir_var_shader_out, VARYING_SLOT_CLIP_DIST1);
   /* resize VARYING_SLOT_CLIP_DIST0 to the full array size */
   dist0->type = glsl_array_type(glsl_float_type(), nir->info.clip_distance_array_size, sizeof(float));
   struct set *deletes = _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
   nir_foreach_function_impl(impl, nir) {
      bool func_progress = false;
      nir_builder b = nir_builder_at(nir_before_impl(impl));
      /* create a new deref for the arrayed clipdistance variable at the start of the function */
      nir_deref_instr *clipdist_deref = nir_build_deref_var(&b, dist0);
      nir_def *zero = nir_imm_zero(&b, 1, 32);
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            /* filter through until a clipdistance store is reached */
            if (instr->type != nir_instr_type_intrinsic)
               continue;
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_store_deref)
               continue;
            nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
            nir_variable *var = nir_deref_instr_get_variable(deref);
            if (var != dist0 && (!dist1 || var != dist1))
               continue;
            b.cursor = nir_before_instr(instr);
            uint32_t wrmask = nir_intrinsic_write_mask(intr);
            unsigned offset = var == dist1 ? 4 : 0;
            /* iterate over the store's writemask for components */
            for (unsigned i = 0; i < nir->info.clip_distance_array_size; i++) {
               /* deref the array member and store each component */
               nir_deref_instr *component_deref = nir_build_deref_array_imm(&b, clipdist_deref, i);
               nir_def *val = zero;
               if (wrmask & BITFIELD_BIT(i - offset))
                  val = nir_channel(&b, intr->src[1].ssa, i - offset);
               nir_store_deref(&b, component_deref, val, 0x1);
            }
            func_progress = true;
            /* immediately remove the old store, save the original deref */
            nir_instr_remove(instr);
            _mesa_set_add(deletes, deref);
         }
      }
      if (func_progress)
         nir_metadata_preserve(impl, nir_metadata_none);
      /* derefs must be queued for deletion to avoid deleting the same deref repeatedly */
      set_foreach_remove(deletes, he)
         nir_instr_remove((void*)he->key);
   }
   /* VARYING_SLOT_CLIP_DIST1 is no longer used and can be removed */
   if (dist1)
      exec_node_remove(&dist1->node);
   return progress;
}

/**
 * Finalizes the NIR in a similar way as st_glsl_to_nir does.
 *
 * Drivers expect that these passes are already performed,
 * so we have to do it here too.
 */
static void
ttn_finalize_nir(struct ttn_compile *c, struct pipe_screen *screen)
{
   struct nir_shader *nir = c->build.shader;

   MESA_TRACE_FUNC();

   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_lower_reg_intrinsics_to_ssa);

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_system_values);
   NIR_PASS_V(nir, nir_lower_compute_system_values, NULL);

   if (!screen->get_param(screen, PIPE_CAP_TEXRECT)) {
      const struct nir_lower_tex_options opts = { .lower_rect = true, };
      NIR_PASS_V(nir, nir_lower_tex, &opts);
   }

   /* driver needs clipdistance as array<float> */
   if ((nir->info.outputs_written &
        (BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST0) | BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST1))) &&
       screen->get_param(screen, PIPE_CAP_NIR_COMPACT_ARRAYS)) {
      NIR_PASS_V(nir, lower_clipdistance_to_array);
   }

   if (nir->options->lower_uniforms_to_ubo)
      NIR_PASS_V(nir, nir_lower_uniforms_to_ubo, false, !c->cap_integers);

   if (nir->options->lower_int64_options)
      NIR_PASS_V(nir, nir_lower_int64);

   if (!c->cap_samplers_as_deref)
      NIR_PASS_V(nir, nir_lower_samplers);

   if (screen->finalize_nir) {
      char *msg = screen->finalize_nir(screen, nir);
      free(msg);
   } else {
      ttn_optimize_nir(nir);
      nir_shader_gather_info(nir, c->build.impl);
   }

   nir->info.num_images = c->num_images;
   nir->info.num_textures = c->num_samplers;

   nir_validate_shader(nir, "TTN: after all optimizations");
}

static void save_nir_to_disk_cache(struct disk_cache *cache,
                                   uint8_t key[CACHE_KEY_SIZE],
                                   const nir_shader *s)
{
   struct blob blob = {0};

   blob_init(&blob);
   /* Because we cannot fully trust disk_cache_put
    * (EGL_ANDROID_blob_cache) we add the shader size,
    * which we'll check after disk_cache_get().
    */
   if (blob_reserve_uint32(&blob) != 0) {
      blob_finish(&blob);
      return;
   }

   nir_serialize(&blob, s, true);
   *(uint32_t *)blob.data = blob.size;

   disk_cache_put(cache, key, blob.data, blob.size, NULL);
   blob_finish(&blob);
}

static nir_shader *
load_nir_from_disk_cache(struct disk_cache *cache,
                         struct pipe_screen *screen,
                         uint8_t key[CACHE_KEY_SIZE],
                         unsigned processor)
{
   const nir_shader_compiler_options *options =
      screen->get_compiler_options(screen, PIPE_SHADER_IR_NIR, processor);
   struct blob_reader blob_reader;
   size_t size;
   nir_shader *s;

   uint32_t *buffer = (uint32_t *)disk_cache_get(cache, key, &size);
   if (!buffer)
      return NULL;

   /* Match found. No need to check crc32 or other things.
    * disk_cache_get is supposed to do that for us.
    * However we do still check if the first element is indeed the size,
    * as we cannot fully trust disk_cache_get (EGL_ANDROID_blob_cache) */
   if (buffer[0] != size) {
      return NULL;
   }

   size -= 4;
   blob_reader_init(&blob_reader, buffer + 1, size);
   s = nir_deserialize(NULL, options, &blob_reader);
   free(buffer); /* buffer was malloc-ed */
   return s;
}

struct nir_shader *
tgsi_to_nir(const void *tgsi_tokens,
            struct pipe_screen *screen,
            bool allow_disk_cache)
{
   struct disk_cache *cache = NULL;
   struct ttn_compile *c;
   struct nir_shader *s = NULL;
   uint8_t key[CACHE_KEY_SIZE];
   unsigned processor;

   if (allow_disk_cache)
      cache = screen->get_disk_shader_cache(screen);

   /* Look first in the cache */
   if (cache) {
      disk_cache_compute_key(cache,
                             tgsi_tokens,
                             tgsi_num_tokens(tgsi_tokens) * sizeof(struct tgsi_token),
                             key);
      processor = tgsi_get_processor_type(tgsi_tokens);
      s = load_nir_from_disk_cache(cache, screen, key, processor);
   }

   if (s)
      return s;

#ifndef NDEBUG
   nir_process_debug_variable();
#endif

   if (NIR_DEBUG(TGSI)) {
      fprintf(stderr, "TGSI before translation to NIR:\n");
      tgsi_dump(tgsi_tokens, 0);
   }

   /* Not in the cache */

   c = ttn_compile_init(tgsi_tokens, NULL, screen);
   s = c->build.shader;
   ttn_finalize_nir(c, screen);
   ralloc_free(c);

   if (NIR_DEBUG(TGSI)) {
      mesa_logi("NIR after translation from TGSI:\n");
      nir_log_shaderi(s);
   }

   if (cache)
      save_nir_to_disk_cache(cache, key, s);

   return s;
}

struct nir_shader *
tgsi_to_nir_noscreen(const void *tgsi_tokens,
                     const nir_shader_compiler_options *options)
{
   struct ttn_compile *c;
   struct nir_shader *s;

   c = ttn_compile_init(tgsi_tokens, options, NULL);
   s = c->build.shader;
   ralloc_free(c);

   return s;
}

