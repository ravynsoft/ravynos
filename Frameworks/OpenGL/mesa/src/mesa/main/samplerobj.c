/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2011  VMware, Inc.  All Rights Reserved.
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


/**
 * \file samplerobj.c
 * \brief Functions for the GL_ARB_sampler_objects extension.
 * \author Brian Paul
 */


#include "util/glheader.h"
#include "main/context.h"
#include "main/enums.h"
#include "main/hash.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/samplerobj.h"
#include "main/texturebindless.h"
#include "util/u_memory.h"
#include "api_exec_decl.h"

/* Take advantage of how the enums are defined. */
const enum pipe_tex_wrap wrap_to_gallium_table[32] = {
   [GL_REPEAT & 0x1f] = PIPE_TEX_WRAP_REPEAT,
   [GL_CLAMP & 0x1f] = PIPE_TEX_WRAP_CLAMP,
   [GL_CLAMP_TO_EDGE & 0x1f] = PIPE_TEX_WRAP_CLAMP_TO_EDGE,
   [GL_CLAMP_TO_BORDER & 0x1f] = PIPE_TEX_WRAP_CLAMP_TO_BORDER,
   [GL_MIRRORED_REPEAT & 0x1f] = PIPE_TEX_WRAP_MIRROR_REPEAT,
   [GL_MIRROR_CLAMP_EXT & 0x1f] = PIPE_TEX_WRAP_MIRROR_CLAMP,
   [GL_MIRROR_CLAMP_TO_EDGE & 0x1f] = PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE,
   [GL_MIRROR_CLAMP_TO_BORDER_EXT & 0x1f] = PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER,
};

struct gl_sampler_object *
_mesa_lookup_samplerobj(struct gl_context *ctx, GLuint name)
{
   if (name == 0)
      return NULL;
   else
      return (struct gl_sampler_object *)
         _mesa_HashLookup(ctx->Shared->SamplerObjects, name);
}

static inline struct gl_sampler_object *
lookup_samplerobj_locked(struct gl_context *ctx, GLuint name)
{
   return (struct gl_sampler_object *)
         _mesa_HashLookupLocked(ctx->Shared->SamplerObjects, name);
}

static void
delete_sampler_object(struct gl_context *ctx,
                      struct gl_sampler_object *sampObj)
{
   _mesa_delete_sampler_handles(ctx, sampObj);
   free(sampObj->Label);
   FREE(sampObj);
}

/**
 * Handle reference counting.
 */
void
_mesa_reference_sampler_object_(struct gl_context *ctx,
                                struct gl_sampler_object **ptr,
                                struct gl_sampler_object *samp)
{
   assert(*ptr != samp); /* The inline wrapper should prevent no-op calls */

   if (*ptr) {
      /* Unreference the old sampler */
      struct gl_sampler_object *oldSamp = *ptr;

      assert(oldSamp->RefCount > 0);

      if (p_atomic_dec_zero(&oldSamp->RefCount))
         delete_sampler_object(ctx, oldSamp);
   }

   if (samp) {
      /* reference new sampler */
      assert(samp->RefCount > 0);

      p_atomic_inc(&samp->RefCount);
   }

   *ptr = samp;
}


/**
 * Initialize the fields of the given sampler object.
 */
static void
_mesa_init_sampler_object(struct gl_sampler_object *sampObj, GLuint name)
{
   sampObj->Name = name;
   sampObj->RefCount = 1;
   sampObj->Attrib.WrapS = GL_REPEAT;
   sampObj->Attrib.WrapT = GL_REPEAT;
   sampObj->Attrib.WrapR = GL_REPEAT;
   sampObj->Attrib.state.wrap_s = PIPE_TEX_WRAP_REPEAT;
   sampObj->Attrib.state.wrap_t = PIPE_TEX_WRAP_REPEAT;
   sampObj->Attrib.state.wrap_r = PIPE_TEX_WRAP_REPEAT;
   sampObj->Attrib.MinFilter = GL_NEAREST_MIPMAP_LINEAR;
   sampObj->Attrib.MagFilter = GL_LINEAR;
   sampObj->Attrib.state.min_img_filter = PIPE_TEX_FILTER_NEAREST;
   sampObj->Attrib.state.min_mip_filter = PIPE_TEX_MIPFILTER_LINEAR;
   sampObj->Attrib.state.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampObj->Attrib.state.border_color.f[0] = 0;
   sampObj->Attrib.state.border_color.f[1] = 0;
   sampObj->Attrib.state.border_color.f[2] = 0;
   sampObj->Attrib.state.border_color.f[3] = 0;
   _mesa_update_is_border_color_nonzero(sampObj);
   sampObj->Attrib.MinLod = -1000.0F;
   sampObj->Attrib.MaxLod = 1000.0F;
   sampObj->Attrib.state.min_lod = 0; /* Gallium doesn't allow negative numbers */
   sampObj->Attrib.state.max_lod = 1000;
   sampObj->Attrib.LodBias = 0.0F;
   sampObj->Attrib.state.lod_bias = 0;
   sampObj->Attrib.MaxAnisotropy = 1.0F;
   sampObj->Attrib.state.max_anisotropy = 0; /* Gallium uses 0 instead of 1. */
   sampObj->Attrib.CompareMode = GL_NONE;
   sampObj->Attrib.CompareFunc = GL_LEQUAL;
   sampObj->Attrib.state.compare_mode = PIPE_TEX_COMPARE_NONE;
   sampObj->Attrib.state.compare_func = PIPE_FUNC_LEQUAL;
   sampObj->Attrib.sRGBDecode = GL_DECODE_EXT;
   sampObj->Attrib.CubeMapSeamless = GL_FALSE;
   sampObj->Attrib.state.seamless_cube_map = false;
   sampObj->Attrib.ReductionMode = GL_WEIGHTED_AVERAGE_EXT;
   sampObj->Attrib.state.reduction_mode = PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE;
   sampObj->HandleAllocated = GL_FALSE;

   /* GL_ARB_bindless_texture */
   _mesa_init_sampler_handles(sampObj);
}

static struct gl_sampler_object *
_mesa_new_sampler_object(struct gl_context *ctx, GLuint name)
{
   struct gl_sampler_object *sampObj = CALLOC_STRUCT(gl_sampler_object);
   if (sampObj) {
      _mesa_init_sampler_object(sampObj, name);
   }
   return sampObj;
}

static void
create_samplers(struct gl_context *ctx, GLsizei count, GLuint *samplers,
                const char *caller)
{
   GLint i;

   if (!samplers)
      return;

   _mesa_HashLockMutex(ctx->Shared->SamplerObjects);

   _mesa_HashFindFreeKeys(ctx->Shared->SamplerObjects, samplers, count);

   /* Insert the ID and pointer to new sampler object into hash table */
   for (i = 0; i < count; i++) {
      struct gl_sampler_object *sampObj;

      sampObj = _mesa_new_sampler_object(ctx, samplers[i]);
      if (!sampObj) {
         _mesa_HashUnlockMutex(ctx->Shared->SamplerObjects);
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
         return;
      }

      _mesa_HashInsertLocked(ctx->Shared->SamplerObjects, samplers[i],
                             sampObj, true);
   }

   _mesa_HashUnlockMutex(ctx->Shared->SamplerObjects);
}

static void
create_samplers_err(struct gl_context *ctx, GLsizei count, GLuint *samplers,
                    const char *caller)
{

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%d)\n", caller, count);

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n<0)", caller);
      return;
   }

   create_samplers(ctx, count, samplers, caller);
}

void GLAPIENTRY
_mesa_GenSamplers_no_error(GLsizei count, GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_samplers(ctx, count, samplers, "glGenSamplers");
}

void GLAPIENTRY
_mesa_GenSamplers(GLsizei count, GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_samplers_err(ctx, count, samplers, "glGenSamplers");
}

void GLAPIENTRY
_mesa_CreateSamplers_no_error(GLsizei count, GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_samplers(ctx, count, samplers, "glCreateSamplers");
}

void GLAPIENTRY
_mesa_CreateSamplers(GLsizei count, GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_samplers_err(ctx, count, samplers, "glCreateSamplers");
}


static void
delete_samplers(struct gl_context *ctx, GLsizei count, const GLuint *samplers)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _mesa_HashLockMutex(ctx->Shared->SamplerObjects);

   for (GLsizei i = 0; i < count; i++) {
      if (samplers[i]) {
         GLuint j;
         struct gl_sampler_object *sampObj =
            lookup_samplerobj_locked(ctx, samplers[i]);

         if (sampObj) {
            /* If the sampler is currently bound, unbind it. */
            for (j = 0; j < ctx->Const.MaxCombinedTextureImageUnits; j++) {
               if (ctx->Texture.Unit[j].Sampler == sampObj) {
                  FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
                  _mesa_reference_sampler_object(ctx, &ctx->Texture.Unit[j].Sampler, NULL);
               }
            }

            /* The ID is immediately freed for re-use */
            _mesa_HashRemoveLocked(ctx->Shared->SamplerObjects, samplers[i]);
            /* But the object exists until its reference count goes to zero */
            _mesa_reference_sampler_object(ctx, &sampObj, NULL);
         }
      }
   }

   _mesa_HashUnlockMutex(ctx->Shared->SamplerObjects);
}


void GLAPIENTRY
_mesa_DeleteSamplers_no_error(GLsizei count, const GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_samplers(ctx, count, samplers);
}


void GLAPIENTRY
_mesa_DeleteSamplers(GLsizei count, const GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);

   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteSamplers(count)");
      return;
   }

   delete_samplers(ctx, count, samplers);
}


GLboolean GLAPIENTRY
_mesa_IsSampler(GLuint sampler)
{
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   return _mesa_lookup_samplerobj(ctx, sampler) != NULL;
}

void
_mesa_bind_sampler(struct gl_context *ctx, GLuint unit,
                   struct gl_sampler_object *sampObj)
{
   if (ctx->Texture.Unit[unit].Sampler != sampObj) {
      FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
   }

   _mesa_reference_sampler_object(ctx, &ctx->Texture.Unit[unit].Sampler,
                                  sampObj);
}

static ALWAYS_INLINE void
bind_sampler(struct gl_context *ctx, GLuint unit, GLuint sampler, bool no_error)
{
   struct gl_sampler_object *sampObj;

   if (sampler == 0) {
      /* Use the default sampler object, the one contained in the texture
       * object.
       */
      sampObj = NULL;
   } else {
      /* user-defined sampler object */
      sampObj = _mesa_lookup_samplerobj(ctx, sampler);
      if (!no_error && !sampObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBindSampler(sampler)");
         return;
      }
   }

   /* bind new sampler */
   _mesa_bind_sampler(ctx, unit, sampObj);
}

void GLAPIENTRY
_mesa_BindSampler_no_error(GLuint unit, GLuint sampler)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_sampler(ctx, unit, sampler, true);
}

void GLAPIENTRY
_mesa_BindSampler(GLuint unit, GLuint sampler)
{
   GET_CURRENT_CONTEXT(ctx);

   if (unit >= ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindSampler(unit %u)", unit);
      return;
   }

   bind_sampler(ctx, unit, sampler, false);
}


static ALWAYS_INLINE void
bind_samplers(struct gl_context *ctx, GLuint first, GLsizei count,
              const GLuint *samplers, bool no_error)
{
   GLsizei i;

   FLUSH_VERTICES(ctx, 0, 0);

   if (samplers) {
      /* Note that the error semantics for multi-bind commands differ from
       * those of other GL commands.
       *
       * The Issues section in the ARB_multi_bind spec says:
       *
       *    "(11) Typically, OpenGL specifies that if an error is generated by
       *          a command, that command has no effect.  This is somewhat
       *          unfortunate for multi-bind commands, because it would require
       *          a first pass to scan the entire list of bound objects for
       *          errors and then a second pass to actually perform the
       *          bindings.  Should we have different error semantics?
       *
       *       RESOLVED:  Yes.  In this specification, when the parameters for
       *       one of the <count> binding points are invalid, that binding
       *       point is not updated and an error will be generated.  However,
       *       other binding points in the same command will be updated if
       *       their parameters are valid and no other error occurs."
       */

      _mesa_HashLockMutex(ctx->Shared->SamplerObjects);

      for (i = 0; i < count; i++) {
         const GLuint unit = first + i;
         struct gl_sampler_object * const currentSampler =
             ctx->Texture.Unit[unit].Sampler;
         struct gl_sampler_object *sampObj;

         if (samplers[i] != 0) {
            if (currentSampler && currentSampler->Name == samplers[i])
               sampObj = currentSampler;
            else
               sampObj = lookup_samplerobj_locked(ctx, samplers[i]);

            /* The ARB_multi_bind spec says:
             *
             *    "An INVALID_OPERATION error is generated if any value
             *     in <samplers> is not zero or the name of an existing
             *     sampler object (per binding)."
             */
            if (!no_error && !sampObj) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glBindSamplers(samplers[%d]=%u is not zero or "
                           "the name of an existing sampler object)",
                           i, samplers[i]);
               continue;
            }
         } else {
            sampObj = NULL;
         }

         /* Bind the new sampler */
         if (sampObj != currentSampler) {
            _mesa_reference_sampler_object(ctx,
                                           &ctx->Texture.Unit[unit].Sampler,
                                           sampObj);
            ctx->NewState |= _NEW_TEXTURE_OBJECT;
            ctx->PopAttribState |= GL_TEXTURE_BIT;
         }
      }

      _mesa_HashUnlockMutex(ctx->Shared->SamplerObjects);
   } else {
      /* Unbind all samplers in the range <first> through <first>+<count>-1 */
      for (i = 0; i < count; i++) {
         const GLuint unit = first + i;

         if (ctx->Texture.Unit[unit].Sampler) {
            _mesa_reference_sampler_object(ctx,
                                           &ctx->Texture.Unit[unit].Sampler,
                                           NULL);
            ctx->NewState |= _NEW_TEXTURE_OBJECT;
            ctx->PopAttribState |= GL_TEXTURE_BIT;
         }
      }
   }
}


void GLAPIENTRY
_mesa_BindSamplers_no_error(GLuint first, GLsizei count, const GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_samplers(ctx, first, count, samplers, true);
}


void GLAPIENTRY
_mesa_BindSamplers(GLuint first, GLsizei count, const GLuint *samplers)
{
   GET_CURRENT_CONTEXT(ctx);

   /* The ARB_multi_bind spec says:
    *
    *   "An INVALID_OPERATION error is generated if <first> + <count> is
    *    greater than the number of texture image units supported by
    *    the implementation."
    */
   if (first + count > ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBindSamplers(first=%u + count=%d > the value of "
                  "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS=%u)",
                  first, count, ctx->Const.MaxCombinedTextureImageUnits);
      return;
   }

   bind_samplers(ctx, first, count, samplers, false);
}


/**
 * Check if a coordinate wrap mode is legal.
 * \return GL_TRUE if legal, GL_FALSE otherwise
 */
static GLboolean
validate_texture_wrap_mode(struct gl_context *ctx, GLenum wrap)
{
   const struct gl_extensions * const e = &ctx->Extensions;

   bool mirror_clamp =
      _mesa_has_ATI_texture_mirror_once(ctx) ||
      _mesa_has_EXT_texture_mirror_clamp(ctx);

   bool mirror_clamp_to_edge =
      _mesa_has_ARB_texture_mirror_clamp_to_edge(ctx) ||
      _mesa_has_EXT_texture_mirror_clamp_to_edge(ctx) ||
      mirror_clamp;

   switch (wrap) {
   case GL_CLAMP:
      /* From GL 3.0 specification section E.1 "Profiles and Deprecated
       * Features of OpenGL 3.0":
       *
       * - Texture wrap mode CLAMP - CLAMP is no longer accepted as a value of
       *   texture parameters TEXTURE_WRAP_S, TEXTURE_WRAP_T, or
       *   TEXTURE_WRAP_R.
       */
      return _mesa_is_desktop_gl_compat(ctx);
   case GL_CLAMP_TO_EDGE:
   case GL_REPEAT:
   case GL_MIRRORED_REPEAT:
   case GL_CLAMP_TO_BORDER:
      return GL_TRUE;
   case GL_MIRROR_CLAMP_EXT:
      return mirror_clamp;
   case GL_MIRROR_CLAMP_TO_EDGE_EXT:
      return mirror_clamp_to_edge;
   case GL_MIRROR_CLAMP_TO_BORDER_EXT:
      return e->EXT_texture_mirror_clamp;
   default:
      return GL_FALSE;
   }
}


/**
 * This is called just prior to changing any sampler object state.
 */
static inline void
flush(struct gl_context *ctx)
{
   FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
}

#define INVALID_PARAM 0x100
#define INVALID_PNAME 0x101
#define INVALID_VALUE 0x102

static GLuint
set_sampler_wrap_s(struct gl_context *ctx, struct gl_sampler_object *samp,
                   GLint param)
{
   if (samp->Attrib.WrapS == param)
      return GL_FALSE;
   if (validate_texture_wrap_mode(ctx, param)) {
      flush(ctx);
      update_sampler_gl_clamp(ctx, samp, is_wrap_gl_clamp(samp->Attrib.WrapS), is_wrap_gl_clamp(param), WRAP_S);
      samp->Attrib.WrapS = param;
      samp->Attrib.state.wrap_s = wrap_to_gallium(param);
      _mesa_lower_gl_clamp(ctx, samp);
      return GL_TRUE;
   }
   return INVALID_PARAM;
}


static GLuint
set_sampler_wrap_t(struct gl_context *ctx, struct gl_sampler_object *samp,
                   GLint param)
{
   if (samp->Attrib.WrapT == param)
      return GL_FALSE;
   if (validate_texture_wrap_mode(ctx, param)) {
      flush(ctx);
      update_sampler_gl_clamp(ctx, samp, is_wrap_gl_clamp(samp->Attrib.WrapT), is_wrap_gl_clamp(param), WRAP_T);
      samp->Attrib.WrapT = param;
      samp->Attrib.state.wrap_t = wrap_to_gallium(param);
      _mesa_lower_gl_clamp(ctx, samp);
      return GL_TRUE;
   }
   return INVALID_PARAM;
}


static GLuint
set_sampler_wrap_r(struct gl_context *ctx, struct gl_sampler_object *samp,
                   GLint param)
{
   if (samp->Attrib.WrapR == param)
      return GL_FALSE;
   if (validate_texture_wrap_mode(ctx, param)) {
      flush(ctx);
      update_sampler_gl_clamp(ctx, samp, is_wrap_gl_clamp(samp->Attrib.WrapR), is_wrap_gl_clamp(param), WRAP_R);
      samp->Attrib.WrapR = param;
      samp->Attrib.state.wrap_r = wrap_to_gallium(param);
      _mesa_lower_gl_clamp(ctx, samp);
      return GL_TRUE;
   }
   return INVALID_PARAM;
}

static GLuint
set_sampler_min_filter(struct gl_context *ctx, struct gl_sampler_object *samp,
                       GLint param)
{
   if (samp->Attrib.MinFilter == param)
      return GL_FALSE;

   switch (param) {
   case GL_NEAREST:
   case GL_LINEAR:
   case GL_NEAREST_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_NEAREST_MIPMAP_LINEAR:
   case GL_LINEAR_MIPMAP_LINEAR:
      flush(ctx);
      samp->Attrib.MinFilter = param;
      samp->Attrib.state.min_img_filter = filter_to_gallium(param);
      samp->Attrib.state.min_mip_filter = mipfilter_to_gallium(param);
      _mesa_lower_gl_clamp(ctx, samp);
      return GL_TRUE;
   default:
      return INVALID_PARAM;
   }
}


static GLuint
set_sampler_mag_filter(struct gl_context *ctx, struct gl_sampler_object *samp,
                       GLint param)
{
   if (samp->Attrib.MagFilter == param)
      return GL_FALSE;

   switch (param) {
   case GL_NEAREST:
   case GL_LINEAR:
      flush(ctx);
      samp->Attrib.MagFilter = param;
      samp->Attrib.state.mag_img_filter = filter_to_gallium(param);
      _mesa_lower_gl_clamp(ctx, samp);
      return GL_TRUE;
   default:
      return INVALID_PARAM;
   }
}


static GLuint
set_sampler_lod_bias(struct gl_context *ctx, struct gl_sampler_object *samp,
                     GLfloat param)
{
   if (samp->Attrib.LodBias == param)
      return GL_FALSE;

   flush(ctx);
   samp->Attrib.LodBias = param;
   samp->Attrib.state.lod_bias = util_quantize_lod_bias(param);
   return GL_TRUE;
}


static GLuint
set_sampler_border_colorf(struct gl_context *ctx,
                          struct gl_sampler_object *samp,
                          const GLfloat params[4])
{
   flush(ctx);
   memcpy(samp->Attrib.state.border_color.f, params, 4 * sizeof(float));
   _mesa_update_is_border_color_nonzero(samp);
   return GL_TRUE;
}


static GLuint
set_sampler_border_colori(struct gl_context *ctx,
                          struct gl_sampler_object *samp,
                          const GLint params[4])
{
   flush(ctx);
   memcpy(samp->Attrib.state.border_color.i, params, 4 * sizeof(float));
   _mesa_update_is_border_color_nonzero(samp);
   return GL_TRUE;
}


static GLuint
set_sampler_border_colorui(struct gl_context *ctx,
                           struct gl_sampler_object *samp,
                           const GLuint params[4])
{
   flush(ctx);
   memcpy(samp->Attrib.state.border_color.ui, params, 4 * sizeof(float));
   _mesa_update_is_border_color_nonzero(samp);
   return GL_TRUE;
}


static GLuint
set_sampler_min_lod(struct gl_context *ctx, struct gl_sampler_object *samp,
                    GLfloat param)
{
   if (samp->Attrib.MinLod == param)
      return GL_FALSE;

   flush(ctx);
   samp->Attrib.MinLod = param;
   samp->Attrib.state.min_lod = MAX2(param, 0.0f); /* only positive */

   return GL_TRUE;
}


static GLuint
set_sampler_max_lod(struct gl_context *ctx, struct gl_sampler_object *samp,
                    GLfloat param)
{
   if (samp->Attrib.MaxLod == param)
      return GL_FALSE;

   flush(ctx);
   samp->Attrib.MaxLod = param;
   samp->Attrib.state.max_lod = param;
   return GL_TRUE;
}


static GLuint
set_sampler_compare_mode(struct gl_context *ctx,
                         struct gl_sampler_object *samp, GLint param)
{
    /* If GL_ARB_shadow is not supported, don't report an error.  The
     * sampler object extension spec isn't clear on this extension interaction.
     * Silences errors with Wine on older GPUs such as R200.
     */
   if (!ctx->Extensions.ARB_shadow)
      return GL_FALSE;

   if (samp->Attrib.CompareMode == param)
      return GL_FALSE;

   if (param == GL_NONE ||
       param == GL_COMPARE_R_TO_TEXTURE_ARB) {
      flush(ctx);
      samp->Attrib.CompareMode = param;
      return GL_TRUE;
   }

   return INVALID_PARAM;
}


static GLuint
set_sampler_compare_func(struct gl_context *ctx,
                         struct gl_sampler_object *samp, GLint param)
{
    /* If GL_ARB_shadow is not supported, don't report an error.  The
     * sampler object extension spec isn't clear on this extension interaction.
     * Silences errors with Wine on older GPUs such as R200.
     */
   if (!ctx->Extensions.ARB_shadow)
      return GL_FALSE;

   if (samp->Attrib.CompareFunc == param)
      return GL_FALSE;

   switch (param) {
   case GL_LEQUAL:
   case GL_GEQUAL:
   case GL_EQUAL:
   case GL_NOTEQUAL:
   case GL_LESS:
   case GL_GREATER:
   case GL_ALWAYS:
   case GL_NEVER:
      flush(ctx);
      samp->Attrib.CompareFunc = param;
      samp->Attrib.state.compare_func = func_to_gallium(param);
      return GL_TRUE;
   default:
      return INVALID_PARAM;
   }
}


static GLuint
set_sampler_max_anisotropy(struct gl_context *ctx,
                           struct gl_sampler_object *samp, GLfloat param)
{
   if (!ctx->Extensions.EXT_texture_filter_anisotropic)
      return INVALID_PNAME;

   if (samp->Attrib.MaxAnisotropy == param)
      return GL_FALSE;

   if (param < 1.0F)
      return INVALID_VALUE;

   flush(ctx);
   /* clamp to max, that's what NVIDIA does */
   samp->Attrib.MaxAnisotropy = MIN2(param, ctx->Const.MaxTextureMaxAnisotropy);
   /* gallium sets 0 for 1 */
   samp->Attrib.state.max_anisotropy = samp->Attrib.MaxAnisotropy == 1 ?
                                          0 : samp->Attrib.MaxAnisotropy;
   return GL_TRUE;
}


static GLuint
set_sampler_cube_map_seamless(struct gl_context *ctx,
                              struct gl_sampler_object *samp, GLboolean param)
{
   if (!_mesa_is_desktop_gl(ctx)
       || !ctx->Extensions.AMD_seamless_cubemap_per_texture)
      return INVALID_PNAME;

   if (samp->Attrib.CubeMapSeamless == param)
      return GL_FALSE;

   if (param != GL_TRUE && param != GL_FALSE)
      return INVALID_VALUE;

   flush(ctx);
   samp->Attrib.CubeMapSeamless = param;
   samp->Attrib.state.seamless_cube_map = param;
   return GL_TRUE;
}

static GLuint
set_sampler_srgb_decode(struct gl_context *ctx,
                              struct gl_sampler_object *samp, GLenum param)
{
   if (!ctx->Extensions.EXT_texture_sRGB_decode)
      return INVALID_PNAME;

   if (samp->Attrib.sRGBDecode == param)
      return GL_FALSE;

   /* The EXT_texture_sRGB_decode spec says:
    *
    *    "INVALID_ENUM is generated if the <pname> parameter of
    *     TexParameter[i,f,Ii,Iui][v][EXT],
    *     MultiTexParameter[i,f,Ii,Iui][v]EXT,
    *     TextureParameter[i,f,Ii,Iui][v]EXT, SamplerParameter[i,f,Ii,Iui][v]
    *     is TEXTURE_SRGB_DECODE_EXT when the <param> parameter is not one of
    *     DECODE_EXT or SKIP_DECODE_EXT.
    *
    * Returning INVALID_PARAM makes that happen.
    */
   if (param != GL_DECODE_EXT && param != GL_SKIP_DECODE_EXT)
      return INVALID_PARAM;

   flush(ctx);
   samp->Attrib.sRGBDecode = param;
   return GL_TRUE;
}

static GLuint
set_sampler_reduction_mode(struct gl_context *ctx,
                           struct gl_sampler_object *samp, GLenum param)
{
   if (!ctx->Extensions.EXT_texture_filter_minmax &&
       !_mesa_has_ARB_texture_filter_minmax(ctx))
      return INVALID_PNAME;

   if (samp->Attrib.ReductionMode == param)
      return GL_FALSE;

   if (param != GL_WEIGHTED_AVERAGE_EXT && param != GL_MIN && param != GL_MAX)
      return INVALID_PARAM;

   flush(ctx);
   samp->Attrib.ReductionMode = param;
   samp->Attrib.state.reduction_mode = reduction_to_gallium(param);
   return GL_TRUE;
}

static struct gl_sampler_object *
sampler_parameter_error_check(struct gl_context *ctx, GLuint sampler,
                              bool get, const char *name)
{
   struct gl_sampler_object *sampObj;

   sampObj = _mesa_lookup_samplerobj(ctx, sampler);
   if (!sampObj) {
      /* OpenGL 4.5 spec, section "8.2 Sampler Objects", page 176 of the PDF
       * states:
       *
       *    "An INVALID_OPERATION error is generated if sampler is not the name
       *    of a sampler object previously returned from a call to
       *    GenSamplers."
       */
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid sampler)", name);
      return NULL;
   }

   if (!get && sampObj->HandleAllocated) {
      /* The ARB_bindless_texture spec says:
       *
       * "The error INVALID_OPERATION is generated by SamplerParameter* if
       *  <sampler> identifies a sampler object referenced by one or more
       *  texture handles."
       */
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable sampler)", name);
      return NULL;
   }

   return sampObj;
}

void GLAPIENTRY
_mesa_SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameteri");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, param);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, param);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, param);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, param);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, param);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, (GLfloat) param);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, (GLfloat) param);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, (GLfloat) param);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, param);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, param);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, (GLfloat) param);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, param);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, param);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, param);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      FALLTHROUGH;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameteri(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameteri(param=%d)\n",
                  param);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameteri(param=%d)\n",
                  param);
      break;
   default:
      ;
   }
}


void GLAPIENTRY
_mesa_SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameterf");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, param);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, param);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, param);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, (GLint) param);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, param);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, (GLboolean) param);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, (GLenum) param);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, (GLenum) param);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      FALLTHROUGH;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterf(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterf(param=%f)\n",
                  param);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameterf(param=%f)\n",
                  param);
      break;
   default:
      ;
   }
}

void GLAPIENTRY
_mesa_SamplerParameteriv(GLuint sampler, GLenum pname, const GLint *params)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameteriv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      {
         GLfloat c[4];
         c[0] = INT_TO_FLOAT(params[0]);
         c[1] = INT_TO_FLOAT(params[1]);
         c[2] = INT_TO_FLOAT(params[2]);
         c[3] = INT_TO_FLOAT(params[3]);
         res = set_sampler_border_colorf(ctx, sampObj, c);
      }
      break;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameteriv(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameteriv(param=%d)\n",
                  params[0]);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameteriv(param=%d)\n",
                  params[0]);
      break;
   default:
      ;
   }
}

void GLAPIENTRY
_mesa_SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *params)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameterfv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, (GLint) params[0]);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, (GLboolean) params[0]);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      res = set_sampler_border_colorf(ctx, sampObj, params);
      break;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterfv(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterfv(param=%f)\n",
                  params[0]);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameterfv(param=%f)\n",
                  params[0]);
      break;
   default:
      ;
   }
}

void GLAPIENTRY
_mesa_SamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *params)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameterIiv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      res = set_sampler_border_colori(ctx, sampObj, params);
      break;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterIiv(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterIiv(param=%d)\n",
                  params[0]);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameterIiv(param=%d)\n",
                  params[0]);
      break;
   default:
      ;
   }
}


void GLAPIENTRY
_mesa_SamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *params)
{
   struct gl_sampler_object *sampObj;
   GLuint res;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, false,
                                           "glSamplerParameterIuiv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      res = set_sampler_wrap_s(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_T:
      res = set_sampler_wrap_t(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_WRAP_R:
      res = set_sampler_wrap_r(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_FILTER:
      res = set_sampler_min_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAG_FILTER:
      res = set_sampler_mag_filter(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MIN_LOD:
      res = set_sampler_min_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_MAX_LOD:
      res = set_sampler_max_lod(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_LOD_BIAS:
      res = set_sampler_lod_bias(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      res = set_sampler_compare_mode(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      res = set_sampler_compare_func(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      res = set_sampler_max_anisotropy(ctx, sampObj, (GLfloat) params[0]);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      res = set_sampler_cube_map_seamless(ctx, sampObj, params[0]);
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      res = set_sampler_srgb_decode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      res = set_sampler_reduction_mode(ctx, sampObj, (GLenum) params[0]);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      res = set_sampler_border_colorui(ctx, sampObj, params);
      break;
   default:
      res = INVALID_PNAME;
   }

   switch (res) {
   case GL_FALSE:
      /* no change */
      break;
   case GL_TRUE:
      /* state change - we do nothing special at this time */
      break;
   case INVALID_PNAME:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterIuiv(pname=%s)\n",
                  _mesa_enum_to_string(pname));
      break;
   case INVALID_PARAM:
      _mesa_error(ctx, GL_INVALID_ENUM, "glSamplerParameterIuiv(param=%u)\n",
                  params[0]);
      break;
   case INVALID_VALUE:
      _mesa_error(ctx, GL_INVALID_VALUE, "glSamplerParameterIuiv(param=%u)\n",
                  params[0]);
      break;
   default:
      ;
   }
}


void GLAPIENTRY
_mesa_GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
   struct gl_sampler_object *sampObj;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, true,
                                           "glGetSamplerParameteriv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      *params = sampObj->Attrib.WrapS;
      break;
   case GL_TEXTURE_WRAP_T:
      *params = sampObj->Attrib.WrapT;
      break;
   case GL_TEXTURE_WRAP_R:
      *params = sampObj->Attrib.WrapR;
      break;
   case GL_TEXTURE_MIN_FILTER:
      *params = sampObj->Attrib.MinFilter;
      break;
   case GL_TEXTURE_MAG_FILTER:
      *params = sampObj->Attrib.MagFilter;
      break;
   case GL_TEXTURE_MIN_LOD:
      /* GL spec 'Data Conversions' section specifies that floating-point
       * value in integer Get function is rounded to nearest integer
       */
      *params = lroundf(sampObj->Attrib.MinLod);
      break;
   case GL_TEXTURE_MAX_LOD:
      /* GL spec 'Data Conversions' section specifies that floating-point
       * value in integer Get function is rounded to nearest integer
       */
      *params = lroundf(sampObj->Attrib.MaxLod);
      break;
   case GL_TEXTURE_LOD_BIAS:
      /* GL spec 'Data Conversions' section specifies that floating-point
       * value in integer Get function is rounded to nearest integer
       */
      *params = lroundf(sampObj->Attrib.LodBias);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      if (!ctx->Extensions.ARB_shadow)
         goto invalid_pname;
      *params = sampObj->Attrib.CompareMode;
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      if (!ctx->Extensions.ARB_shadow)
         goto invalid_pname;
      *params = sampObj->Attrib.CompareFunc;
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (!ctx->Extensions.EXT_texture_filter_anisotropic)
         goto invalid_pname;
      /* GL spec 'Data Conversions' section specifies that floating-point
       * value in integer Get function is rounded to nearest integer
       */
      *params = lroundf(sampObj->Attrib.MaxAnisotropy);
      break;
   case GL_TEXTURE_BORDER_COLOR:
      params[0] = FLOAT_TO_INT(sampObj->Attrib.state.border_color.f[0]);
      params[1] = FLOAT_TO_INT(sampObj->Attrib.state.border_color.f[1]);
      params[2] = FLOAT_TO_INT(sampObj->Attrib.state.border_color.f[2]);
      params[3] = FLOAT_TO_INT(sampObj->Attrib.state.border_color.f[3]);
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (!ctx->Extensions.AMD_seamless_cubemap_per_texture)
         goto invalid_pname;
      *params = sampObj->Attrib.CubeMapSeamless;
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (!ctx->Extensions.EXT_texture_sRGB_decode)
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.sRGBDecode;
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      if (!ctx->Extensions.EXT_texture_filter_minmax &&
          !_mesa_has_ARB_texture_filter_minmax(ctx))
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.ReductionMode;
      break;
   default:
      goto invalid_pname;
   }
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetSamplerParameteriv(pname=%s)",
               _mesa_enum_to_string(pname));
}


void GLAPIENTRY
_mesa_GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
   struct gl_sampler_object *sampObj;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, true,
                                           "glGetSamplerParameterfv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      *params = (GLfloat) sampObj->Attrib.WrapS;
      break;
   case GL_TEXTURE_WRAP_T:
      *params = (GLfloat) sampObj->Attrib.WrapT;
      break;
   case GL_TEXTURE_WRAP_R:
      *params = (GLfloat) sampObj->Attrib.WrapR;
      break;
   case GL_TEXTURE_MIN_FILTER:
      *params = (GLfloat) sampObj->Attrib.MinFilter;
      break;
   case GL_TEXTURE_MAG_FILTER:
      *params = (GLfloat) sampObj->Attrib.MagFilter;
      break;
   case GL_TEXTURE_MIN_LOD:
      *params = sampObj->Attrib.MinLod;
      break;
   case GL_TEXTURE_MAX_LOD:
      *params = sampObj->Attrib.MaxLod;
      break;
   case GL_TEXTURE_LOD_BIAS:
      *params = sampObj->Attrib.LodBias;
      break;
   case GL_TEXTURE_COMPARE_MODE:
      *params = (GLfloat) sampObj->Attrib.CompareMode;
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      *params = (GLfloat) sampObj->Attrib.CompareFunc;
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      *params = sampObj->Attrib.MaxAnisotropy;
      break;
   case GL_TEXTURE_BORDER_COLOR:
      params[0] = sampObj->Attrib.state.border_color.f[0];
      params[1] = sampObj->Attrib.state.border_color.f[1];
      params[2] = sampObj->Attrib.state.border_color.f[2];
      params[3] = sampObj->Attrib.state.border_color.f[3];
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (!ctx->Extensions.AMD_seamless_cubemap_per_texture)
         goto invalid_pname;
      *params = (GLfloat) sampObj->Attrib.CubeMapSeamless;
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (!ctx->Extensions.EXT_texture_sRGB_decode)
         goto invalid_pname;
      *params = (GLfloat) sampObj->Attrib.sRGBDecode;
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      if (!ctx->Extensions.EXT_texture_filter_minmax &&
          !_mesa_has_ARB_texture_filter_minmax(ctx))
         goto invalid_pname;
      *params = (GLfloat) sampObj->Attrib.ReductionMode;
      break;
   default:
      goto invalid_pname;
   }
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetSamplerParameterfv(pname=%s)",
               _mesa_enum_to_string(pname));
}


void GLAPIENTRY
_mesa_GetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params)
{
   struct gl_sampler_object *sampObj;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, true,
                                           "glGetSamplerParameterIiv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      *params = sampObj->Attrib.WrapS;
      break;
   case GL_TEXTURE_WRAP_T:
      *params = sampObj->Attrib.WrapT;
      break;
   case GL_TEXTURE_WRAP_R:
      *params = sampObj->Attrib.WrapR;
      break;
   case GL_TEXTURE_MIN_FILTER:
      *params = sampObj->Attrib.MinFilter;
      break;
   case GL_TEXTURE_MAG_FILTER:
      *params = sampObj->Attrib.MagFilter;
      break;
   case GL_TEXTURE_MIN_LOD:
      *params = (GLint) sampObj->Attrib.MinLod;
      break;
   case GL_TEXTURE_MAX_LOD:
      *params = (GLint) sampObj->Attrib.MaxLod;
      break;
   case GL_TEXTURE_LOD_BIAS:
      *params = (GLint) sampObj->Attrib.LodBias;
      break;
   case GL_TEXTURE_COMPARE_MODE:
      *params = sampObj->Attrib.CompareMode;
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      *params = sampObj->Attrib.CompareFunc;
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      *params = (GLint) sampObj->Attrib.MaxAnisotropy;
      break;
   case GL_TEXTURE_BORDER_COLOR:
      params[0] = sampObj->Attrib.state.border_color.i[0];
      params[1] = sampObj->Attrib.state.border_color.i[1];
      params[2] = sampObj->Attrib.state.border_color.i[2];
      params[3] = sampObj->Attrib.state.border_color.i[3];
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (!ctx->Extensions.AMD_seamless_cubemap_per_texture)
         goto invalid_pname;
      *params = sampObj->Attrib.CubeMapSeamless;
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (!ctx->Extensions.EXT_texture_sRGB_decode)
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.sRGBDecode;
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      if (!ctx->Extensions.EXT_texture_filter_minmax &&
          !_mesa_has_ARB_texture_filter_minmax(ctx))
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.ReductionMode;
      break;
   default:
      goto invalid_pname;
   }
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetSamplerParameterIiv(pname=%s)",
               _mesa_enum_to_string(pname));
}


void GLAPIENTRY
_mesa_GetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params)
{
   struct gl_sampler_object *sampObj;
   GET_CURRENT_CONTEXT(ctx);

   sampObj = sampler_parameter_error_check(ctx, sampler, true,
                                           "glGetSamplerParameterIuiv");
   if (!sampObj)
      return;

   switch (pname) {
   case GL_TEXTURE_WRAP_S:
      *params = sampObj->Attrib.WrapS;
      break;
   case GL_TEXTURE_WRAP_T:
      *params = sampObj->Attrib.WrapT;
      break;
   case GL_TEXTURE_WRAP_R:
      *params = sampObj->Attrib.WrapR;
      break;
   case GL_TEXTURE_MIN_FILTER:
      *params = sampObj->Attrib.MinFilter;
      break;
   case GL_TEXTURE_MAG_FILTER:
      *params = sampObj->Attrib.MagFilter;
      break;
   case GL_TEXTURE_MIN_LOD:
      *params = (GLuint) sampObj->Attrib.MinLod;
      break;
   case GL_TEXTURE_MAX_LOD:
      *params = (GLuint) sampObj->Attrib.MaxLod;
      break;
   case GL_TEXTURE_LOD_BIAS:
      *params = (GLuint) sampObj->Attrib.LodBias;
      break;
   case GL_TEXTURE_COMPARE_MODE:
      *params = sampObj->Attrib.CompareMode;
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      *params = sampObj->Attrib.CompareFunc;
      break;
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      *params = (GLuint) sampObj->Attrib.MaxAnisotropy;
      break;
   case GL_TEXTURE_BORDER_COLOR:
      params[0] = sampObj->Attrib.state.border_color.ui[0];
      params[1] = sampObj->Attrib.state.border_color.ui[1];
      params[2] = sampObj->Attrib.state.border_color.ui[2];
      params[3] = sampObj->Attrib.state.border_color.ui[3];
      break;
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (!ctx->Extensions.AMD_seamless_cubemap_per_texture)
         goto invalid_pname;
      *params = sampObj->Attrib.CubeMapSeamless;
      break;
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (!ctx->Extensions.EXT_texture_sRGB_decode)
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.sRGBDecode;
      break;
   case GL_TEXTURE_REDUCTION_MODE_EXT:
      if (!ctx->Extensions.EXT_texture_filter_minmax &&
          !_mesa_has_ARB_texture_filter_minmax(ctx))
         goto invalid_pname;
      *params = (GLenum) sampObj->Attrib.ReductionMode;
      break;
   default:
      goto invalid_pname;
   }
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetSamplerParameterIuiv(pname=%s)",
               _mesa_enum_to_string(pname));
}
