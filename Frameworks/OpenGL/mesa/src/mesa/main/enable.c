/**
 * \file enable.c
 * Enable/disable/query GL capabilities.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "util/glheader.h"
#include "arrayobj.h"
#include "blend.h"
#include "clip.h"
#include "context.h"
#include "debug_output.h"
#include "draw_validate.h"
#include "enable.h"
#include "errors.h"
#include "light.h"
#include "mtypes.h"
#include "enums.h"
#include "state.h"
#include "texstate.h"
#include "varray.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_bitmap.h"
#include "state_tracker/st_context.h"

void
_mesa_update_derived_primitive_restart_state(struct gl_context *ctx)
{
   if (ctx->Array.PrimitiveRestart ||
       ctx->Array.PrimitiveRestartFixedIndex) {
      unsigned restart_index[3] = {
         _mesa_primitive_restart_index(ctx, 1),
         _mesa_primitive_restart_index(ctx, 2),
         _mesa_primitive_restart_index(ctx, 4),
      };

      ctx->Array._RestartIndex[0] = restart_index[0];
      ctx->Array._RestartIndex[1] = restart_index[1];
      ctx->Array._RestartIndex[2] = restart_index[2];

      /* Enable primitive restart only when the restart index can have an
       * effect. This is required for correctness in AMD GFX8 support.
       * Other hardware may also benefit from taking a faster, non-restart path
       * when possible.
       */
      ctx->Array._PrimitiveRestart[0] = true && restart_index[0] <= UINT8_MAX;
      ctx->Array._PrimitiveRestart[1] = true && restart_index[1] <= UINT16_MAX;
      ctx->Array._PrimitiveRestart[2] = true;
   } else {
      ctx->Array._PrimitiveRestart[0] = false;
      ctx->Array._PrimitiveRestart[1] = false;
      ctx->Array._PrimitiveRestart[2] = false;
   }
}


/**
 * Helper to enable/disable VAO client-side state.
 */
static void
vao_state(struct gl_context *ctx, struct gl_vertex_array_object* vao,
          gl_vert_attrib attr, GLboolean state)
{
   if (state)
      _mesa_enable_vertex_array_attrib(ctx, vao, attr);
   else
      _mesa_disable_vertex_array_attrib(ctx, vao, attr);
}


/**
 * Helper to enable/disable client-side state.
 */
static void
client_state(struct gl_context *ctx, struct gl_vertex_array_object* vao,
             GLenum cap, GLboolean state)
{
   switch (cap) {
      case GL_VERTEX_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_POS, state);
         break;
      case GL_NORMAL_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_NORMAL, state);
         break;
      case GL_COLOR_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_COLOR0, state);
         break;
      case GL_INDEX_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_COLOR_INDEX, state);
         break;
      case GL_TEXTURE_COORD_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_TEX(ctx->Array.ActiveTexture), state);
         break;
      case GL_EDGE_FLAG_ARRAY:
         vao_state(ctx, vao, VERT_ATTRIB_EDGEFLAG, state);
         break;
      case GL_FOG_COORDINATE_ARRAY_EXT:
         vao_state(ctx, vao, VERT_ATTRIB_FOG, state);
         break;
      case GL_SECONDARY_COLOR_ARRAY_EXT:
         vao_state(ctx, vao, VERT_ATTRIB_COLOR1, state);
         break;

      case GL_POINT_SIZE_ARRAY_OES:
         if (ctx->VertexProgram.PointSizeEnabled != state) {
            FLUSH_VERTICES(ctx, ctx->st->lower_point_size ? _NEW_PROGRAM : 0,
                           0);
            ctx->NewDriverState |= ST_NEW_RASTERIZER;
            ctx->VertexProgram.PointSizeEnabled = state;
         }
         vao_state(ctx, vao, VERT_ATTRIB_POINT_SIZE, state);
         break;

      /* GL_NV_primitive_restart */
      case GL_PRIMITIVE_RESTART_NV:
         if (!_mesa_has_NV_primitive_restart(ctx))
            goto invalid_enum_error;
         if (ctx->Array.PrimitiveRestart == state)
            return;

         ctx->Array.PrimitiveRestart = state;
         _mesa_update_derived_primitive_restart_state(ctx);
         return;

      default:
         goto invalid_enum_error;
   }
   return;

invalid_enum_error:
   _mesa_error(ctx, GL_INVALID_ENUM, "gl%sClientState(%s)",
               state ? "Enable" : "Disable", _mesa_enum_to_string(cap));
}


/* Helper for GL_EXT_direct_state_access following functions:
 *   - EnableClientStateIndexedEXT
 *   - EnableClientStateiEXT
 *   - DisableClientStateIndexedEXT
 *   - DisableClientStateiEXT
 */
static void
client_state_i(struct gl_context *ctx, struct gl_vertex_array_object* vao,
               GLenum cap, GLuint index, GLboolean state)
{
   int saved_active;

   if (cap != GL_TEXTURE_COORD_ARRAY) {
      _mesa_error(ctx, GL_INVALID_ENUM, "gl%sClientStateiEXT(cap=%s)",
         state ? "Enable" : "Disable",
         _mesa_enum_to_string(cap));
      return;
   }

   if (index >= ctx->Const.MaxTextureCoordUnits) {
      _mesa_error(ctx, GL_INVALID_VALUE, "gl%sClientStateiEXT(index=%d)",
         state ? "Enable" : "Disable",
         index);
      return;
   }

   saved_active = ctx->Array.ActiveTexture;
   _mesa_ClientActiveTexture(GL_TEXTURE0 + index);
   client_state(ctx, vao, cap, state);
   _mesa_ClientActiveTexture(GL_TEXTURE0 + saved_active);
}


/**
 * Enable GL capability.
 * \param cap  state to enable/disable.
 *
 * Get's the current context, assures that we're outside glBegin()/glEnd() and
 * calls client_state().
 */
void GLAPIENTRY
_mesa_EnableClientState( GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);
   client_state( ctx, ctx->Array.VAO, cap, GL_TRUE );
}


void GLAPIENTRY
_mesa_EnableVertexArrayEXT( GLuint vaobj, GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_vertex_array_object* vao = _mesa_lookup_vao_err(ctx, vaobj,
                                                             true,
                                                             "glEnableVertexArrayEXT");
   if (!vao)
      return;

   /* The EXT_direct_state_access spec says:
    *    "Additionally EnableVertexArrayEXT and DisableVertexArrayEXT accept
    *    the tokens TEXTURE0 through TEXTUREn where n is less than the
    *    implementation-dependent limit of MAX_TEXTURE_COORDS.  For these
    *    GL_TEXTUREi tokens, EnableVertexArrayEXT and DisableVertexArrayEXT
    *    act identically to EnableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY)
    *    or DisableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY) respectively
    *    as if the active client texture is set to texture coordinate set i
    *    based on the token TEXTUREi indicated by array."
    */
   if (GL_TEXTURE0 <= cap && cap < GL_TEXTURE0 + ctx->Const.MaxTextureCoordUnits) {
      GLuint saved_active = ctx->Array.ActiveTexture;
      _mesa_ClientActiveTexture(cap);
      client_state(ctx, vao, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
      _mesa_ClientActiveTexture(GL_TEXTURE0 + saved_active);
   } else {
      client_state(ctx, vao, cap, GL_TRUE);
   }
}


void GLAPIENTRY
_mesa_EnableClientStateiEXT( GLenum cap, GLuint index )
{
   GET_CURRENT_CONTEXT(ctx);
   client_state_i(ctx, ctx->Array.VAO, cap, index, GL_TRUE);
}


/**
 * Disable GL capability.
 * \param cap  state to enable/disable.
 *
 * Get's the current context, assures that we're outside glBegin()/glEnd() and
 * calls client_state().
 */
void GLAPIENTRY
_mesa_DisableClientState( GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);
   client_state( ctx, ctx->Array.VAO, cap, GL_FALSE );
}

void GLAPIENTRY
_mesa_DisableVertexArrayEXT( GLuint vaobj, GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_vertex_array_object* vao = _mesa_lookup_vao_err(ctx, vaobj,
                                                             true,
                                                             "glDisableVertexArrayEXT");
   if (!vao)
      return;

   /* The EXT_direct_state_access spec says:
    *    "Additionally EnableVertexArrayEXT and DisableVertexArrayEXT accept
    *    the tokens TEXTURE0 through TEXTUREn where n is less than the
    *    implementation-dependent limit of MAX_TEXTURE_COORDS.  For these
    *    GL_TEXTUREi tokens, EnableVertexArrayEXT and DisableVertexArrayEXT
    *    act identically to EnableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY)
    *    or DisableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY) respectively
    *    as if the active client texture is set to texture coordinate set i
    *    based on the token TEXTUREi indicated by array."
    */
   if (GL_TEXTURE0 <= cap && cap < GL_TEXTURE0 + ctx->Const.MaxTextureCoordUnits) {
      GLuint saved_active = ctx->Array.ActiveTexture;
      _mesa_ClientActiveTexture(cap);
      client_state(ctx, vao, GL_TEXTURE_COORD_ARRAY, GL_FALSE);
      _mesa_ClientActiveTexture(GL_TEXTURE0 + saved_active);
   } else {
      client_state(ctx, vao, cap, GL_FALSE);
   }
}

void GLAPIENTRY
_mesa_DisableClientStateiEXT( GLenum cap, GLuint index )
{
   GET_CURRENT_CONTEXT(ctx);
   client_state_i(ctx, ctx->Array.VAO, cap, index, GL_FALSE);
}

/**
 * Return pointer to current texture unit for setting/getting coordinate
 * state.
 * Note that we'll set GL_INVALID_OPERATION and return NULL if the active
 * texture unit is higher than the number of supported coordinate units.
 */
static struct gl_fixedfunc_texture_unit *
get_texcoord_unit(struct gl_context *ctx)
{
   if (ctx->Texture.CurrentUnit >= ctx->Const.MaxTextureCoordUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glEnable/Disable(texcoord unit)");
      return NULL;
   }
   else {
      return &ctx->Texture.FixedFuncUnit[ctx->Texture.CurrentUnit];
   }
}


/**
 * Helper function to enable or disable a texture target.
 * \param bit  one of the TEXTURE_x_BIT values
 * \return GL_TRUE if state is changing or GL_FALSE if no change
 */
static GLboolean
enable_texture(struct gl_context *ctx, GLboolean state, GLbitfield texBit)
{
   struct gl_fixedfunc_texture_unit *texUnit =
      _mesa_get_fixedfunc_tex_unit(ctx, ctx->Texture.CurrentUnit);
   if (!texUnit)
      return GL_FALSE;

   const GLbitfield newenabled = state
      ? (texUnit->Enabled | texBit) : (texUnit->Enabled & ~texBit);

   if (texUnit->Enabled == newenabled)
       return GL_FALSE;

   FLUSH_VERTICES(ctx, _NEW_TEXTURE_STATE, GL_TEXTURE_BIT | GL_ENABLE_BIT);
   texUnit->Enabled = newenabled;
   return GL_TRUE;
}


/**
 * Helper function to enable or disable GL_MULTISAMPLE, skipping the check for
 * whether the API supports it (GLES doesn't).
 */
void
_mesa_set_multisample(struct gl_context *ctx, GLboolean state)
{
   if (ctx->Multisample.Enabled == state)
      return;

   /* GL compatibility needs Multisample.Enable to determine program state
    * constants.
    */
   if (_mesa_is_desktop_gl_compat(ctx) || _mesa_is_gles1(ctx)) {
      FLUSH_VERTICES(ctx, _NEW_MULTISAMPLE, GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
   } else {
      FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
   }

   ctx->NewDriverState |= ctx->DriverFlags.NewMultisampleEnable;
   ctx->Multisample.Enabled = state;
}

/**
 * Helper function to enable or disable GL_FRAMEBUFFER_SRGB, skipping the
 * check for whether the API supports it (GLES doesn't).
 */
void
_mesa_set_framebuffer_srgb(struct gl_context *ctx, GLboolean state)
{
   if (ctx->Color.sRGBEnabled == state)
      return;

   /* TODO: Switch i965 to the new flag and remove the conditional */
   FLUSH_VERTICES(ctx, 0,
                  GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
   ctx->NewDriverState |= ST_NEW_FB_STATE;
   ctx->Color.sRGBEnabled = state;
}

/**
 * Helper function to enable or disable state.
 *
 * \param ctx GL context.
 * \param cap  the state to enable/disable
 * \param state whether to enable or disable the specified capability.
 *
 * Updates the current context and flushes the vertices as needed. For
 * capabilities associated with extensions it verifies that those extensions
 * are effectivly present before updating. Notifies the driver via
 * dd_function_table::Enable.
 */
void
_mesa_set_enable(struct gl_context *ctx, GLenum cap, GLboolean state)
{
   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s %s (newstate is %x)\n",
                  state ? "glEnable" : "glDisable",
                  _mesa_enum_to_string(cap),
                  ctx->NewState);

   switch (cap) {
      case GL_ALPHA_TEST:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Color.AlphaEnabled == state)
            return;
         /* AlphaEnabled is used by the fixed-func fragment program */
         FLUSH_VERTICES(ctx, _NEW_COLOR | _NEW_FF_FRAG_PROGRAM,
                        GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ctx->DriverFlags.NewAlphaTest;
         ctx->Color.AlphaEnabled = state;
         break;
      case GL_AUTO_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.AutoNormal == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.AutoNormal = state;
         break;
      case GL_BLEND:
         {
            GLbitfield newEnabled =
               state * ((1 << ctx->Const.MaxDrawBuffers) - 1);
            if (newEnabled != ctx->Color.BlendEnabled) {
               _mesa_flush_vertices_for_blend_adv(ctx, newEnabled,
                                               ctx->Color._AdvancedBlendMode);
               ctx->PopAttribState |= GL_ENABLE_BIT;
               ctx->Color.BlendEnabled = newEnabled;
               _mesa_update_allow_draw_out_of_order(ctx);
               _mesa_update_valid_to_render_state(ctx);
            }
         }
         break;
      case GL_CLIP_DISTANCE0: /* aka GL_CLIP_PLANE0 */
      case GL_CLIP_DISTANCE1:
      case GL_CLIP_DISTANCE2:
      case GL_CLIP_DISTANCE3:
      case GL_CLIP_DISTANCE4:
      case GL_CLIP_DISTANCE5:
      case GL_CLIP_DISTANCE6:
      case GL_CLIP_DISTANCE7:
         {
            const GLuint p = cap - GL_CLIP_DISTANCE0;

            if (p >= ctx->Const.MaxClipPlanes)
               goto invalid_enum_error;

            if ((ctx->Transform.ClipPlanesEnabled & (1 << p))
                == ((GLuint) state << p))
               return;

            /* The compatibility profile needs _NEW_TRANSFORM to transform
             * clip planes according to the projection matrix.
             */
            if (_mesa_is_desktop_gl_compat(ctx) || _mesa_is_gles1(ctx)) {
               FLUSH_VERTICES(ctx, _NEW_TRANSFORM,
                              GL_TRANSFORM_BIT | GL_ENABLE_BIT);
            } else {
               FLUSH_VERTICES(ctx, 0, GL_TRANSFORM_BIT | GL_ENABLE_BIT);
            }
            ctx->NewDriverState |= ctx->DriverFlags.NewClipPlaneEnable;

            if (state) {
               ctx->Transform.ClipPlanesEnabled |= (1 << p);

               /* The projection matrix transforms the clip plane. */
               /* TODO: glEnable might not be the best place to do it. */
               if (_mesa_is_desktop_gl_compat(ctx) || _mesa_is_gles1(ctx)) {
                  _mesa_update_clip_plane(ctx, p);
                  ctx->NewDriverState |= ST_NEW_CLIP_STATE;
               }
            }
            else {
               ctx->Transform.ClipPlanesEnabled &= ~(1 << p);
            }
         }
         break;
      case GL_COLOR_MATERIAL:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Light.ColorMaterialEnabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM,
                        GL_LIGHTING_BIT | GL_ENABLE_BIT);
         FLUSH_CURRENT(ctx, 0);
         ctx->Light.ColorMaterialEnabled = state;
         if (state) {
            _mesa_update_color_material( ctx,
                                  ctx->Current.Attrib[VERT_ATTRIB_COLOR0] );
         }
         break;
      case GL_CULL_FACE:
         if (ctx->Polygon.CullFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.CullFlag = state;
         break;
      case GL_DEPTH_TEST:
         if (ctx->Depth.Test == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Depth.Test = state;
         _mesa_update_allow_draw_out_of_order(ctx);
         break;
      case GL_DEBUG_OUTPUT:
      case GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB:
         _mesa_set_debug_state_int(ctx, cap, state);
         _mesa_update_debug_callback(ctx);
         break;
      case GL_DITHER:
         if (ctx->Color.DitherFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Color.DitherFlag = state;
         break;
      case GL_FOG:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Fog.Enabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_FOG | _NEW_FF_FRAG_PROGRAM,
                        GL_FOG_BIT | GL_ENABLE_BIT);
         ctx->Fog.Enabled = state;
         ctx->Fog._PackedEnabledMode = state ? ctx->Fog._PackedMode : FOG_NONE;
         break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Light.Light[cap-GL_LIGHT0].Enabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM,
                        GL_LIGHTING_BIT | GL_ENABLE_BIT);
         ctx->Light.Light[cap-GL_LIGHT0].Enabled = state;
         if (state) {
            ctx->Light._EnabledLights |= 1u << (cap - GL_LIGHT0);
         }
         else {
            ctx->Light._EnabledLights &= ~(1u << (cap - GL_LIGHT0));
         }
         break;
      case GL_LIGHTING:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Light.Enabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM |
                        _NEW_FF_FRAG_PROGRAM | _NEW_LIGHT_STATE,
                        GL_LIGHTING_BIT | GL_ENABLE_BIT);
         ctx->Light.Enabled = state;
         break;
      case GL_LINE_SMOOTH:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Line.SmoothFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_LINE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Line.SmoothFlag = state;
         break;
      case GL_LINE_STIPPLE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Line.StippleFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_LINE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Line.StippleFlag = state;
         break;
      case GL_INDEX_LOGIC_OP:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Color.IndexLogicOpEnabled == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Color.IndexLogicOpEnabled = state;
         break;
      case GL_CONSERVATIVE_RASTERIZATION_INTEL:
         if (!_mesa_has_INTEL_conservative_rasterization(ctx))
            goto invalid_enum_error;
         if (ctx->IntelConservativeRasterization == state)
            return;
         FLUSH_VERTICES(ctx, 0, 0);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->IntelConservativeRasterization = state;
         _mesa_update_valid_to_render_state(ctx);
         break;
      case GL_CONSERVATIVE_RASTERIZATION_NV:
         if (!_mesa_has_NV_conservative_raster(ctx))
            goto invalid_enum_error;
         if (ctx->ConservativeRasterization == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->ConservativeRasterization = state;
         break;
      case GL_COLOR_LOGIC_OP:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Color.ColorLogicOpEnabled == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Color.ColorLogicOpEnabled = state;
         _mesa_update_allow_draw_out_of_order(ctx);
         break;
      case GL_MAP1_COLOR_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1Color4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1Color4 = state;
         break;
      case GL_MAP1_INDEX:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1Index == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1Index = state;
         break;
      case GL_MAP1_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1Normal == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1Normal = state;
         break;
      case GL_MAP1_TEXTURE_COORD_1:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1TextureCoord1 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1TextureCoord1 = state;
         break;
      case GL_MAP1_TEXTURE_COORD_2:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1TextureCoord2 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1TextureCoord2 = state;
         break;
      case GL_MAP1_TEXTURE_COORD_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1TextureCoord3 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1TextureCoord3 = state;
         break;
      case GL_MAP1_TEXTURE_COORD_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1TextureCoord4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1TextureCoord4 = state;
         break;
      case GL_MAP1_VERTEX_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1Vertex3 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1Vertex3 = state;
         break;
      case GL_MAP1_VERTEX_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map1Vertex4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map1Vertex4 = state;
         break;
      case GL_MAP2_COLOR_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2Color4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2Color4 = state;
         break;
      case GL_MAP2_INDEX:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2Index == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2Index = state;
         break;
      case GL_MAP2_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2Normal == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2Normal = state;
         break;
      case GL_MAP2_TEXTURE_COORD_1:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2TextureCoord1 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2TextureCoord1 = state;
         break;
      case GL_MAP2_TEXTURE_COORD_2:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2TextureCoord2 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2TextureCoord2 = state;
         break;
      case GL_MAP2_TEXTURE_COORD_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2TextureCoord3 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2TextureCoord3 = state;
         break;
      case GL_MAP2_TEXTURE_COORD_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2TextureCoord4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2TextureCoord4 = state;
         break;
      case GL_MAP2_VERTEX_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2Vertex3 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2Vertex3 = state;
         break;
      case GL_MAP2_VERTEX_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Eval.Map2Vertex4 == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_EVAL_BIT | GL_ENABLE_BIT);
         vbo_exec_update_eval_maps(ctx);
         ctx->Eval.Map2Vertex4 = state;
         break;
      case GL_NORMALIZE:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Transform.Normalize == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_TRANSFORM | _NEW_FF_VERT_PROGRAM,
                        GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->Transform.Normalize = state;
         break;
      case GL_POINT_SMOOTH:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Point.SmoothFlag == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT, GL_POINT_BIT | GL_ENABLE_BIT);
         ctx->Point.SmoothFlag = state;
         break;
      case GL_POLYGON_SMOOTH:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (ctx->Polygon.SmoothFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.SmoothFlag = state;
         break;
      case GL_POLYGON_STIPPLE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Polygon.StippleFlag == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.StippleFlag = state;
         break;
      case GL_POLYGON_OFFSET_POINT:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (ctx->Polygon.OffsetPoint == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.OffsetPoint = state;
         break;
      case GL_POLYGON_OFFSET_LINE:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (ctx->Polygon.OffsetLine == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.OffsetLine = state;
         break;
      case GL_POLYGON_OFFSET_FILL:
         if (ctx->Polygon.OffsetFill == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_POLYGON_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Polygon.OffsetFill = state;
         break;
      case GL_RESCALE_NORMAL_EXT:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Transform.RescaleNormals == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_TRANSFORM | _NEW_FF_VERT_PROGRAM,
                        GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->Transform.RescaleNormals = state;
         break;
      case GL_SCISSOR_TEST:
         {
            /* Must expand glEnable to all scissors */
            GLbitfield newEnabled =
               state * ((1 << ctx->Const.MaxViewports) - 1);
            if (newEnabled != ctx->Scissor.EnableFlags) {
               FLUSH_VERTICES(ctx, 0,
                              GL_SCISSOR_BIT | GL_ENABLE_BIT);
               ctx->NewDriverState |= ST_NEW_SCISSOR | ST_NEW_RASTERIZER;
               ctx->Scissor.EnableFlags = newEnabled;
            }
         }
         break;
      case GL_STENCIL_TEST:
         if (ctx->Stencil.Enabled == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Stencil.Enabled = state;
         _mesa_update_allow_draw_out_of_order(ctx);
         break;
      case GL_TEXTURE_1D:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_1D_BIT)) {
            return;
         }
         break;
      case GL_TEXTURE_2D:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_2D_BIT)) {
            return;
         }
         break;
      case GL_TEXTURE_3D:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_3D_BIT)) {
            return;
         }
         break;
      case GL_TEXTURE_GEN_S:
      case GL_TEXTURE_GEN_T:
      case GL_TEXTURE_GEN_R:
      case GL_TEXTURE_GEN_Q:
         {
            struct gl_fixedfunc_texture_unit *texUnit = get_texcoord_unit(ctx);

            if (ctx->API != API_OPENGL_COMPAT)
               goto invalid_enum_error;

            if (texUnit) {
               GLbitfield coordBit = S_BIT << (cap - GL_TEXTURE_GEN_S);
               GLbitfield newenabled = texUnit->TexGenEnabled & ~coordBit;
               if (state)
                  newenabled |= coordBit;
               if (texUnit->TexGenEnabled == newenabled)
                  return;
               FLUSH_VERTICES(ctx, _NEW_TEXTURE_STATE | _NEW_FF_VERT_PROGRAM |
                              _NEW_FF_FRAG_PROGRAM,
                              GL_TEXTURE_BIT | GL_ENABLE_BIT);
               texUnit->TexGenEnabled = newenabled;
            }
         }
         break;

      case GL_TEXTURE_GEN_STR_OES:
         /* disable S, T, and R at the same time */
         {
            struct gl_fixedfunc_texture_unit *texUnit = get_texcoord_unit(ctx);

            if (ctx->API != API_OPENGLES)
               goto invalid_enum_error;

            if (texUnit) {
               GLuint newenabled =
                  texUnit->TexGenEnabled & ~STR_BITS;
               if (state)
                  newenabled |= STR_BITS;
               if (texUnit->TexGenEnabled == newenabled)
                  return;
               FLUSH_VERTICES(ctx, _NEW_TEXTURE_STATE | _NEW_FF_VERT_PROGRAM |
                              _NEW_FF_FRAG_PROGRAM, 0);
               texUnit->TexGenEnabled = newenabled;
            }
         }
         break;

      /* client-side state */
      case GL_VERTEX_ARRAY:
      case GL_NORMAL_ARRAY:
      case GL_COLOR_ARRAY:
      case GL_TEXTURE_COORD_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         client_state( ctx, ctx->Array.VAO, cap, state );
         return;
      case GL_INDEX_ARRAY:
      case GL_EDGE_FLAG_ARRAY:
      case GL_FOG_COORDINATE_ARRAY_EXT:
      case GL_SECONDARY_COLOR_ARRAY_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         client_state( ctx, ctx->Array.VAO, cap, state );
         return;
      case GL_POINT_SIZE_ARRAY_OES:
         if (ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         client_state( ctx, ctx->Array.VAO, cap, state );
         return;

      /* GL_ARB_texture_cube_map */
      case GL_TEXTURE_CUBE_MAP:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_CUBE_BIT)) {
            return;
         }
         break;

      /* GL_EXT_secondary_color */
      case GL_COLOR_SUM_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Fog.ColorSumEnabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_FOG | _NEW_FF_FRAG_PROGRAM,
                        GL_FOG_BIT | GL_ENABLE_BIT);
         ctx->Fog.ColorSumEnabled = state;
         break;

      /* GL_ARB_multisample */
      case GL_MULTISAMPLE_ARB:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         _mesa_set_multisample(ctx, state);
         return;
      case GL_SAMPLE_ALPHA_TO_COVERAGE_ARB:
         if (ctx->Multisample.SampleAlphaToCoverage == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Multisample.SampleAlphaToCoverage = state;
         break;
      case GL_SAMPLE_ALPHA_TO_ONE_ARB:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         if (ctx->Multisample.SampleAlphaToOne == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Multisample.SampleAlphaToOne = state;
         break;
      case GL_SAMPLE_COVERAGE_ARB:
         if (ctx->Multisample.SampleCoverage == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_SAMPLE_STATE;
         ctx->Multisample.SampleCoverage = state;
         break;
      case GL_SAMPLE_COVERAGE_INVERT_ARB:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (ctx->Multisample.SampleCoverageInvert == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT);
         ctx->NewDriverState |= ST_NEW_SAMPLE_STATE;
         ctx->Multisample.SampleCoverageInvert = state;
         break;

      /* GL_ARB_sample_shading */
      case GL_SAMPLE_SHADING:
         if (!_mesa_has_ARB_sample_shading(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (ctx->Multisample.SampleShading == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_MULTISAMPLE_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ctx->DriverFlags.NewSampleShading;
         ctx->Multisample.SampleShading = state;
         break;

      /* GL_IBM_rasterpos_clip */
      case GL_RASTER_POSITION_UNCLIPPED_IBM:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         if (ctx->Transform.RasterPositionUnclipped == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->Transform.RasterPositionUnclipped = state;
         break;

      /* GL_ARB_point_sprite */
      case GL_POINT_SPRITE:
         if (!(_mesa_is_desktop_gl_compat(ctx) &&
               _mesa_has_ARB_point_sprite(ctx)) &&
             !_mesa_has_OES_point_sprite(ctx))
            goto invalid_enum_error;
         if (ctx->Point.PointSprite == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_POINT | _NEW_FF_VERT_PROGRAM |
                        _NEW_FF_FRAG_PROGRAM, GL_POINT_BIT | GL_ENABLE_BIT);
         ctx->Point.PointSprite = state;
         break;

      case GL_VERTEX_PROGRAM_ARB:
         if (!_mesa_has_ARB_vertex_program(ctx))
            goto invalid_enum_error;
         if (ctx->VertexProgram.Enabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_PROGRAM, GL_ENABLE_BIT);
         ctx->VertexProgram.Enabled = state;
         _mesa_update_vertex_processing_mode(ctx);
         _mesa_update_valid_to_render_state(ctx);
         break;
      case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
         /* This was added with ARB_vertex_program, but it is also used with
          * GLSL vertex shaders on desktop.
          */
         if (!_mesa_has_ARB_vertex_program(ctx) &&
             ctx->API != API_OPENGL_CORE)
            goto invalid_enum_error;
         if (ctx->VertexProgram.PointSizeEnabled == state)
            return;
         FLUSH_VERTICES(ctx, ctx->st->lower_point_size ? _NEW_PROGRAM : 0,
                        GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->VertexProgram.PointSizeEnabled = state;
         break;
      case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
         if (!_mesa_has_ARB_vertex_program(ctx))
            goto invalid_enum_error;
         if (ctx->VertexProgram.TwoSideEnabled == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_ENABLE_BIT);
         if (ctx->st->lower_two_sided_color) {
            /* TODO: this could be smaller, but most drivers don't get here */
            ctx->NewDriverState |= ST_NEW_VS_STATE |
                                   ST_NEW_TES_STATE |
                                   ST_NEW_GS_STATE;
         }
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->VertexProgram.TwoSideEnabled = state;
         break;

      /* GL_NV_texture_rectangle */
      case GL_TEXTURE_RECTANGLE_NV:
         if (!_mesa_has_NV_texture_rectangle(ctx))
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_RECT_BIT)) {
            return;
         }
         break;

      /* GL_EXT_stencil_two_side */
      case GL_STENCIL_TEST_TWO_SIDE_EXT:
         if (!_mesa_has_EXT_stencil_two_side(ctx))
            goto invalid_enum_error;
         if (ctx->Stencil.TestTwoSide == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Stencil.TestTwoSide = state;
         if (state) {
            ctx->Stencil._BackFace = 2;
         } else {
            ctx->Stencil._BackFace = 1;
         }
         break;

      case GL_FRAGMENT_PROGRAM_ARB:
         if (!_mesa_has_ARB_fragment_program(ctx))
            goto invalid_enum_error;
         if (ctx->FragmentProgram.Enabled == state)
            return;
         FLUSH_VERTICES(ctx, _NEW_PROGRAM, GL_ENABLE_BIT);
         ctx->FragmentProgram.Enabled = state;
         _mesa_update_valid_to_render_state(ctx);
         break;

      /* GL_EXT_depth_bounds_test */
      case GL_DEPTH_BOUNDS_TEST_EXT:
         if (!_mesa_has_EXT_depth_bounds_test(ctx))
            goto invalid_enum_error;
         if (ctx->Depth.BoundsTest == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_DSA;
         ctx->Depth.BoundsTest = state;
         break;

      case GL_DEPTH_CLAMP:
         if (!_mesa_has_ARB_depth_clamp(ctx) &&
             !_mesa_has_EXT_depth_clamp(ctx))
            goto invalid_enum_error;
         if (ctx->Transform.DepthClampNear == state &&
             ctx->Transform.DepthClampFar == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Transform.DepthClampNear = state;
         ctx->Transform.DepthClampFar = state;
         break;

      case GL_DEPTH_CLAMP_NEAR_AMD:
         if (!_mesa_has_AMD_depth_clamp_separate(ctx))
            goto invalid_enum_error;
         if (ctx->Transform.DepthClampNear == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Transform.DepthClampNear = state;
         break;

      case GL_DEPTH_CLAMP_FAR_AMD:
         if (!_mesa_has_AMD_depth_clamp_separate(ctx))
            goto invalid_enum_error;
         if (ctx->Transform.DepthClampFar == state)
            return;
         FLUSH_VERTICES(ctx, 0,
                        GL_TRANSFORM_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_RASTERIZER;
         ctx->Transform.DepthClampFar = state;
         break;

      case GL_FRAGMENT_SHADER_ATI:
        if (!_mesa_has_ATI_fragment_shader(ctx))
           goto invalid_enum_error;
        if (ctx->ATIFragmentShader.Enabled == state)
           return;
        FLUSH_VERTICES(ctx, _NEW_PROGRAM, GL_ENABLE_BIT);
        ctx->ATIFragmentShader.Enabled = state;
        break;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (!_mesa_has_ARB_seamless_cube_map(ctx))
            goto invalid_enum_error;
         if (ctx->Texture.CubeMapSeamless != state) {
            FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, 0);
            ctx->Texture.CubeMapSeamless = state;
         }
         break;

      case GL_RASTERIZER_DISCARD:
         if (!(_mesa_has_EXT_transform_feedback(ctx) || _mesa_is_gles3(ctx)))
            goto invalid_enum_error;
         if (ctx->RasterDiscard != state) {
            FLUSH_VERTICES(ctx, 0, 0);
            ctx->NewDriverState |= ST_NEW_RASTERIZER;
            ctx->RasterDiscard = state;
         }
         break;

      case GL_TILE_RASTER_ORDER_FIXED_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         if (ctx->TileRasterOrderFixed != state) {
            FLUSH_VERTICES(ctx, 0, GL_ENABLE_BIT);
            ctx->NewDriverState |= ST_NEW_RASTERIZER;
            ctx->TileRasterOrderFixed = state;
         }
         break;

      case GL_TILE_RASTER_ORDER_INCREASING_X_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         if (ctx->TileRasterOrderIncreasingX != state) {
            FLUSH_VERTICES(ctx, 0, GL_ENABLE_BIT);
            ctx->NewDriverState |= ST_NEW_RASTERIZER;
            ctx->TileRasterOrderIncreasingX = state;
         }
         break;

      case GL_TILE_RASTER_ORDER_INCREASING_Y_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         if (ctx->TileRasterOrderIncreasingY != state) {
            FLUSH_VERTICES(ctx, 0, GL_ENABLE_BIT);
            ctx->NewDriverState |= ST_NEW_RASTERIZER;
            ctx->TileRasterOrderIncreasingY = state;
         }
         break;

      /* GL 3.1 primitive restart.  Note: this enum is different from
       * GL_PRIMITIVE_RESTART_NV (which is client state).
       */
      case GL_PRIMITIVE_RESTART:
         if (!_mesa_is_desktop_gl(ctx) || ctx->Version < 31) {
            goto invalid_enum_error;
         }
         if (ctx->Array.PrimitiveRestart != state) {
            ctx->Array.PrimitiveRestart = state;
            _mesa_update_derived_primitive_restart_state(ctx);
         }
         break;

      case GL_PRIMITIVE_RESTART_FIXED_INDEX:
         if (!_mesa_is_gles3(ctx) && !_mesa_has_ARB_ES3_compatibility(ctx))
            goto invalid_enum_error;
         if (ctx->Array.PrimitiveRestartFixedIndex != state) {
            ctx->Array.PrimitiveRestartFixedIndex = state;
            _mesa_update_derived_primitive_restart_state(ctx);
         }
         break;

      /* GL3.0 - GL_framebuffer_sRGB */
      case GL_FRAMEBUFFER_SRGB_EXT:
         if (!_mesa_has_EXT_framebuffer_sRGB(ctx) &&
             !_mesa_has_EXT_sRGB_write_control(ctx))
            goto invalid_enum_error;
         _mesa_set_framebuffer_srgb(ctx, state);
         return;

      /* GL_OES_EGL_image_external */
      case GL_TEXTURE_EXTERNAL_OES:
         if (!_mesa_has_OES_EGL_image_external(ctx))
            goto invalid_enum_error;
         if (!enable_texture(ctx, state, TEXTURE_EXTERNAL_BIT)) {
            return;
         }
         break;

      /* ARB_texture_multisample */
      case GL_SAMPLE_MASK:
         if (!_mesa_has_ARB_texture_multisample(ctx) && !_mesa_is_gles31(ctx))
            goto invalid_enum_error;
         if (ctx->Multisample.SampleMask == state)
            return;
         FLUSH_VERTICES(ctx, 0, 0);
         ctx->NewDriverState |= ST_NEW_SAMPLE_STATE;
         ctx->Multisample.SampleMask = state;
         break;

      case GL_BLEND_ADVANCED_COHERENT_KHR:
         if (!_mesa_has_KHR_blend_equation_advanced_coherent(ctx))
            goto invalid_enum_error;
         if (ctx->Color.BlendCoherent == state)
            return;
         FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
         ctx->NewDriverState |= ST_NEW_BLEND;
         ctx->Color.BlendCoherent = state;
         break;

      case GL_BLACKHOLE_RENDER_INTEL:
         if (!_mesa_has_INTEL_blackhole_render(ctx))
            goto invalid_enum_error;
         if (ctx->IntelBlackholeRender == state)
            return;
         FLUSH_VERTICES(ctx, 0, 0);
         ctx->IntelBlackholeRender = state;
         ctx->pipe->set_frontend_noop(ctx->pipe, state);
         break;

      default:
         goto invalid_enum_error;
   }
   return;

invalid_enum_error:
   _mesa_error(ctx, GL_INVALID_ENUM, "gl%s(%s)",
               state ? "Enable" : "Disable", _mesa_enum_to_string(cap));
}


/**
 * Enable GL capability.  Called by glEnable()
 * \param cap  state to enable.
 */
void GLAPIENTRY
_mesa_Enable( GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);

   _mesa_set_enable( ctx, cap, GL_TRUE );
}


/**
 * Disable GL capability.  Called by glDisable()
 * \param cap  state to disable.
 */
void GLAPIENTRY
_mesa_Disable( GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);

   _mesa_set_enable( ctx, cap, GL_FALSE );
}



/**
 * Enable/disable an indexed state var.
 */
void
_mesa_set_enablei(struct gl_context *ctx, GLenum cap,
                  GLuint index, GLboolean state)
{
   assert(state == 0 || state == 1);
   switch (cap) {
   case GL_BLEND:
      if (!ctx->Extensions.EXT_draw_buffers2) {
         goto invalid_enum_error;
      }
      if (index >= ctx->Const.MaxDrawBuffers) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index=%u)",
                     state ? "glEnableIndexed" : "glDisableIndexed", index);
         return;
      }
      if (((ctx->Color.BlendEnabled >> index) & 1) != state) {
         GLbitfield enabled = ctx->Color.BlendEnabled;

         if (state)
            enabled |= (1 << index);
         else
            enabled &= ~(1 << index);

         _mesa_flush_vertices_for_blend_adv(ctx, enabled,
                                            ctx->Color._AdvancedBlendMode);
         ctx->PopAttribState |= GL_ENABLE_BIT;
         ctx->Color.BlendEnabled = enabled;
         _mesa_update_allow_draw_out_of_order(ctx);
         _mesa_update_valid_to_render_state(ctx);
      }
      break;
   case GL_SCISSOR_TEST:
      if (index >= ctx->Const.MaxViewports) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index=%u)",
                     state ? "glEnablei" : "glDisablei", index);
         return;
      }
      if (((ctx->Scissor.EnableFlags >> index) & 1) != state) {
         FLUSH_VERTICES(ctx, 0,
                        GL_SCISSOR_BIT | GL_ENABLE_BIT);
         ctx->NewDriverState |= ST_NEW_SCISSOR | ST_NEW_RASTERIZER;
         if (state)
            ctx->Scissor.EnableFlags |= (1 << index);
         else
            ctx->Scissor.EnableFlags &= ~(1 << index);
      }
      break;
   /* EXT_direct_state_access */
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_GEN_S:
   case GL_TEXTURE_GEN_T:
   case GL_TEXTURE_GEN_R:
   case GL_TEXTURE_GEN_Q:
   case GL_TEXTURE_RECTANGLE_ARB: {
      const GLuint curTexUnitSave = ctx->Texture.CurrentUnit;
      if (index >= MAX2(ctx->Const.MaxCombinedTextureImageUnits,
                        ctx->Const.MaxTextureCoordUnits)) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(index=%u)",
                     state ? "glEnablei" : "glDisablei", index);
         return;
      }
      _mesa_ActiveTexture(GL_TEXTURE0 + index);
      _mesa_set_enable( ctx, cap, state );
      _mesa_ActiveTexture(GL_TEXTURE0 + curTexUnitSave);
      break;
   }
   default:
      goto invalid_enum_error;
   }
   return;

invalid_enum_error:
    _mesa_error(ctx, GL_INVALID_ENUM, "%s(cap=%s)",
                state ? "glEnablei" : "glDisablei",
                _mesa_enum_to_string(cap));
}


void GLAPIENTRY
_mesa_Disablei( GLenum cap, GLuint index )
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_set_enablei(ctx, cap, index, GL_FALSE);
}


void GLAPIENTRY
_mesa_Enablei( GLenum cap, GLuint index )
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_set_enablei(ctx, cap, index, GL_TRUE);
}


GLboolean GLAPIENTRY
_mesa_IsEnabledi( GLenum cap, GLuint index )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, 0);
   switch (cap) {
   case GL_BLEND:
      if (index >= ctx->Const.MaxDrawBuffers) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glIsEnabledIndexed(index=%u)",
                     index);
         return GL_FALSE;
      }
      return (ctx->Color.BlendEnabled >> index) & 1;
   case GL_SCISSOR_TEST:
      if (index >= ctx->Const.MaxViewports) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glIsEnabledIndexed(index=%u)",
                     index);
         return GL_FALSE;
      }
      return (ctx->Scissor.EnableFlags >> index) & 1;
   /* EXT_direct_state_access */
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_GEN_S:
   case GL_TEXTURE_GEN_T:
   case GL_TEXTURE_GEN_R:
   case GL_TEXTURE_GEN_Q:
   case GL_TEXTURE_RECTANGLE_ARB: {
      GLboolean state;
      const GLuint curTexUnitSave = ctx->Texture.CurrentUnit;
      if (index >= MAX2(ctx->Const.MaxCombinedTextureImageUnits,
                        ctx->Const.MaxTextureCoordUnits)) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glIsEnabledIndexed(index=%u)",
                     index);
         return GL_FALSE;
      }
      _mesa_ActiveTexture(GL_TEXTURE0 + index);
      state = _mesa_IsEnabled(cap);
      _mesa_ActiveTexture(GL_TEXTURE0 + curTexUnitSave);
      return state;
   }
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glIsEnabledIndexed(cap=%s)",
                  _mesa_enum_to_string(cap));
      return GL_FALSE;
   }
}



/**
 * Helper function to determine whether a texture target is enabled.
 */
static GLboolean
is_texture_enabled(struct gl_context *ctx, GLbitfield bit)
{
   const struct gl_fixedfunc_texture_unit *const texUnit =
      _mesa_get_fixedfunc_tex_unit(ctx, ctx->Texture.CurrentUnit);

   if (!texUnit)
      return GL_FALSE;

   return (texUnit->Enabled & bit) ? GL_TRUE : GL_FALSE;
}


/**
 * Return simple enable/disable state.
 *
 * \param cap  state variable to query.
 *
 * Returns the state of the specified capability from the current GL context.
 * For the capabilities associated with extensions verifies that those
 * extensions are effectively present before reporting.
 */
GLboolean GLAPIENTRY
_mesa_IsEnabled( GLenum cap )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, 0);

   switch (cap) {
      case GL_ALPHA_TEST:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Color.AlphaEnabled;
      case GL_AUTO_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.AutoNormal;
      case GL_BLEND:
         return ctx->Color.BlendEnabled & 1;  /* return state for buffer[0] */
      case GL_CLIP_DISTANCE0: /* aka GL_CLIP_PLANE0 */
      case GL_CLIP_DISTANCE1:
      case GL_CLIP_DISTANCE2:
      case GL_CLIP_DISTANCE3:
      case GL_CLIP_DISTANCE4:
      case GL_CLIP_DISTANCE5:
      case GL_CLIP_DISTANCE6:
      case GL_CLIP_DISTANCE7: {
         const GLuint p = cap - GL_CLIP_DISTANCE0;

         if (p >= ctx->Const.MaxClipPlanes)
            goto invalid_enum_error;

         return (ctx->Transform.ClipPlanesEnabled >> p) & 1;
      }
      case GL_COLOR_MATERIAL:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Light.ColorMaterialEnabled;
      case GL_CULL_FACE:
         return ctx->Polygon.CullFlag;
      case GL_DEBUG_OUTPUT:
      case GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB:
         return (GLboolean) _mesa_get_debug_state_int(ctx, cap);
      case GL_DEPTH_TEST:
         return ctx->Depth.Test;
      case GL_DITHER:
         return ctx->Color.DitherFlag;
      case GL_FOG:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Fog.Enabled;
      case GL_LIGHTING:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Light.Enabled;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Light.Light[cap-GL_LIGHT0].Enabled;
      case GL_LINE_SMOOTH:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Line.SmoothFlag;
      case GL_LINE_STIPPLE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Line.StippleFlag;
      case GL_INDEX_LOGIC_OP:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Color.IndexLogicOpEnabled;
      case GL_COLOR_LOGIC_OP:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Color.ColorLogicOpEnabled;
      case GL_MAP1_COLOR_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1Color4;
      case GL_MAP1_INDEX:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1Index;
      case GL_MAP1_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1Normal;
      case GL_MAP1_TEXTURE_COORD_1:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1TextureCoord1;
      case GL_MAP1_TEXTURE_COORD_2:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1TextureCoord2;
      case GL_MAP1_TEXTURE_COORD_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1TextureCoord3;
      case GL_MAP1_TEXTURE_COORD_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1TextureCoord4;
      case GL_MAP1_VERTEX_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1Vertex3;
      case GL_MAP1_VERTEX_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map1Vertex4;
      case GL_MAP2_COLOR_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2Color4;
      case GL_MAP2_INDEX:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2Index;
      case GL_MAP2_NORMAL:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2Normal;
      case GL_MAP2_TEXTURE_COORD_1:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2TextureCoord1;
      case GL_MAP2_TEXTURE_COORD_2:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2TextureCoord2;
      case GL_MAP2_TEXTURE_COORD_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2TextureCoord3;
      case GL_MAP2_TEXTURE_COORD_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2TextureCoord4;
      case GL_MAP2_VERTEX_3:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2Vertex3;
      case GL_MAP2_VERTEX_4:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Eval.Map2Vertex4;
      case GL_NORMALIZE:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Transform.Normalize;
      case GL_POINT_SMOOTH:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Point.SmoothFlag;
      case GL_POLYGON_SMOOTH:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         return ctx->Polygon.SmoothFlag;
      case GL_POLYGON_STIPPLE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Polygon.StippleFlag;
      case GL_POLYGON_OFFSET_POINT:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         return ctx->Polygon.OffsetPoint;
      case GL_POLYGON_OFFSET_LINE:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         return ctx->Polygon.OffsetLine;
      case GL_POLYGON_OFFSET_FILL:
         return ctx->Polygon.OffsetFill;
      case GL_RESCALE_NORMAL_EXT:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Transform.RescaleNormals;
      case GL_SCISSOR_TEST:
         return ctx->Scissor.EnableFlags & 1;  /* return state for index 0 */
      case GL_STENCIL_TEST:
         return ctx->Stencil.Enabled;
      case GL_TEXTURE_1D:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_1D_BIT);
      case GL_TEXTURE_2D:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_2D_BIT);
      case GL_TEXTURE_3D:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_3D_BIT);
      case GL_TEXTURE_GEN_S:
      case GL_TEXTURE_GEN_T:
      case GL_TEXTURE_GEN_R:
      case GL_TEXTURE_GEN_Q:
         {
            const struct gl_fixedfunc_texture_unit *texUnit =
               get_texcoord_unit(ctx);

            if (ctx->API != API_OPENGL_COMPAT)
               goto invalid_enum_error;

            if (texUnit) {
               GLbitfield coordBit = S_BIT << (cap - GL_TEXTURE_GEN_S);
               return (texUnit->TexGenEnabled & coordBit) ? GL_TRUE : GL_FALSE;
            }
         }
         return GL_FALSE;
      case GL_TEXTURE_GEN_STR_OES:
         {
            const struct gl_fixedfunc_texture_unit *texUnit =
               get_texcoord_unit(ctx);

            if (ctx->API != API_OPENGLES)
               goto invalid_enum_error;

            if (texUnit) {
               return (texUnit->TexGenEnabled & STR_BITS) == STR_BITS
                  ? GL_TRUE : GL_FALSE;
            }

            return GL_FALSE;
         }

      /* client-side state */
      case GL_VERTEX_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_POS);
      case GL_NORMAL_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_NORMAL);
      case GL_COLOR_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_COLOR0);
      case GL_INDEX_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_COLOR_INDEX);
      case GL_TEXTURE_COORD_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled &
                   VERT_BIT_TEX(ctx->Array.ActiveTexture));
      case GL_EDGE_FLAG_ARRAY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_EDGEFLAG);
      case GL_FOG_COORDINATE_ARRAY_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_FOG);
      case GL_SECONDARY_COLOR_ARRAY_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_COLOR1);
      case GL_POINT_SIZE_ARRAY_OES:
         if (ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return !!(ctx->Array.VAO->Enabled & VERT_BIT_POINT_SIZE);

      /* GL_ARB_texture_cube_map */
      case GL_TEXTURE_CUBE_MAP:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_CUBE_BIT);

      /* GL_EXT_secondary_color */
      case GL_COLOR_SUM_EXT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Fog.ColorSumEnabled;

      /* GL_ARB_multisample */
      case GL_MULTISAMPLE_ARB:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Multisample.Enabled;
      case GL_SAMPLE_ALPHA_TO_COVERAGE_ARB:
         return ctx->Multisample.SampleAlphaToCoverage;
      case GL_SAMPLE_ALPHA_TO_ONE_ARB:
         if (!_mesa_is_desktop_gl(ctx) && ctx->API != API_OPENGLES)
            goto invalid_enum_error;
         return ctx->Multisample.SampleAlphaToOne;
      case GL_SAMPLE_COVERAGE_ARB:
         return ctx->Multisample.SampleCoverage;
      case GL_SAMPLE_COVERAGE_INVERT_ARB:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         return ctx->Multisample.SampleCoverageInvert;

      /* GL_IBM_rasterpos_clip */
      case GL_RASTER_POSITION_UNCLIPPED_IBM:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_enum_error;
         return ctx->Transform.RasterPositionUnclipped;

      /* GL_ARB_point_sprite */
      case GL_POINT_SPRITE:
         if (!(_mesa_is_desktop_gl_compat(ctx) &&
               _mesa_has_ARB_point_sprite(ctx)) &&
             !_mesa_has_OES_point_sprite(ctx))
            goto invalid_enum_error;
         return ctx->Point.PointSprite;

      case GL_VERTEX_PROGRAM_ARB:
         if (!_mesa_has_ARB_vertex_program(ctx))
            goto invalid_enum_error;
         return ctx->VertexProgram.Enabled;
      case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
         /* This was added with ARB_vertex_program, but it is also used with
          * GLSL vertex shaders on desktop.
          */
         if (!_mesa_has_ARB_vertex_program(ctx) &&
             ctx->API != API_OPENGL_CORE)
            goto invalid_enum_error;
         return ctx->VertexProgram.PointSizeEnabled;
      case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
         if (!_mesa_has_ARB_vertex_program(ctx))
            goto invalid_enum_error;
         return ctx->VertexProgram.TwoSideEnabled;

      /* GL_NV_texture_rectangle */
      case GL_TEXTURE_RECTANGLE_NV:
         if (!_mesa_has_NV_texture_rectangle(ctx))
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_RECT_BIT);

      /* GL_EXT_stencil_two_side */
      case GL_STENCIL_TEST_TWO_SIDE_EXT:
         if (!_mesa_has_EXT_stencil_two_side(ctx))
            goto invalid_enum_error;
         return ctx->Stencil.TestTwoSide;

      case GL_FRAGMENT_PROGRAM_ARB:
         if (!_mesa_has_ARB_fragment_program(ctx))
            goto invalid_enum_error;
         return ctx->FragmentProgram.Enabled;

      /* GL_EXT_depth_bounds_test */
      case GL_DEPTH_BOUNDS_TEST_EXT:
         if (!_mesa_has_EXT_depth_bounds_test(ctx))
            goto invalid_enum_error;
         return ctx->Depth.BoundsTest;

      /* GL_ARB_depth_clamp */
      case GL_DEPTH_CLAMP:
         if (!_mesa_has_ARB_depth_clamp(ctx) &&
             !_mesa_has_EXT_depth_clamp(ctx))
            goto invalid_enum_error;
         return ctx->Transform.DepthClampNear ||
                ctx->Transform.DepthClampFar;

      case GL_DEPTH_CLAMP_NEAR_AMD:
         if (!_mesa_has_AMD_depth_clamp_separate(ctx))
            goto invalid_enum_error;
         return ctx->Transform.DepthClampNear;

      case GL_DEPTH_CLAMP_FAR_AMD:
         if (!_mesa_has_AMD_depth_clamp_separate(ctx))
            goto invalid_enum_error;
         return ctx->Transform.DepthClampFar;

      case GL_FRAGMENT_SHADER_ATI:
         if (!_mesa_has_ATI_fragment_shader(ctx))
            goto invalid_enum_error;
         return ctx->ATIFragmentShader.Enabled;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (!_mesa_has_ARB_seamless_cube_map(ctx))
            goto invalid_enum_error;
         return ctx->Texture.CubeMapSeamless;

      case GL_RASTERIZER_DISCARD:
         if (!(_mesa_has_EXT_transform_feedback(ctx) || _mesa_is_gles3(ctx)))
            goto invalid_enum_error;
         return ctx->RasterDiscard;

      /* GL_NV_primitive_restart */
      case GL_PRIMITIVE_RESTART_NV:
         if (!_mesa_has_NV_primitive_restart(ctx))
            goto invalid_enum_error;
         return ctx->Array.PrimitiveRestart;

      /* GL 3.1 primitive restart */
      case GL_PRIMITIVE_RESTART:
         if (!_mesa_is_desktop_gl(ctx) || ctx->Version < 31) {
            goto invalid_enum_error;
         }
         return ctx->Array.PrimitiveRestart;

      case GL_PRIMITIVE_RESTART_FIXED_INDEX:
         if (!_mesa_is_gles3(ctx) && !_mesa_has_ARB_ES3_compatibility(ctx))
            goto invalid_enum_error;
         return ctx->Array.PrimitiveRestartFixedIndex;

      /* GL3.0 - GL_framebuffer_sRGB */
      case GL_FRAMEBUFFER_SRGB_EXT:
         if (!_mesa_has_EXT_framebuffer_sRGB(ctx) &&
             !_mesa_has_EXT_sRGB_write_control(ctx))
            goto invalid_enum_error;
         return ctx->Color.sRGBEnabled;

      /* GL_OES_EGL_image_external */
      case GL_TEXTURE_EXTERNAL_OES:
         if (!_mesa_has_OES_EGL_image_external(ctx))
            goto invalid_enum_error;
         return is_texture_enabled(ctx, TEXTURE_EXTERNAL_BIT);

      /* ARB_texture_multisample */
      case GL_SAMPLE_MASK:
         if (!_mesa_has_ARB_texture_multisample(ctx) && !_mesa_is_gles31(ctx))
            goto invalid_enum_error;
         return ctx->Multisample.SampleMask;

      /* ARB_sample_shading */
      case GL_SAMPLE_SHADING:
         if (!_mesa_has_ARB_sample_shading(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         return ctx->Multisample.SampleShading;

      case GL_BLEND_ADVANCED_COHERENT_KHR:
         if (!_mesa_has_KHR_blend_equation_advanced_coherent(ctx))
            goto invalid_enum_error;
         return ctx->Color.BlendCoherent;

      case GL_CONSERVATIVE_RASTERIZATION_INTEL:
         if (!_mesa_has_INTEL_conservative_rasterization(ctx))
            goto invalid_enum_error;
         return ctx->IntelConservativeRasterization;

      case GL_CONSERVATIVE_RASTERIZATION_NV:
         if (!_mesa_has_NV_conservative_raster(ctx))
            goto invalid_enum_error;
         return ctx->ConservativeRasterization;

      case GL_TILE_RASTER_ORDER_FIXED_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         return ctx->TileRasterOrderFixed;

      case GL_TILE_RASTER_ORDER_INCREASING_X_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         return ctx->TileRasterOrderIncreasingX;

      case GL_TILE_RASTER_ORDER_INCREASING_Y_MESA:
         if (!_mesa_has_MESA_tile_raster_order(ctx))
            goto invalid_enum_error;
         return ctx->TileRasterOrderIncreasingY;

      case GL_BLACKHOLE_RENDER_INTEL:
         if (!_mesa_has_INTEL_blackhole_render(ctx))
            goto invalid_enum_error;
         return ctx->IntelBlackholeRender;

      default:
         goto invalid_enum_error;
   }

   return GL_FALSE;

invalid_enum_error:
   _mesa_error(ctx, GL_INVALID_ENUM, "glIsEnabled(%s)",
               _mesa_enum_to_string(cap));
   return GL_FALSE;
}
