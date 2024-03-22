/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * \file ffvertex_prog.c
 *
 * Create a vertex program to execute the current fixed function T&L pipeline.
 * \author Keith Whitwell
 */


#include "main/errors.h"
#include "util/glheader.h"
#include "main/mtypes.h"
#include "main/macros.h"
#include "main/enums.h"
#include "main/context.h"
#include "main/ffvertex_prog.h"
#include "program/program.h"
#include "program/prog_cache.h"
#include "program/prog_statevars.h"
#include "util/bitscan.h"

#include "state_tracker/st_program.h"
#include "state_tracker/st_nir.h"

#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_builtin_builder.h"

/** Max of number of lights and texture coord units */
#define NUM_UNITS MAX2(MAX_TEXTURE_COORD_UNITS, MAX_LIGHTS)

struct state_key {
   GLbitfield varying_vp_inputs;

   unsigned fragprog_inputs_read:12;

   unsigned light_color_material_mask:12;
   unsigned light_global_enabled:1;
   unsigned light_local_viewer:1;
   unsigned light_twoside:1;
   unsigned material_shininess_is_zero:1;
   unsigned need_eye_coords:1;
   unsigned normalize:1;
   unsigned rescale_normals:1;

   unsigned fog_distance_mode:2;
   unsigned separate_specular:1;
   unsigned point_attenuated:1;

   struct {
      unsigned char light_enabled:1;
      unsigned char light_eyepos3_is_zero:1;
      unsigned char light_spotcutoff_is_180:1;
      unsigned char light_attenuated:1;
      unsigned char texmat_enabled:1;
      unsigned char coord_replace:1;
      unsigned char texgen_enabled:1;
      unsigned char texgen_mode0:4;
      unsigned char texgen_mode1:4;
      unsigned char texgen_mode2:4;
      unsigned char texgen_mode3:4;
   } unit[NUM_UNITS];
};


#define TXG_NONE           0
#define TXG_OBJ_LINEAR     1
#define TXG_EYE_LINEAR     2
#define TXG_SPHERE_MAP     3
#define TXG_REFLECTION_MAP 4
#define TXG_NORMAL_MAP     5

static GLuint translate_texgen( GLboolean enabled, GLenum mode )
{
   if (!enabled)
      return TXG_NONE;

   switch (mode) {
   case GL_OBJECT_LINEAR: return TXG_OBJ_LINEAR;
   case GL_EYE_LINEAR: return TXG_EYE_LINEAR;
   case GL_SPHERE_MAP: return TXG_SPHERE_MAP;
   case GL_REFLECTION_MAP_NV: return TXG_REFLECTION_MAP;
   case GL_NORMAL_MAP_NV: return TXG_NORMAL_MAP;
   default: return TXG_NONE;
   }
}

#define FDM_EYE_RADIAL    0
#define FDM_EYE_PLANE     1
#define FDM_EYE_PLANE_ABS 2
#define FDM_FROM_ARRAY    3

static GLuint translate_fog_distance_mode(GLenum source, GLenum mode)
{
   if (source == GL_FRAGMENT_DEPTH_EXT) {
      switch (mode) {
      case GL_EYE_RADIAL_NV:
         return FDM_EYE_RADIAL;
      case GL_EYE_PLANE:
         return FDM_EYE_PLANE;
      default: /* shouldn't happen; fall through to a sensible default */
      case GL_EYE_PLANE_ABSOLUTE_NV:
         return FDM_EYE_PLANE_ABS;
      }
   } else {
      return FDM_FROM_ARRAY;
   }
}

static GLboolean check_active_shininess( struct gl_context *ctx,
                                         const struct state_key *key,
                                         GLuint side )
{
   GLuint attr = MAT_ATTRIB_FRONT_SHININESS + side;

   if ((key->varying_vp_inputs & VERT_BIT_COLOR0) &&
       (key->light_color_material_mask & (1 << attr)))
      return GL_TRUE;

   if (key->varying_vp_inputs & VERT_BIT_MAT(attr))
      return GL_TRUE;

   if (ctx->Light.Material.Attrib[attr][0] != 0.0F)
      return GL_TRUE;

   return GL_FALSE;
}


static void make_state_key( struct gl_context *ctx, struct state_key *key )
{
   const struct gl_program *fp = ctx->FragmentProgram._Current;
   GLbitfield mask;

   memset(key, 0, sizeof(struct state_key));

   if (_mesa_hw_select_enabled(ctx)) {
      /* GL_SELECT mode only need position calculation.
       * glBegin/End use VERT_BIT_SELECT_RESULT_OFFSET for multi name stack in one draw.
       * glDrawArrays may also be called without user shader, fallback to FF one.
       */
      key->varying_vp_inputs = ctx->VertexProgram._VaryingInputs &
         (VERT_BIT_POS | VERT_BIT_SELECT_RESULT_OFFSET);
      return;
   }

   /* This now relies on texenvprogram.c being active:
    */
   assert(fp);

   key->need_eye_coords = ctx->_NeedEyeCoords;

   key->fragprog_inputs_read = fp->info.inputs_read;
   key->varying_vp_inputs = ctx->VertexProgram._VaryingInputs;

   if (ctx->RenderMode == GL_FEEDBACK) {
      /* make sure the vertprog emits color and tex0 */
      key->fragprog_inputs_read |= (VARYING_BIT_COL0 | VARYING_BIT_TEX0);
   }

   if (ctx->Light.Enabled) {
      key->light_global_enabled = 1;

      if (ctx->Light.Model.LocalViewer)
          key->light_local_viewer = 1;

      if (ctx->Light.Model.TwoSide)
          key->light_twoside = 1;

      if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
         key->separate_specular = 1;

      if (ctx->Light.ColorMaterialEnabled) {
          key->light_color_material_mask = ctx->Light._ColorMaterialBitmask;
      }

      mask = ctx->Light._EnabledLights;
      while (mask) {
         const int i = u_bit_scan(&mask);
         struct gl_light_uniforms *lu = &ctx->Light.LightSource[i];

         key->unit[i].light_enabled = 1;

         if (lu->EyePosition[3] == 0.0F)
            key->unit[i].light_eyepos3_is_zero = 1;

         if (lu->SpotCutoff == 180.0F)
            key->unit[i].light_spotcutoff_is_180 = 1;

         if (lu->ConstantAttenuation != 1.0F ||
             lu->LinearAttenuation != 0.0F ||
             lu->QuadraticAttenuation != 0.0F)
            key->unit[i].light_attenuated = 1;
      }

      if (check_active_shininess(ctx, key, 0)) {
         key->material_shininess_is_zero = 0;
      }
      else if (key->light_twoside &&
               check_active_shininess(ctx, key, 1)) {
         key->material_shininess_is_zero = 0;
      }
      else {
         key->material_shininess_is_zero = 1;
      }
   }

   if (ctx->Transform.Normalize)
      key->normalize = 1;

   if (ctx->Transform.RescaleNormals)
      key->rescale_normals = 1;

   /* Only distinguish fog parameters if we actually need */
   if (key->fragprog_inputs_read & VARYING_BIT_FOGC)
      key->fog_distance_mode =
         translate_fog_distance_mode(ctx->Fog.FogCoordinateSource,
                                     ctx->Fog.FogDistanceMode);

   if (ctx->Point._Attenuated)
      key->point_attenuated = 1;

   mask = ctx->Texture._EnabledCoordUnits | ctx->Texture._TexGenEnabled
      | ctx->Texture._TexMatEnabled | ctx->Point.CoordReplace;
   while (mask) {
      const int i = u_bit_scan(&mask);
      struct gl_fixedfunc_texture_unit *texUnit =
         &ctx->Texture.FixedFuncUnit[i];

      if (ctx->Point.PointSprite)
         if (ctx->Point.CoordReplace & (1u << i))
            key->unit[i].coord_replace = 1;

      if (ctx->Texture._TexMatEnabled & ENABLE_TEXMAT(i))
         key->unit[i].texmat_enabled = 1;

      if (texUnit->TexGenEnabled) {
         key->unit[i].texgen_enabled = 1;

         key->unit[i].texgen_mode0 =
            translate_texgen( texUnit->TexGenEnabled & (1<<0),
                              texUnit->GenS.Mode );
         key->unit[i].texgen_mode1 =
            translate_texgen( texUnit->TexGenEnabled & (1<<1),
                              texUnit->GenT.Mode );
         key->unit[i].texgen_mode2 =
            translate_texgen( texUnit->TexGenEnabled & (1<<2),
                              texUnit->GenR.Mode );
         key->unit[i].texgen_mode3 =
            translate_texgen( texUnit->TexGenEnabled & (1<<3),
                              texUnit->GenQ.Mode );
      }
   }
}

struct tnl_program {
   const struct state_key *state;
   struct gl_program_parameter_list *state_params;
   GLboolean mvp_with_dp4;

   nir_builder *b;

   nir_def *eye_position;
   nir_def *eye_position_z;
   nir_def *eye_position_normalized;
   nir_def *transformed_normal;

   GLuint materials;
   GLuint color_materials;
};

static nir_variable *
register_state_var(struct tnl_program *p,
                   gl_state_index16 s0,
                   gl_state_index16 s1,
                   gl_state_index16 s2,
                   gl_state_index16 s3,
                   const struct glsl_type *type)
{
   gl_state_index16 tokens[STATE_LENGTH];
   tokens[0] = s0;
   tokens[1] = s1;
   tokens[2] = s2;
   tokens[3] = s3;
   nir_variable *var = nir_find_state_variable(p->b->shader, tokens);
   if (var)
      return var;

   var = st_nir_state_variable_create(p->b->shader, type, tokens);
   var->data.driver_location = _mesa_add_state_reference(p->state_params, tokens);

   return var;
}

static nir_def *
load_state_var(struct tnl_program *p,
               gl_state_index16 s0,
               gl_state_index16 s1,
               gl_state_index16 s2,
               gl_state_index16 s3,
               const struct glsl_type *type)
{
   nir_variable *var = register_state_var(p, s0, s1, s2, s3, type);
   return nir_load_var(p->b, var);
}

static nir_def *
load_state_vec4(struct tnl_program *p,
                gl_state_index16 s0,
                gl_state_index16 s1,
                gl_state_index16 s2,
                gl_state_index16 s3)
{
   return load_state_var(p, s0, s1, s2, s3, glsl_vec4_type());
}

static void
load_state_mat4(struct tnl_program *p, nir_def *out[4],
                gl_state_index state_index, unsigned tex_index)
{
   for (int i = 0; i < 4; ++i)
      out[i] = load_state_vec4(p, state_index, tex_index, i, i);
}

static nir_def *
load_input(struct tnl_program *p, gl_vert_attrib attr,
           const struct glsl_type *type)
{
   if (p->state->varying_vp_inputs & VERT_BIT(attr)) {
      nir_variable *var = nir_get_variable_with_location(p->b->shader, nir_var_shader_in,
                                                         attr, type);
      p->b->shader->info.inputs_read |= (uint64_t)VERT_BIT(attr);
      return nir_load_var(p->b, var);
   } else
      return load_state_var(p, STATE_CURRENT_ATTRIB, attr, 0, 0, type);
}

static nir_def *
load_input_vec4(struct tnl_program *p, gl_vert_attrib attr)
{
   return load_input(p, attr, glsl_vec4_type());
}

static nir_variable *
register_output(struct tnl_program *p, gl_varying_slot slot,
                const struct glsl_type *type)
{
   nir_variable *var = nir_get_variable_with_location(p->b->shader, nir_var_shader_out,
                                                      slot, type);
   p->b->shader->info.outputs_written |= BITFIELD64_BIT(slot);
   return var;
}

static void
store_output_vec4_masked(struct tnl_program *p, gl_varying_slot slot,
                         nir_def *value, unsigned mask)
{
   assert(mask <= 0xf);
   nir_variable *var = register_output(p, slot, glsl_vec4_type());
   nir_store_var(p->b, var, value, mask);
}

static void
store_output_vec4(struct tnl_program *p, gl_varying_slot slot,
                  nir_def *value)
{
   store_output_vec4_masked(p, slot, value, 0xf);
}

static void
store_output_float(struct tnl_program *p, gl_varying_slot slot,
                   nir_def *value)
{
   nir_variable *var = register_output(p, slot, glsl_float_type());
   nir_store_var(p->b, var, value, 0x1);
}


static nir_def *
emit_matrix_transform_vec4(nir_builder *b,
                           nir_def *mat[4],
                           nir_def *src)
{
   return nir_vec4(b,
                   nir_fdot4(b, src, mat[0]),
                   nir_fdot4(b, src, mat[1]),
                   nir_fdot4(b, src, mat[2]),
                   nir_fdot4(b, src, mat[3]));
}

static nir_def *
emit_transpose_matrix_transform_vec4(nir_builder *b,
                                     nir_def *mat[4],
                                     nir_def *src)
{
   nir_def *result;
   result = nir_fmul(b, nir_channel(b, src, 0), mat[0]);
   result = nir_fmad(b, nir_channel(b, src, 1), mat[1], result);
   result = nir_fmad(b, nir_channel(b, src, 2), mat[2], result);
   result = nir_fmad(b, nir_channel(b, src, 3), mat[3], result);
   return result;
}

static nir_def *
emit_matrix_transform_vec3(nir_builder *b,
                           nir_def *mat[3],
                           nir_def *src)
{
   return nir_vec3(b,
                   nir_fdot3(b, src, mat[0]),
                   nir_fdot3(b, src, mat[1]),
                   nir_fdot3(b, src, mat[2]));
}

static nir_def *
emit_normalize_vec3(nir_builder *b, nir_def *src)
{
   nir_def *tmp = nir_frsq(b, nir_fdot3(b, src, src));
   return nir_fmul(b, src, tmp);
}

static void
emit_passthrough(struct tnl_program *p, gl_vert_attrib attr,
                 gl_varying_slot varying)
{
   nir_def *val = load_input_vec4(p, attr);
   store_output_vec4(p, varying, val);
}

static nir_def *
get_eye_position(struct tnl_program *p)
{
   if (!p->eye_position) {
      nir_def *pos =
         load_input_vec4(p, VERT_ATTRIB_POS);
      if (p->mvp_with_dp4) {
         nir_def *modelview[4];
         load_state_mat4(p, modelview, STATE_MODELVIEW_MATRIX, 0);
         p->eye_position =
            emit_matrix_transform_vec4(p->b, modelview, pos);
      } else {
         nir_def *modelview[4];
         load_state_mat4(p, modelview,
                         STATE_MODELVIEW_MATRIX_TRANSPOSE, 0);
         p->eye_position =
            emit_transpose_matrix_transform_vec4(p->b, modelview, pos);
      }
   }

   return p->eye_position;
}

static nir_def *
get_eye_position_z(struct tnl_program *p)
{
   return nir_channel(p->b, get_eye_position(p), 2);
}

static nir_def *
get_eye_position_normalized(struct tnl_program *p)
{
   if (!p->eye_position_normalized) {
      nir_def *eye = get_eye_position(p);
      p->eye_position_normalized = emit_normalize_vec3(p->b, eye);
   }

   return p->eye_position_normalized;
}

static nir_def *
get_transformed_normal(struct tnl_program *p)
{
   if (!p->transformed_normal &&
       !p->state->need_eye_coords &&
       !p->state->normalize &&
       !(p->state->need_eye_coords == p->state->rescale_normals)) {
      p->transformed_normal =
         load_input(p, VERT_ATTRIB_NORMAL,
                    glsl_vector_type(GLSL_TYPE_FLOAT, 3));
   } else if (!p->transformed_normal) {
      nir_def *normal =
         load_input(p, VERT_ATTRIB_NORMAL,
                    glsl_vector_type(GLSL_TYPE_FLOAT, 3));

      if (p->state->need_eye_coords) {
         nir_def *mvinv[4];
         load_state_mat4(p, mvinv, STATE_MODELVIEW_MATRIX_INVTRANS, 0);
         normal = emit_matrix_transform_vec3(p->b, mvinv, normal);
      }

      /* Normalize/Rescale:
       */
      if (p->state->normalize)
         normal = emit_normalize_vec3(p->b, normal);
      else if (p->state->need_eye_coords == p->state->rescale_normals) {
         nir_def *scale =
            load_state_var(p, STATE_NORMAL_SCALE, 0, 0, 0,
                           glsl_float_type());
         normal = nir_fmul(p->b, normal, scale);
      }

      p->transformed_normal = normal;
   }

   return p->transformed_normal;
}

static GLuint material_attrib( GLuint side, GLuint property )
{
   switch (property) {
   case STATE_AMBIENT:
      return MAT_ATTRIB_FRONT_AMBIENT + side;
   case STATE_DIFFUSE:
      return MAT_ATTRIB_FRONT_DIFFUSE + side;
   case STATE_SPECULAR:
      return MAT_ATTRIB_FRONT_SPECULAR + side;
   case STATE_EMISSION:
      return MAT_ATTRIB_FRONT_EMISSION + side;
   case STATE_SHININESS:
      return MAT_ATTRIB_FRONT_SHININESS + side;
   default:
      unreachable("invalid value");
   }
}


/**
 * Get a bitmask of which material values vary on a per-vertex basis.
 */
static void set_material_flags( struct tnl_program *p )
{
   p->color_materials = 0;
   p->materials = 0;

   if (p->state->varying_vp_inputs & VERT_BIT_COLOR0) {
      p->materials =
         p->color_materials = p->state->light_color_material_mask;
   }

   p->materials |= ((p->state->varying_vp_inputs & VERT_BIT_MAT_ALL)
                    >> VERT_ATTRIB_MAT(0));
}


static nir_def *
get_material(struct tnl_program *p, GLuint side,
             GLuint property)
{
   GLuint attrib = material_attrib(side, property);

   if (p->color_materials & (1<<attrib))
      return load_input_vec4(p, VERT_ATTRIB_COLOR0);
   else if (p->materials & (1<<attrib)) {
      /* Put material values in the GENERIC slots -- they are not used
       * for anything in fixed function mode.
       */
      return load_input_vec4(p, VERT_ATTRIB_MAT(attrib));
   } else {
      return load_state_vec4(p, STATE_MATERIAL, attrib, 0, 0);
   }
}

#define SCENE_COLOR_BITS(side) (( MAT_BIT_FRONT_EMISSION | \
                                  MAT_BIT_FRONT_AMBIENT | \
                                  MAT_BIT_FRONT_DIFFUSE) << (side))


/**
 * Either return a precalculated constant value or emit code to
 * calculate these values dynamically in the case where material calls
 * are present between begin/end pairs.
 *
 * Probably want to shift this to the program compilation phase - if
 * we always emitted the calculation here, a smart compiler could
 * detect that it was constant (given a certain set of inputs), and
 * lift it out of the main loop.  That way the programs created here
 * would be independent of the vertex_buffer details.
 */
static nir_def *
get_scenecolor(struct tnl_program *p, GLuint side)
{
   if (p->materials & SCENE_COLOR_BITS(side)) {
      nir_def *lm_ambient =
         load_state_vec4(p, STATE_LIGHTMODEL_AMBIENT, 0, 0, 0);
      nir_def *material_emission =
         get_material(p, side, STATE_EMISSION);
      nir_def *material_ambient =
         get_material(p, side, STATE_AMBIENT);
      nir_def *material_diffuse =
         get_material(p, side, STATE_DIFFUSE);

      // rgb: material_emission + material_ambient * lm_ambient
      // alpha: material_diffuse.a
      return nir_vector_insert_imm(p->b, nir_fmad(p->b,
                                                  lm_ambient,
                                                  material_ambient,
                                                  material_emission),
                                   nir_channel(p->b,
                                               material_diffuse,
                                               3),
                                   3);
   }
   else
      return load_state_vec4(p, STATE_LIGHTMODEL_SCENECOLOR, side, 0, 0);
}

static nir_def *
get_lightprod(struct tnl_program *p, GLuint light,
              GLuint side, GLuint property, bool *is_state_light)
{
   GLuint attrib = material_attrib(side, property);
   if (p->materials & (1<<attrib)) {
      *is_state_light = true;
      return load_state_vec4(p, STATE_LIGHT, light, property, 0);
   } else {
      *is_state_light = false;
      return load_state_vec4(p, STATE_LIGHTPROD, light, attrib, 0);
   }
}


static nir_def *
calculate_light_attenuation(struct tnl_program *p,
                            GLuint i,
                            nir_def *VPpli,
                            nir_def *dist)
{
   nir_def *attenuation = NULL;
   nir_def *att = NULL;

   /* Calculate spot attenuation:
    */
   if (!p->state->unit[i].light_spotcutoff_is_180) {
       nir_def *spot_dir_norm =
         load_state_vec4(p, STATE_LIGHT_SPOT_DIR_NORMALIZED, i, 0, 0);
      attenuation =
         load_state_vec4(p, STATE_LIGHT, i, STATE_ATTENUATION, 0);

      nir_def *spot = nir_fdot3(p->b, nir_fneg(p->b, VPpli),
                                    spot_dir_norm);
      nir_def *cmp = nir_flt(p->b, nir_channel(p->b, spot_dir_norm, 3),
                                 spot);
      spot = nir_fpow(p->b, spot, nir_channel(p->b, attenuation, 3));
      att = nir_bcsel(p->b, cmp, spot, nir_imm_zero(p->b, 1, 32));
   }

   /* Calculate distance attenuation(See formula (2.4) at glspec 2.1 page 62):
    *
    * Skip the calucation when _dist_ is undefined(light_eyepos3_is_zero)
    */
   if (p->state->unit[i].light_attenuated && dist) {
      if (!attenuation) {
         attenuation = load_state_vec4(p, STATE_LIGHT, i,
                                       STATE_ATTENUATION, 0);
      }

      /* dist is the reciprocal of ||VP|| used in the distance
       * attenuation formula. So need to get the reciprocal of dist first
       * before applying to the formula.
       */
      dist = nir_frcp(p->b, dist);

      /* 1, d, d*d */
      nir_def *tmp = nir_vec3(p->b,
         nir_imm_float(p->b, 1.0f),
         dist,
         nir_fmul(p->b, dist, dist)
      );
      tmp = nir_frcp(p->b, nir_fdot3(p->b, tmp, attenuation));

      if (!p->state->unit[i].light_spotcutoff_is_180)
         return nir_fmul(p->b, tmp, att);
      return tmp;
   }

   return att;
}

static nir_def *
emit_lit(nir_builder *b,
         nir_def *src)
{
   nir_def *zero = nir_imm_zero(b, 1, 32);
   nir_def *one = nir_imm_float(b, 1.0f);
   nir_def *src_x = nir_channel(b, src, 0);
   nir_def *src_y = nir_channel(b, src, 1);
   nir_def *src_w = nir_channel(b, src, 3);

   nir_def *wclamp = nir_fmax(b, nir_fmin(b, src_w,
                                              nir_imm_float(b, 128.0f)),
                                  nir_imm_float(b, -128.0f));
   nir_def *pow = nir_fpow(b, nir_fmax(b, src_y, zero), wclamp);

   return nir_vec4(b,
                   one,
                   nir_fmax(b, src_x, zero),
                   nir_bcsel(b,
                             nir_fge(b, zero, src_x),
                             zero,
                             pow),
                   one);
}

/**
 * Compute:
 *   lit.y = MAX(0, dots.x)
 *   lit.z = SLT(0, dots.x)
 */
static nir_def *
emit_degenerate_lit(nir_builder *b,
                    nir_def *dots)
{
   nir_def *id = nir_imm_vec4(b, 0.0f, 0.0f, 0.0f, 1.0f);

   /* Note that lit.x & lit.w will not be examined.  Note also that
    * dots.xyzw == dots.xxxx.
    */

   nir_def *zero = nir_imm_zero(b, 1, 32);
   nir_def *dots_x = nir_channel(b, dots, 0);
   nir_def *tmp = nir_fmax(b, id, dots);
   return nir_vector_insert_imm(b, tmp, nir_slt(b, zero, dots_x), 2);
}


/* Need to add some addtional parameters to allow lighting in object
 * space - STATE_SPOT_DIRECTION and STATE_HALF_VECTOR implicitly assume eye
 * space lighting.
 */
static void build_lighting( struct tnl_program *p )
{
   const GLboolean twoside = p->state->light_twoside;
   const GLboolean separate = p->state->separate_specular;
   GLuint nr_lights = 0;
   nir_def *lit = NULL;
   nir_def *dots = nir_imm_zero(p->b, 4, 32);
   nir_def *normal = get_transformed_normal(p);
   nir_def *_col0 = NULL, *_col1 = NULL;
   nir_def *_bfc0 = NULL, *_bfc1 = NULL;
   GLuint i;

   /*
    * NOTE:
    * dots.x = dot(normal, VPpli)
    * dots.y = dot(normal, halfAngle)
    * dots.z = back.shininess
    * dots.w = front.shininess
    */

   for (i = 0; i < MAX_LIGHTS; i++)
      if (p->state->unit[i].light_enabled)
         nr_lights++;

   set_material_flags(p);

   {
      if (!p->state->material_shininess_is_zero) {
         nir_def *shininess = get_material(p, 0, STATE_SHININESS);
         nir_def *tmp = nir_channel(p->b, shininess, 0);
         dots = nir_vector_insert_imm(p->b, dots, tmp, 3);
      }

      _col0 = get_scenecolor(p, 0);
      if (separate)
         _col1 = nir_imm_vec4(p->b, 0.0f, 0.0f, 0.0f, 1.0f);
   }

   if (twoside) {
      if (!p->state->material_shininess_is_zero) {
         /* Note that we negate the back-face specular exponent here.
          * The negation will be un-done later in the back-face code below.
          */
         nir_def *shininess = get_material(p, 1, STATE_SHININESS);
         nir_def *tmp = nir_channel(p->b, shininess, 0);
         tmp = nir_fneg(p->b, tmp);
         dots = nir_vector_insert_imm(p->b, dots, tmp, 2);
      }

      _bfc0 = get_scenecolor(p, 1);
      if (separate)
         _bfc1 = nir_imm_vec4(p->b, 0.0f, 0.0f, 0.0f, 1.0f);
   }

   /* If no lights, still need to emit the scenecolor.
    */
   store_output_vec4(p, VARYING_SLOT_COL0, _col0);

   if (separate)
      store_output_vec4(p, VARYING_SLOT_COL1, _col1);

   if (twoside)
      store_output_vec4(p, VARYING_SLOT_BFC0, _bfc0);

   if (twoside && separate)
      store_output_vec4(p, VARYING_SLOT_BFC1, _bfc1);

   if (nr_lights == 0)
      return;

   /* Declare light products first to place them sequentially next to each
    * other for optimal constant uploads.
    */
   nir_def *lightprod_front[MAX_LIGHTS][3];
   nir_def *lightprod_back[MAX_LIGHTS][3];
   bool lightprod_front_is_state_light[MAX_LIGHTS][3];
   bool lightprod_back_is_state_light[MAX_LIGHTS][3];

   for (i = 0; i < MAX_LIGHTS; i++) {
      if (p->state->unit[i].light_enabled) {
         lightprod_front[i][0] = get_lightprod(p, i, 0, STATE_AMBIENT,
                                               &lightprod_front_is_state_light[i][0]);
         if (twoside)
            lightprod_back[i][0] = get_lightprod(p, i, 1, STATE_AMBIENT,
                                                 &lightprod_back_is_state_light[i][0]);

         lightprod_front[i][1] = get_lightprod(p, i, 0, STATE_DIFFUSE,
                                               &lightprod_front_is_state_light[i][1]);
         if (twoside)
            lightprod_back[i][1] = get_lightprod(p, i, 1, STATE_DIFFUSE,
                                                 &lightprod_back_is_state_light[i][1]);

         lightprod_front[i][2] = get_lightprod(p, i, 0, STATE_SPECULAR,
                                               &lightprod_front_is_state_light[i][2]);
         if (twoside)
            lightprod_back[i][2] = get_lightprod(p, i, 1, STATE_SPECULAR,
                                                 &lightprod_back_is_state_light[i][2]);
      }
   }

   /* Add more variables now that we'll use later, so that they are nicely
    * sorted in the parameter list.
    */
   for (i = 0; i < MAX_LIGHTS; i++) {
      if (p->state->unit[i].light_enabled) {
         if (p->state->unit[i].light_eyepos3_is_zero)
            register_state_var(p, STATE_LIGHT_POSITION_NORMALIZED,
                               i, 0, 0,
                               glsl_vector_type(GLSL_TYPE_FLOAT, 3));
         else
            register_state_var(p, STATE_LIGHT_POSITION, i, 0, 0,
                               glsl_vec4_type());
      }
   }
   for (i = 0; i < MAX_LIGHTS; i++) {
      if (p->state->unit[i].light_enabled &&
          (!p->state->unit[i].light_spotcutoff_is_180 ||
           (p->state->unit[i].light_attenuated &&
            !p->state->unit[i].light_eyepos3_is_zero)))
         register_state_var(p, STATE_LIGHT, i, STATE_ATTENUATION, 0,
                            glsl_vec4_type());
   }

   for (i = 0; i < MAX_LIGHTS; i++) {
      if (p->state->unit[i].light_enabled) {
         nir_def *half = NULL;
         nir_def *att = NULL, *VPpli = NULL;
         nir_def *dist = NULL;

         if (p->state->unit[i].light_eyepos3_is_zero) {
            VPpli = load_state_var(p, STATE_LIGHT_POSITION_NORMALIZED,
                                   i, 0, 0,
                                   glsl_vector_type(GLSL_TYPE_FLOAT, 3));
         } else {
            nir_def *Ppli =
               load_state_vec4(p, STATE_LIGHT_POSITION, i, 0, 0);

            nir_def *V = get_eye_position(p);
            VPpli = nir_fsub(p->b, Ppli, V);

            /* Normalize VPpli.  The dist value also used in
             * attenuation below.
             */
            dist = nir_frsq(p->b, nir_fdot3(p->b, VPpli, VPpli));
            VPpli = nir_fmul(p->b, VPpli, dist);
         }

         /* Calculate attenuation:
          */
         att = calculate_light_attenuation(p, i, VPpli, dist);

         /* Calculate viewer direction, or use infinite viewer:
          */
         if (!p->state->material_shininess_is_zero) {
            if (p->state->light_local_viewer) {
               nir_def *eye_hat = get_eye_position_normalized(p);
               half = emit_normalize_vec3(p->b,
                                          nir_fsub(p->b, VPpli, eye_hat));
            } else if (p->state->unit[i].light_eyepos3_is_zero) {
               half =
                  load_state_var(p, STATE_LIGHT_HALF_VECTOR,
                                 i, 0, 0,
                                 glsl_vector_type(GLSL_TYPE_FLOAT, 3));
            } else {
               nir_def *tmp =
                  nir_fadd(p->b,
                           VPpli,
                           nir_imm_vec3(p->b, 0.0f, 0.0f, 1.0f));
               half = emit_normalize_vec3(p->b, tmp);
            }
         }

         /* Calculate dot products:
          */
         nir_def *dot = nir_fdot3(p->b, normal, VPpli);
         if (p->state->material_shininess_is_zero) {
            dots = nir_replicate(p->b, dot, 4);
         } else {
            dots = nir_vector_insert_imm(p->b, dots, dot, 0);
            dot = nir_fdot3(p->b, normal, half);
            dots = nir_vector_insert_imm(p->b, dots, dot, 1);
         }

         /* Front face lighting:
          */
         {
            /* Transform STATE_LIGHT into STATE_LIGHTPROD if needed. This isn't done in
            * get_lightprod to avoid using too many temps.
            */
            for (int j = 0; j < 3; j++) {
               if (lightprod_front_is_state_light[i][j]) {
                  nir_def *material =
                     get_material(p, 0, STATE_AMBIENT + j);
                  lightprod_front[i][j] =
                     nir_fmul(p->b, lightprod_front[i][j], material);
               }
            }

            nir_def *ambient = lightprod_front[i][0];
            nir_def *diffuse = lightprod_front[i][1];
            nir_def *specular = lightprod_front[i][2];

            if (att) {
               /* light is attenuated by distance */
               lit = emit_lit(p->b, dots);
               lit = nir_fmul(p->b, lit, att);
               _col0 = nir_fmad(p->b, nir_channel(p->b, lit, 0), ambient, _col0);
            } else if (!p->state->material_shininess_is_zero) {
               /* there's a non-zero specular term */
               lit = emit_lit(p->b, dots);
               _col0 = nir_fadd(p->b, ambient, _col0);
            } else {
               /* no attenutation, no specular */
               lit = emit_degenerate_lit(p->b, dots);
               _col0 = nir_fadd(p->b, ambient, _col0);
            }

            _col0 = nir_fmad(p->b, nir_channel(p->b, lit, 1),
                             diffuse, _col0);
            if (separate)
               _col1 = nir_fmad(p->b, nir_channel(p->b, lit, 2),
                                specular, _col1);
            else
               _col0 = nir_fmad(p->b, nir_channel(p->b, lit, 2),
                                specular, _col0);
         }
         /* Back face lighting:
          */
         nir_def *old_dots = dots;
         if (twoside) {
            /* Transform STATE_LIGHT into STATE_LIGHTPROD if needed. This isn't done in
            * get_lightprod to avoid using too many temps.
            */
            for (int j = 0; j < 3; j++) {
               if (lightprod_back_is_state_light[i][j]) {
                  nir_def *material =
                     get_material(p, 1, STATE_AMBIENT + j);
                  lightprod_back[i][j] =
                     nir_fmul(p->b, lightprod_back[i][j], material);
               }
            }

            nir_def *ambient = lightprod_back[i][0];
            nir_def *diffuse = lightprod_back[i][1];
            nir_def *specular = lightprod_back[i][2];

            /* For the back face we need to negate the X and Y component
             * dot products.  dots.Z has the negated back-face specular
             * exponent.  We swizzle that into the W position.  This
             * negation makes the back-face specular term positive again.
             */
            unsigned swiz_xywz[] = {0, 1, 3, 2};
            nir_def *dots =
               nir_fneg(p->b, nir_swizzle(p->b, old_dots, swiz_xywz, 4));

            if (att) {
               /* light is attenuated by distance */
               lit = emit_lit(p->b, dots);
               lit = nir_fmul(p->b, lit, att);
               _bfc0 = nir_fmad(p->b, nir_channel(p->b, lit, 0), ambient, _bfc0);
            } else if (!p->state->material_shininess_is_zero) {
               /* there's a non-zero specular term */
               lit = emit_lit(p->b, dots);
               _bfc0 = nir_fadd(p->b, ambient, _bfc0);
            } else {
               /* no attenutation, no specular */
               lit = emit_degenerate_lit(p->b, dots);
               _bfc0 = nir_fadd(p->b, ambient, _bfc0);
            }

            _bfc0 = nir_fmad(p->b, nir_channel(p->b, lit, 1),
                             diffuse, _bfc0);
            if (separate)
               _bfc1 = nir_fmad(p->b, nir_channel(p->b, lit, 2),
                                specular, _bfc1);
            else
               _bfc0 = nir_fmad(p->b, nir_channel(p->b, lit, 2),
                                specular, _bfc0);
         }
      }
   }

   store_output_vec4_masked(p, VARYING_SLOT_COL0, _col0, 0x7);
   if (separate)
      store_output_vec4_masked(p, VARYING_SLOT_COL1, _col1, 0x7);

   if (twoside) {
      store_output_vec4_masked(p, VARYING_SLOT_BFC0, _bfc0, 0x7);
      if (separate)
         store_output_vec4_masked(p, VARYING_SLOT_BFC1, _bfc1, 0x7);
   }
}


static void build_fog( struct tnl_program *p )
{
   nir_def *fog;
   switch (p->state->fog_distance_mode) {
   case FDM_EYE_RADIAL:
      /* Z = sqrt(Xe*Xe + Ye*Ye + Ze*Ze) */
      fog = nir_fast_length(p->b,
                            nir_trim_vector(p->b, get_eye_position(p), 3));
      break;
   case FDM_EYE_PLANE: /* Z = Ze */
      fog = get_eye_position_z(p);
      break;
   case FDM_EYE_PLANE_ABS: /* Z = abs(Ze) */
      fog = nir_fabs(p->b, get_eye_position_z(p));
      break;
   case FDM_FROM_ARRAY:
      fog = load_input(p, VERT_ATTRIB_FOG, glsl_float_type());
      break;
   default:
      unreachable("Bad fog mode in build_fog()");
   }

   store_output_float(p, VARYING_SLOT_FOGC, fog);
}


static nir_def *
build_reflect_texgen(struct tnl_program *p)
{
   nir_def *normal = get_transformed_normal(p);
   nir_def *eye_hat = get_eye_position_normalized(p);
   /* n.u */
   nir_def *tmp = nir_fdot3(p->b, normal, eye_hat);
   /* 2n.u */
   tmp = nir_fadd(p->b, tmp, tmp);
   /* (-2n.u)n + u */
   return nir_fmad(p->b, nir_fneg(p->b, tmp), normal, eye_hat);
}


static nir_def *
build_sphere_texgen(struct tnl_program *p)
{
   nir_def *normal = get_transformed_normal(p);
   nir_def *eye_hat = get_eye_position_normalized(p);

   /* Could share the above calculations, but it would be
    * a fairly odd state for someone to set (both sphere and
    * reflection active for different texture coordinate
    * components.  Of course - if two texture units enable
    * reflect and/or sphere, things start to tilt in favour
    * of seperating this out:
    */

   /* n.u */
   nir_def *tmp = nir_fdot3(p->b, normal, eye_hat);
   /* 2n.u */
   tmp = nir_fadd(p->b, tmp, tmp);
   /* (-2n.u)n + u */
   nir_def *r = nir_fmad(p->b, nir_fneg(p->b, tmp), normal, eye_hat);
   /* r + 0,0,1 */
   tmp = nir_fadd(p->b, r, nir_imm_vec4(p->b, 0.0f, 0.0f, 1.0f, 0.0f));
   /* rx^2 + ry^2 + (rz+1)^2 */
   tmp = nir_fdot3(p->b, tmp, tmp);
   /* 2/m */
   tmp = nir_frsq(p->b, tmp);
   /* 1/m */
   nir_def *inv_m = nir_fmul_imm(p->b, tmp, 0.5f);
   /* r/m + 1/2 */
   return nir_fmad(p->b, r, inv_m, nir_imm_float(p->b, 0.5f));
}

static void build_texture_transform( struct tnl_program *p )
{
   GLuint i, j;

   for (i = 0; i < MAX_TEXTURE_COORD_UNITS; i++) {

      if (!(p->state->fragprog_inputs_read & VARYING_BIT_TEX(i)))
         continue;

      if (p->state->unit[i].coord_replace)
         continue;

      nir_def *texcoord;
      if (p->state->unit[i].texgen_enabled) {
         GLuint copy_mask = 0;
         GLuint sphere_mask = 0;
         GLuint reflect_mask = 0;
         GLuint normal_mask = 0;
         GLuint modes[4];
         nir_def *comps[4];

         modes[0] = p->state->unit[i].texgen_mode0;
         modes[1] = p->state->unit[i].texgen_mode1;
         modes[2] = p->state->unit[i].texgen_mode2;
         modes[3] = p->state->unit[i].texgen_mode3;

         for (j = 0; j < 4; j++) {
            switch (modes[j]) {
            case TXG_OBJ_LINEAR: {
               nir_def *obj = load_input_vec4(p, VERT_ATTRIB_POS);
               nir_def *plane =
                  load_state_vec4(p, STATE_TEXGEN, i,
                                  STATE_TEXGEN_OBJECT_S + j, 0);
               comps[j] = nir_fdot4(p->b, obj, plane);
               break;
            }
            case TXG_EYE_LINEAR: {
               nir_def *eye = get_eye_position(p);
               nir_def *plane =
                  load_state_vec4(p, STATE_TEXGEN, i,
                                  STATE_TEXGEN_EYE_S + j, 0);
               comps[j] = nir_fdot4(p->b, eye, plane);
               break;
            }
            case TXG_SPHERE_MAP:
               sphere_mask |= 1u << j;
               break;
            case TXG_REFLECTION_MAP:
               reflect_mask |= 1u << j;
               break;
            case TXG_NORMAL_MAP:
               normal_mask |= 1u << j;
               break;
            case TXG_NONE:
               copy_mask |= 1u << j;
            }
         }

         if (sphere_mask) {
            nir_def *sphere = build_sphere_texgen(p);
            for (j = 0; j < 4; j++)
               if (sphere_mask & (1 << j))
                  comps[j] = nir_channel(p->b, sphere, j);
         }

         if (reflect_mask) {
            nir_def *reflect = build_reflect_texgen(p);
            for (j = 0; j < 4; j++)
               if (reflect_mask & (1 << j))
                  comps[j] = nir_channel(p->b, reflect, j);
         }

         if (normal_mask) {
            nir_def *normal = get_transformed_normal(p);
            for (j = 0; j < 4; j++)
               if (normal_mask & (1 << j))
                  comps[j] = nir_channel(p->b, normal, j);
         }

         if (copy_mask) {
            nir_def *in = load_input_vec4(p, VERT_ATTRIB_TEX0 + i);
            for (j = 0; j < 4; j++)
               if (copy_mask & (1 << j))
                  comps[j] = nir_channel(p->b, in, j);
         }

         texcoord = nir_vec(p->b, comps, 4);
      } else
         texcoord = load_input_vec4(p, VERT_ATTRIB_TEX0 + i);

      if (p->state->unit[i].texmat_enabled) {
         nir_def *texmat[4];
         if (p->mvp_with_dp4) {
            load_state_mat4(p, texmat, STATE_TEXTURE_MATRIX, i);
            texcoord =
               emit_matrix_transform_vec4(p->b, texmat, texcoord);
         } else {
            load_state_mat4(p, texmat,
                            STATE_TEXTURE_MATRIX_TRANSPOSE, i);
            texcoord =
               emit_transpose_matrix_transform_vec4(p->b, texmat,
                                                      texcoord);
         }
      }

      store_output_vec4(p, VARYING_SLOT_TEX0 + i, texcoord);
   }
}


/**
 * Point size attenuation computation.
 */
static void build_atten_pointsize( struct tnl_program *p )
{
   nir_def *eye = get_eye_position_z(p);
   nir_def *in_size =
      load_state_vec4(p, STATE_POINT_SIZE_CLAMPED, 0, 0, 0);
   nir_def *att =
      load_state_vec4(p, STATE_POINT_ATTENUATION, 0, 0, 0);

   /* dist = |eyez| */
   nir_def *dist = nir_fabs(p->b, eye);

   /* p1 + dist * (p2 + dist * p3); */
   nir_def *factor = nir_fmad(p->b, dist, nir_channel(p->b, att, 2),
                                              nir_channel(p->b, att, 1));
   factor = nir_fmad(p->b, dist, factor, nir_channel(p->b, att, 0));

   /* 1 / sqrt(factor) */
   factor = nir_frsq(p->b, factor);

   /* pointSize / sqrt(factor) */
   nir_def *size = nir_fmul(p->b, factor,
                                nir_channel(p->b, in_size, 0));

#if 1
   /* this is a good place to clamp the point size since there's likely
    * no hardware registers to clamp point size at rasterization time.
    */
   size = nir_fclamp(p->b, size, nir_channel(p->b, in_size, 1),
                                 nir_channel(p->b, in_size, 2));
#endif

   store_output_float(p, VARYING_SLOT_PSIZ, size);
}


/**
 * Pass-though per-vertex point size, from user's point size array.
 */
static void build_array_pointsize( struct tnl_program *p )
{
   nir_def *val = load_input(p, VERT_ATTRIB_POINT_SIZE,
                                 glsl_float_type());
   store_output_float(p, VARYING_SLOT_PSIZ, val);
}


static void build_tnl_program( struct tnl_program *p )
{
   /* Emit the program (except for the MVP transform, which is a separate pass) */

   /* Lighting calculations:
    */
   if (p->state->fragprog_inputs_read &
       (VARYING_BIT_COL0 | VARYING_BIT_COL1)) {
      if (p->state->light_global_enabled)
         build_lighting(p);
      else {
         if (p->state->fragprog_inputs_read & VARYING_BIT_COL0)
            emit_passthrough(p, VERT_ATTRIB_COLOR0, VARYING_SLOT_COL0);

         if (p->state->fragprog_inputs_read & VARYING_BIT_COL1)
            emit_passthrough(p, VERT_ATTRIB_COLOR1, VARYING_SLOT_COL1);
      }
   }

   if (p->state->fragprog_inputs_read & VARYING_BIT_FOGC)
      build_fog(p);

   if (p->state->fragprog_inputs_read & VARYING_BITS_TEX_ANY)
      build_texture_transform(p);

   if (p->state->point_attenuated)
      build_atten_pointsize(p);
   else if (p->state->varying_vp_inputs & VERT_BIT_POINT_SIZE)
      build_array_pointsize(p);

   if (p->state->varying_vp_inputs & VERT_BIT_SELECT_RESULT_OFFSET)
      emit_passthrough(p, VERT_ATTRIB_SELECT_RESULT_OFFSET,
                       VARYING_SLOT_VAR0);
}


static nir_shader *
create_new_program( const struct state_key *key,
                    struct gl_program *program,
                    GLboolean mvp_with_dp4,
                    const nir_shader_compiler_options *options)
{
   struct tnl_program p;

   memset(&p, 0, sizeof(p));
   p.state = key;
   p.mvp_with_dp4 = mvp_with_dp4;

   program->Parameters = _mesa_new_parameter_list();
   p.state_params = _mesa_new_parameter_list();

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX,
                                                  options,
                                                  "ff-vs");

   nir_shader *s = b.shader;

   s->info.separate_shader = true;

   p.b = &b;

   build_tnl_program( &p );

   nir_validate_shader(b.shader, "after generating ff-vertex shader");

   /* Emit the MVP position transformation */
   NIR_PASS_V(b.shader, st_nir_lower_position_invariant, mvp_with_dp4, p.state_params);

   _mesa_add_separate_state_parameters(program, p.state_params);
   _mesa_free_parameter_list(p.state_params);

   return s;
}


/**
 * Return a vertex program which implements the current fixed-function
 * transform/lighting/texgen operations.
 */
struct gl_program *
_mesa_get_fixed_func_vertex_program(struct gl_context *ctx)
{
   struct gl_program *prog;
   struct state_key key;

   /* We only update ctx->VertexProgram._VaryingInputs when in VP_MODE_FF _VPMode */
   assert(VP_MODE_FF == ctx->VertexProgram._VPMode);

   /* Grab all the relevant state and put it in a single structure:
    */
   make_state_key(ctx, &key);

   /* Look for an already-prepared program for this state:
    */
   prog = _mesa_search_program_cache(ctx->VertexProgram.Cache, &key,
                                     sizeof(key));

   if (!prog) {
      /* OK, we'll have to build a new one */
      if (0)
         printf("Build new TNL program\n");

      prog = ctx->Driver.NewProgram(ctx, MESA_SHADER_VERTEX, 0, false);
      if (!prog)
         return NULL;

      const struct nir_shader_compiler_options *options =
         st_get_nir_compiler_options(ctx->st, MESA_SHADER_VERTEX);

      nir_shader *s =
         create_new_program( &key, prog,
                             ctx->Const.ShaderCompilerOptions[MESA_SHADER_VERTEX].OptimizeForAOS,
                             options);

      prog->state.type = PIPE_SHADER_IR_NIR;
      prog->nir = s;

      st_program_string_notify(ctx, GL_VERTEX_PROGRAM_ARB, prog);

      _mesa_program_cache_insert(ctx, ctx->VertexProgram.Cache, &key,
                                 sizeof(key), prog);
   }

   return prog;
}
