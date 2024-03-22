/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 * \file barrier.c
 * Implementation of various pipeline barrier entry points.
 *
 * \author Marek Olšák <maraeo@gmail.com>
 */

#include "context.h"
#include "api_exec_decl.h"

#include "pipe/p_context.h"


static void
memory_barrier(struct gl_context *ctx, GLbitfield barriers)
{
   struct pipe_context *pipe = ctx->pipe;
   unsigned flags = 0;

   if (barriers & GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT)
      flags |= PIPE_BARRIER_VERTEX_BUFFER;
   if (barriers & GL_ELEMENT_ARRAY_BARRIER_BIT)
      flags |= PIPE_BARRIER_INDEX_BUFFER;
   if (barriers & GL_UNIFORM_BARRIER_BIT)
      flags |= PIPE_BARRIER_CONSTANT_BUFFER;
   if (barriers & GL_TEXTURE_FETCH_BARRIER_BIT)
      flags |= PIPE_BARRIER_TEXTURE;
   if (barriers & GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)
      flags |= PIPE_BARRIER_IMAGE;
   if (barriers & GL_COMMAND_BARRIER_BIT)
      flags |= PIPE_BARRIER_INDIRECT_BUFFER;
   if (barriers & GL_PIXEL_BUFFER_BARRIER_BIT) {
      /* The PBO may be
       *  (1) bound as a texture for PBO uploads, or
       *  (2) accessed by the CPU via transfer ops.
       * For case (2), we assume automatic flushing by the driver.
       */
      flags |= PIPE_BARRIER_TEXTURE;
   }
   if (barriers & GL_TEXTURE_UPDATE_BARRIER_BIT) {
      /* GL_TEXTURE_UPDATE_BARRIER_BIT:
       * Texture updates translate to:
       *  (1) texture transfers to/from the CPU,
       *  (2) texture as blit destination, or
       *  (3) texture as framebuffer.
       * Some drivers may handle these automatically, and can ignore the bit.
       */
      flags |= PIPE_BARRIER_UPDATE_TEXTURE;
   }
   if (barriers & GL_BUFFER_UPDATE_BARRIER_BIT) {
      /* GL_BUFFER_UPDATE_BARRIER_BIT:
       * Buffer updates translate to
       *  (1) buffer transfers to/from the CPU,
       *  (2) resource copies and clears.
       * Some drivers may handle these automatically, and can ignore the bit.
       */
      flags |= PIPE_BARRIER_UPDATE_BUFFER;
   }
   if (barriers & GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT)
      flags |= PIPE_BARRIER_MAPPED_BUFFER;
   if (barriers & GL_QUERY_BUFFER_BARRIER_BIT)
      flags |= PIPE_BARRIER_QUERY_BUFFER;
   if (barriers & GL_FRAMEBUFFER_BARRIER_BIT)
      flags |= PIPE_BARRIER_FRAMEBUFFER;
   if (barriers & GL_TRANSFORM_FEEDBACK_BARRIER_BIT)
      flags |= PIPE_BARRIER_STREAMOUT_BUFFER;
   if (barriers & GL_ATOMIC_COUNTER_BARRIER_BIT)
      flags |= PIPE_BARRIER_SHADER_BUFFER;
   if (barriers & GL_SHADER_STORAGE_BARRIER_BIT)
      flags |= PIPE_BARRIER_SHADER_BUFFER;

   if (flags && pipe->memory_barrier)
      pipe->memory_barrier(pipe, flags);
}

void GLAPIENTRY
_mesa_TextureBarrierNV(void)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.NV_texture_barrier) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureBarrier(not supported)");
      return;
   }

   ctx->pipe->texture_barrier(ctx->pipe, PIPE_TEXTURE_BARRIER_SAMPLER);
}

void GLAPIENTRY
_mesa_MemoryBarrier(GLbitfield barriers)
{
   GET_CURRENT_CONTEXT(ctx);

   memory_barrier(ctx, barriers);
}

static ALWAYS_INLINE void
memory_barrier_by_region(struct gl_context *ctx, GLbitfield barriers,
                         bool no_error)
{
   GLbitfield all_allowed_bits = GL_ATOMIC_COUNTER_BARRIER_BIT |
                                 GL_FRAMEBUFFER_BARRIER_BIT |
                                 GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                                 GL_SHADER_STORAGE_BARRIER_BIT |
                                 GL_TEXTURE_FETCH_BARRIER_BIT |
                                 GL_UNIFORM_BARRIER_BIT;

   /* From section 7.11.2 of the OpenGL ES 3.1 specification:
    *
    *    "When barriers is ALL_BARRIER_BITS, shader memory accesses will be
    *     synchronized relative to all these barrier bits, but not to other
    *     barrier bits specific to MemoryBarrier."
    *
    * That is, if barriers is the special value GL_ALL_BARRIER_BITS, then all
    * barriers allowed by glMemoryBarrierByRegion should be activated."
    */
   if (barriers == GL_ALL_BARRIER_BITS) {
      memory_barrier(ctx, all_allowed_bits);
      return;
   }

   /* From section 7.11.2 of the OpenGL ES 3.1 specification:
    *
    *    "An INVALID_VALUE error is generated if barriers is not the special
    *     value ALL_BARRIER_BITS, and has any bits set other than those
    *     described above."
    */
   if (!no_error && (barriers & ~all_allowed_bits) != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glMemoryBarrierByRegion(unsupported barrier bit");
   }

   memory_barrier(ctx, barriers);
}

void GLAPIENTRY
_mesa_MemoryBarrierByRegion_no_error(GLbitfield barriers)
{
   GET_CURRENT_CONTEXT(ctx);
   memory_barrier_by_region(ctx, barriers, true);
}

void GLAPIENTRY
_mesa_MemoryBarrierByRegion(GLbitfield barriers)
{
   GET_CURRENT_CONTEXT(ctx);
   memory_barrier_by_region(ctx, barriers, false);
}

void GLAPIENTRY
_mesa_BlendBarrier(void)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.KHR_blend_equation_advanced) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBlendBarrier(not supported)");
      return;
   }

   ctx->pipe->texture_barrier(ctx->pipe, PIPE_TEXTURE_BARRIER_FRAMEBUFFER);
}

void GLAPIENTRY
_mesa_FramebufferFetchBarrierEXT(void)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.EXT_shader_framebuffer_fetch_non_coherent) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glFramebufferFetchBarrierEXT(not supported)");
      return;
   }

   ctx->pipe->texture_barrier(ctx->pipe, PIPE_TEXTURE_BARRIER_FRAMEBUFFER);
}
