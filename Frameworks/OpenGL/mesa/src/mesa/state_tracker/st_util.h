/**************************************************************************
 *
 * Copyright 2019 VMware, Inc.
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


/*
 * Miscellantous state tracker utility functions, macros.
 */


#ifndef ST_UTIL
#define ST_UTIL


#include "state_tracker/st_context.h"
#include "main/context.h"


#ifdef __cplusplus
extern "C" {
#endif


/** For drawing quads for glClear, glDraw/CopyPixels, glBitmap, etc. */
struct st_util_vertex
{
   float x, y, z;
   float r, g, b, a;
   float s, t;
};



/* Invalidate the readpixels cache to ensure we don't read stale data.
 */
static inline void
st_invalidate_readpix_cache(struct st_context *st)
{
   if (unlikely(st->readpix_cache.src)) {
      pipe_resource_reference(&st->readpix_cache.src, NULL);
      pipe_resource_reference(&st->readpix_cache.cache, NULL);
   }
}

static inline bool
st_user_clip_planes_enabled(struct gl_context *ctx)
{
   return (_mesa_is_desktop_gl_compat(ctx) ||
           _mesa_is_gles1(ctx)) && /* only ES 1.x */
          ctx->Transform.ClipPlanesEnabled;
}

static inline bool
st_point_size_per_vertex(struct gl_context *ctx)
{
   const struct gl_program *vertProg = ctx->VertexProgram._Current;
   if (vertProg) {
      if (vertProg->Id == 0) {
         if (vertProg->info.outputs_written &
             BITFIELD64_BIT(VARYING_SLOT_PSIZ)) {
            /* generated program which emits point size */
            return true;
         }
      }
      else if (ctx->API != API_OPENGLES2) {
         /* PointSizeEnabled is always set in ES2 contexts */
         return ctx->VertexProgram.PointSizeEnabled;
      }
      else {
         /* ST_NEW_TESSEVAL_PROGRAM | ST_NEW_GEOMETRY_PROGRAM */
         /* We have to check the last bound stage and see if it writes psize */
         struct gl_program *last = NULL;
         if (ctx->GeometryProgram._Current)
            last = ctx->GeometryProgram._Current;
         else if (ctx->TessEvalProgram._Current)
            last = ctx->TessEvalProgram._Current;
         else if (ctx->VertexProgram._Current)
            last = ctx->VertexProgram._Current;
         if (last)
            return !!(last->info.outputs_written &
                      BITFIELD64_BIT(VARYING_SLOT_PSIZ));
      }
   }
   return false;
}

static inline void
st_validate_state(struct st_context *st, uint64_t pipeline_state_mask)
{
   struct gl_context *ctx = st->ctx;

   /* Inactive states are shader states not used by shaders at the moment. */
   uint64_t dirty = ctx->NewDriverState & st->active_states & pipeline_state_mask;

   if (dirty) {
      ctx->NewDriverState &= ~dirty;

      /* Execute functions that set states that have been changed since
       * the last draw.
       *
       * x86_64: u_bit_scan64 is negligibly faster than u_bit_scan
       * i386:   u_bit_scan64 is noticably slower than u_bit_scan
       */
      if (sizeof(void*) == 8) {
         while (dirty)
            st_update_functions[u_bit_scan64(&dirty)](st);
      } else {
         /* Split u_bit_scan64 into 2x u_bit_scan32 for i386. */
         uint32_t dirty_lo = dirty;
         uint32_t dirty_hi = dirty >> 32;

         while (dirty_lo)
            st_update_functions[u_bit_scan(&dirty_lo)](st);
         while (dirty_hi)
            st_update_functions[32 + u_bit_scan(&dirty_hi)](st);
      }
   }
}

#ifdef __cplusplus
}
#endif


#endif /* ST_UTIL */
