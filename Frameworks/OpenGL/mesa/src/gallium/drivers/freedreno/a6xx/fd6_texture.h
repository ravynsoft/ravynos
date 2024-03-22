/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD6_TEXTURE_H_
#define FD6_TEXTURE_H_

#include "pipe/p_context.h"

#include "freedreno_resource.h"
#include "freedreno_texture.h"

#include "fd6_context.h"
#include "fdl/fd6_format_table.h"


BEGINC;

/* Border color layout is diff from a4xx/a5xx.. if it turns out to be
 * the same as a6xx then move this somewhere common ;-)
 *
 * Entry layout looks like (total size, 0x80 bytes):
 */

struct PACKED fd6_bcolor_entry {
   uint32_t fp32[4];
   uint16_t ui16[4];
   int16_t si16[4];
   uint16_t fp16[4];
   uint16_t rgb565;
   uint16_t rgb5a1;
   uint16_t rgba4;
   uint8_t __pad0[2];
   uint8_t ui8[4];
   int8_t si8[4];
   uint32_t rgb10a2;
   uint32_t z24;
   uint16_t srgb[4]; /* appears to duplicate fp16[], but clamped, used for srgb */
   uint8_t __pad1[56];
};

#define FD6_BORDER_COLOR_SIZE sizeof(struct fd6_bcolor_entry)
#define FD6_MAX_BORDER_COLORS 256

struct fd6_sampler_stateobj {
   struct pipe_sampler_state base;
   uint32_t texsamp0, texsamp1, texsamp2, texsamp3;
   uint16_t seqno;
};

static inline struct fd6_sampler_stateobj *
fd6_sampler_stateobj(struct pipe_sampler_state *samp)
{
   return (struct fd6_sampler_stateobj *)samp;
}

struct fd6_pipe_sampler_view {
   struct pipe_sampler_view base;
   struct fd_resource *ptr1, *ptr2;
   uint16_t seqno;

   /* TEX_CONST descriptor, with just offsets from the BOs in the iova dwords. */
   uint32_t descriptor[FDL6_TEX_CONST_DWORDS];

   /* For detecting when a resource has transitioned from UBWC compressed
    * to uncompressed, which means the sampler state needs to be updated
    */
   uint16_t rsc_seqno;
};

static inline struct fd6_pipe_sampler_view *
fd6_pipe_sampler_view(struct pipe_sampler_view *pview)
{
   return (struct fd6_pipe_sampler_view *)pview;
}

void fd6_texture_init(struct pipe_context *pctx);
void fd6_texture_fini(struct pipe_context *pctx);

/*
 * Texture stateobj:
 *
 * The sampler and sampler-view state is mapped to a single hardware
 * stateobj which can be emit'd as a pointer in a CP_SET_DRAW_STATE
 * packet, to avoid the overhead of re-generating the entire cmdstream
 * when application toggles thru multiple different texture states.
 */

struct fd6_texture_key {
   uint16_t view_seqno[16];
   uint16_t samp_seqno[16];
   uint8_t type;
};

struct fd6_texture_state {
   struct fd6_texture_key key;
   struct fd_ringbuffer *stateobj;
   /**
    * Track the rsc seqno's associated with the texture views so
    * we know what to invalidate when a rsc is rebound when the
    * underlying bo changes.  (For example, demotion from UBWC.)
    */
   uint16_t view_rsc_seqno[16];
   bool invalidate;
};

struct fd6_texture_state *
fd6_texture_state(struct fd_context *ctx, enum pipe_shader_type type) assert_dt;

ENDC;

#endif /* FD6_TEXTURE_H_ */
