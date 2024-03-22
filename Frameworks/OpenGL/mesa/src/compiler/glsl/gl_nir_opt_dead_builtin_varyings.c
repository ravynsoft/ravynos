/*
 * Copyright © 2013 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2022 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * This eliminates the built-in shader outputs which are either not written
 * at all or not used by the next stage. It also eliminates unused elements
 * of gl_TexCoord inputs, which reduces the overall varying usage.
 * The varyings handled here are the primary and secondary color, the fog,
 * and the texture coordinates (gl_TexCoord).
 *
 * This pass is necessary, because the Mesa GLSL linker cannot eliminate
 * built-in varyings like it eliminates user-defined varyings, because
 * the built-in varyings have pre-assigned locations. Also, the elimination
 * of unused gl_TexCoord elements requires its own lowering pass anyway.
 *
 * It's implemented by replacing all occurrences of dead varyings with
 * temporary variables, which creates dead code. It is recommended to run
 * a dead-code elimination pass after this.
 *
 * If any texture coordinate slots can be eliminated, the gl_TexCoord array is
 * broken down into separate vec4 variables with locations equal to
 * VARYING_SLOT_TEX0 + i.
 */

#include "gl_nir_link_varyings.h"
#include "gl_nir_linker.h"
#include "glsl_types.h"
#include "linker_util.h"
#include "nir_builder.h"

#include "main/consts_exts.h"
#include "main/shader_types.h"
#include "util/u_string.h"

struct varying_info {
   bool lower_texcoord_array;
   nir_variable *texcoord_array;
   unsigned texcoord_usage; /* bitmask */

   bool find_frag_outputs; /* false if it's looking for varyings */

   nir_variable *color[2];
   nir_variable *backcolor[2];
   unsigned color_usage; /* bitmask */
   unsigned tfeedback_color_usage; /* bitmask */

   nir_variable *fog;
   bool has_fog;
   bool tfeedback_has_fog;

   nir_variable_mode mode;
};

static void
initialise_varying_info(struct varying_info *info, nir_variable_mode mode,
                        bool find_frag_outputs)
{
   info->find_frag_outputs = find_frag_outputs;
   info->lower_texcoord_array = true;
   info->texcoord_array = NULL;
   info->texcoord_usage = 0;
   info->color_usage = 0;
   info->tfeedback_color_usage = 0;
   info->fog = NULL;
   info->has_fog = false;
   info->tfeedback_has_fog = false;
   info->mode = mode;

   memset(info->color, 0, sizeof(info->color));
   memset(info->backcolor, 0, sizeof(info->backcolor));
}

static void
gather_info_on_varying_deref(struct varying_info *info, nir_deref_instr *deref)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   if (!glsl_type_is_array(var->type) || !is_gl_identifier(var->name))
      return;

   if (!info->find_frag_outputs && var->data.location == VARYING_SLOT_TEX0) {
      info->texcoord_array = var;

      assert(deref->deref_type == nir_deref_type_array);
      if (nir_src_is_const(deref->arr.index)) {
         info->texcoord_usage |= 1 << nir_src_as_uint(deref->arr.index);
      } else {
         /* There is variable indexing, we can't lower the texcoord array. */
         info->texcoord_usage |= (1 << glsl_array_size(var->type)) - 1;
         info->lower_texcoord_array = false;
      }

      return;
   }
}

/**
 * This obtains detailed information about built-in varyings from shader code.
 */
static void
get_varying_info(struct varying_info *info, nir_shader *shader,
                 unsigned num_tfeedback_decls, struct xfb_decl *tfeedback_decls)
{
   /* Handle the transform feedback varyings. */
   for (unsigned i = 0; i < num_tfeedback_decls; i++) {
      if (!xfb_decl_is_varying(&tfeedback_decls[i]))
         continue;

      unsigned location = tfeedback_decls[i].location;

      switch (location) {
      case VARYING_SLOT_COL0:
      case VARYING_SLOT_BFC0:
         info->tfeedback_color_usage |= 1;
         break;
      case VARYING_SLOT_COL1:
      case VARYING_SLOT_BFC1:
         info->tfeedback_color_usage |= 2;
         break;
      case VARYING_SLOT_FOGC:
         info->tfeedback_has_fog = true;
         break;
      default:
         if (location >= VARYING_SLOT_TEX0 &&
             location <= VARYING_SLOT_TEX7) {
            info->lower_texcoord_array = false;
         }
      }
   }

   /* Process frag shader vars */
    nir_foreach_variable_with_modes(var, shader, info->mode) {
      /* Nothing to do here for fragment outputs. */
      if (info->find_frag_outputs)
         break;

      /* Handle colors and fog. */
      switch (var->data.location) {
      case VARYING_SLOT_COL0:
         info->color[0] = var;
         info->color_usage |= 1;
         break;
      case VARYING_SLOT_COL1:
         info->color[1] = var;
         info->color_usage |= 2;
         break;
      case VARYING_SLOT_BFC0:
         info->backcolor[0] = var;
         info->color_usage |= 1;
         break;
      case VARYING_SLOT_BFC1:
         info->backcolor[1] = var;
         info->color_usage |= 2;
         break;
      case VARYING_SLOT_FOGC:
         info->fog = var;
         info->has_fog = true;
         break;
      }
   }

   /* Process the shader. */
   assert(shader->info.stage != MESA_SHADER_COMPUTE);
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   /* assert that functions have been inlined before packing is called */
   nir_foreach_function(f, shader) {
      assert(f->impl == impl);
   }

   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         /* Copies should have been lowered by nir_split_var_copies() before
          * calling this pass.
          */
         assert(intrin->intrinsic != nir_intrinsic_copy_deref);

         if (intrin->intrinsic != nir_intrinsic_load_deref &&
             intrin->intrinsic != nir_intrinsic_store_deref)
            continue;

         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (!nir_deref_mode_is(deref, info->mode))
            continue;

         gather_info_on_varying_deref(info, deref);
      }
   }

   if (!info->texcoord_array) {
      info->lower_texcoord_array = false;
   }
}

struct replace_varyings_data {
   const struct gl_constants *consts;

   struct gl_shader_program *prog;
   struct gl_linked_shader *shader;
   const struct varying_info *info;

   nir_variable *new_texcoord[MAX_TEXTURE_COORD_UNITS];
   nir_variable *new_color[2];
   nir_variable *new_backcolor[2];
   nir_variable *new_fog;
};

static nir_variable *
create_new_var(nir_shader *shader, char *name, nir_variable_mode mode,
               const struct glsl_type *type)
{
   nir_variable *var = rzalloc(shader, nir_variable);
   var->name = ralloc_strdup(var, name);
   var->data.mode = mode;
   var->type = type;

   nir_shader_add_variable(shader, var);
   return var;
}

static void
replace_varying(struct replace_varyings_data *rv_data, nir_variable *var)
{
   /* Remove the gl_TexCoord array. */
   if (rv_data->info->lower_texcoord_array &&
       var == rv_data->info->texcoord_array) {
      var->data.mode = nir_var_shader_temp;
   }

   /* Lower set-but-unused color and fog outputs to shader temps. */
   for (int i = 0; i < 2; i++) {
      if (var == rv_data->info->color[i] && rv_data->new_color[i]) {
         var->data.mode = nir_var_shader_temp;
      }

      if (var == rv_data->info->backcolor[i] && rv_data->new_backcolor[i]) {
         var->data.mode = nir_var_shader_temp;
      }
   }

   if (var == rv_data->info->fog && rv_data->new_fog) {
      var->data.mode = nir_var_shader_temp;
   }
}

static void
rewrite_varying_deref(nir_builder *b, struct replace_varyings_data *rv_data,
                      nir_deref_instr *deref)
{
   if (deref->deref_type != nir_deref_type_array)
      return;

   nir_variable *var = nir_deref_instr_get_variable(deref);
   const struct varying_info *info = rv_data->info;
   b->cursor = nir_before_instr(&deref->instr);

   /* Replace an array dereference gl_TexCoord[i] with a single
    * variable dereference representing gl_TexCoord[i].
    */
   if (info->lower_texcoord_array && info->texcoord_array == var) {
      /* gl_TexCoord[i] occurrence */
      unsigned i = nir_src_as_uint(deref->arr.index);
      nir_deref_instr *new_deref =
         nir_build_deref_var(b, rv_data->new_texcoord[i]);
      nir_def_rewrite_uses(&deref->def, &new_deref->def);
      return;
   }
}

static void
prepare_array(struct replace_varyings_data *rv_data,
              nir_shader *shader, nir_variable **new_var,
              int max_elements, unsigned start_location,
              const char *var_name, const char *mode_str,
              unsigned usage, unsigned external_usage)
{
   for (int i = max_elements - 1; i >= 0; i--) {
      if (usage & (1 << i)) {
         char name[32];

         if (!(external_usage & (1 << i))) {
            /* This varying is unused in the next stage. Declare
             * a temporary instead of an output. */
            snprintf(name, 32, "gl_%s_%s%i_dummy", mode_str, var_name, i);
            new_var[i] = create_new_var(shader, name, nir_var_shader_temp,
                                        glsl_vec4_type());
         } else {
            snprintf(name, 32, "gl_%s_%s%i", mode_str, var_name, i);
            new_var[i] = create_new_var(shader, name, rv_data->info->mode,
                                        glsl_vec4_type());
            new_var[i]->data.location = start_location + i;
            new_var[i]->data.explicit_location = true;
         }
      }
   }
}

/**
 * This replaces unused varyings with temporary variables.
 *
 * If "ir" is the producer, the "external" usage should come from
 * the consumer. It also works the other way around. If either one is
 * missing, set the "external" usage to a full mask.
 */
static void
replace_varyings(const struct gl_constants *consts,
                 struct gl_linked_shader *shader,
                 struct gl_shader_program *prog,
                 const struct varying_info *info,
                 unsigned external_texcoord_usage,
                 unsigned external_color_usage, bool external_has_fog)
{
   struct replace_varyings_data rv_data;
   rv_data.shader = shader;
   rv_data.info = info;
   rv_data.consts = consts;
   rv_data.prog = prog;

   memset(rv_data.new_texcoord, 0, sizeof(rv_data.new_texcoord));
   memset(rv_data.new_color, 0, sizeof(rv_data.new_color));
   memset(rv_data.new_backcolor, 0, sizeof(rv_data.new_backcolor));
   rv_data.new_fog = NULL;

   const char *mode_str = info->mode == nir_var_shader_in ? "in" : "out";

   /* Handle texcoord outputs.
    *
    * We're going to break down the gl_TexCoord array into separate
    * variables. First, add declarations of the new variables all
    * occurrences of gl_TexCoord will be replaced with.
    */
   if (info->lower_texcoord_array) {
      prepare_array(&rv_data, shader->Program->nir, rv_data.new_texcoord,
                    ARRAY_SIZE(rv_data.new_texcoord),
                    VARYING_SLOT_TEX0, "TexCoord", mode_str,
                    info->texcoord_usage, external_texcoord_usage);
   }

   /* Create dummy variables which will replace set-but-unused color and
    * fog outputs.
    */
   external_color_usage |= info->tfeedback_color_usage;

   for (int i = 0; i < 2; i++) {
      char name[32];

      if (!(external_color_usage & (1 << i))) {
         if (info->color[i]) {
            snprintf(name, 32, "gl_%s_FrontColor%i_dummy", mode_str, i);
            rv_data.new_color[i] =
               create_new_var(shader->Program->nir, name, nir_var_shader_temp,
                              glsl_vec4_type());
         }

         if (info->backcolor[i]) {
            snprintf(name, 32, "gl_%s_BackColor%i_dummy", mode_str, i);
            rv_data.new_backcolor[i] =
               create_new_var(shader->Program->nir, name, nir_var_shader_temp,
                              glsl_vec4_type());
         }
      }
   }

   if (!external_has_fog && !info->tfeedback_has_fog && info->fog) {
      char name[32];

      snprintf(name, 32, "gl_%s_FogFragCoord_dummy", mode_str);
      rv_data.new_fog =
         create_new_var(shader->Program->nir, name, nir_var_shader_temp,
                        glsl_float_type());
   }

   /* Now do the replacing. */
   nir_foreach_variable_with_modes_safe(var, shader->Program->nir, info->mode) {
      replace_varying(&rv_data, var);
   }

   nir_function_impl *impl = nir_shader_get_entrypoint(shader->Program->nir);
   nir_builder b = nir_builder_create(impl);

   /* assert that functions have been inlined before packing is called */
   nir_foreach_function(f, shader->Program->nir) {
      assert(f->impl == impl);
   }

   /* Rewrite the derefs to use the new vars */
   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         /* Copies should have been lowered by nir_split_var_copies() before
          * calling this pass.
          */
         assert(intrin->intrinsic != nir_intrinsic_copy_deref);

         if (intrin->intrinsic != nir_intrinsic_load_deref &&
             intrin->intrinsic != nir_intrinsic_store_deref)
            continue;

         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (!nir_deref_mode_is(deref, info->mode))
            continue;

         rewrite_varying_deref(&b, &rv_data, deref);
      }
   }
}

static void
lower_texcoord_array(const struct gl_constants *consts,
                     struct gl_linked_shader *shader,
                     struct gl_shader_program *prog,
                     const struct varying_info *info)
{
   replace_varyings(consts, shader, prog, info,
                    (1 << MAX_TEXTURE_COORD_UNITS) - 1, 1 | 2, true);
}

void
gl_nir_opt_dead_builtin_varyings(const struct gl_constants *consts, gl_api api,
                                 struct gl_shader_program *prog,
                                 struct gl_linked_shader *producer,
                                 struct gl_linked_shader *consumer,
                                 unsigned num_tfeedback_decls,
                                 struct xfb_decl *tfeedback_decls)
{
   /* Lowering of built-in varyings has no effect with the core context and
    * GLES2, because they are not available there.
    */
   if (api == API_OPENGL_CORE ||
       api == API_OPENGLES2) {
      goto done;
   }

   /* Information about built-in varyings. */
   struct varying_info producer_info;
   struct varying_info consumer_info;
   initialise_varying_info(&producer_info, nir_var_shader_out, false);
   initialise_varying_info(&consumer_info, nir_var_shader_in, false);

   if (producer) {
      get_varying_info(&producer_info, producer->Program->nir,
                       num_tfeedback_decls, tfeedback_decls);

      if (producer->Stage == MESA_SHADER_TESS_CTRL)
         producer_info.lower_texcoord_array = false;

      if (!consumer) {
         /* At least eliminate unused gl_TexCoord elements. */
         if (producer_info.lower_texcoord_array) {
            lower_texcoord_array(consts, producer, prog, &producer_info);
         }
         goto done;
      }
   }

   if (consumer) {
      get_varying_info(&consumer_info, consumer->Program->nir,
                       num_tfeedback_decls, tfeedback_decls);

      if (consumer->Stage != MESA_SHADER_FRAGMENT)
         consumer_info.lower_texcoord_array = false;

      if (!producer) {
         /* At least eliminate unused gl_TexCoord elements. */
         if (consumer_info.lower_texcoord_array) {
            lower_texcoord_array(consts, consumer, prog, &consumer_info);
         }
         goto done;
      }
   }

   /* Eliminate the outputs unused by the consumer. */
   if (producer_info.lower_texcoord_array ||
       producer_info.color_usage ||
       producer_info.has_fog) {
      replace_varyings(consts, producer, prog, &producer_info,
                       consumer_info.texcoord_usage,
                       consumer_info.color_usage,
                       consumer_info.has_fog);
   }

   /* The gl_TexCoord fragment shader inputs can be initialized
    * by GL_COORD_REPLACE, so we can't eliminate them.
    *
    * This doesn't prevent elimination of the gl_TexCoord elements which
    * are not read by the fragment shader. We want to eliminate those anyway.
    */
   if (consumer->Stage == MESA_SHADER_FRAGMENT) {
      producer_info.texcoord_usage = (1 << MAX_TEXTURE_COORD_UNITS) - 1;
   }

   /* Eliminate the inputs uninitialized by the producer. */
   if (consumer_info.lower_texcoord_array ||
       consumer_info.color_usage ||
       consumer_info.has_fog) {
      replace_varyings(consts, consumer, prog, &consumer_info,
                       producer_info.texcoord_usage,
                       producer_info.color_usage,
                       producer_info.has_fog);
   }

done:
   if (producer)
      nir_fixup_deref_modes(producer->Program->nir);

   if (consumer)
      nir_fixup_deref_modes(consumer->Program->nir);
}
