/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
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

#define FD_BO_NO_HARDPIN 1

#include "pipe/p_state.h"

#include "freedreno_resource.h"
#include "freedreno_state.h"

#include "fd6_image.h"
#include "fd6_pack.h"
#include "fd6_resource.h"
#include "fd6_screen.h"
#include "fd6_texture.h"

static const uint8_t swiz_identity[4] = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
                                         PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W};

static uint64_t
rsc_iova(struct pipe_resource *prsc, unsigned offset)
{
   if (!prsc)
      return 0;
   return fd_bo_get_iova(fd_resource(prsc)->bo) + offset;
}

static void
fd6_ssbo_descriptor(struct fd_context *ctx,
                    const struct pipe_shader_buffer *buf, uint32_t *descriptor)
{
   fdl6_buffer_view_init(
      descriptor,
      PIPE_FORMAT_R32_UINT,
      swiz_identity, rsc_iova(buf->buffer, buf->buffer_offset),
      buf->buffer_size);
}

static void
fd6_image_descriptor(struct fd_context *ctx, const struct pipe_image_view *buf,
                     uint32_t *descriptor)
{
   if (buf->resource->target == PIPE_BUFFER) {
      uint32_t size = fd_clamp_buffer_size(buf->format, buf->u.buf.size,
                                           A4XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT);

      fdl6_buffer_view_init(descriptor, buf->format, swiz_identity,
                            rsc_iova(buf->resource, buf->u.buf.offset),
                            size);
   } else {
      struct fdl_view_args args = {
         .chip = A6XX,

         .iova = rsc_iova(buf->resource, 0),

         .base_miplevel = buf->u.tex.level,
         .level_count = 1,

         .base_array_layer = buf->u.tex.first_layer,
         .layer_count = buf->u.tex.last_layer - buf->u.tex.first_layer + 1,

         .swiz = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
         .format = buf->format,

         .type = fdl_type_from_pipe_target(buf->resource->target),
         .chroma_offsets = {FDL_CHROMA_LOCATION_COSITED_EVEN,
                            FDL_CHROMA_LOCATION_COSITED_EVEN},
      };

      /* fdl6_view makes the storage descriptor treat cubes like a 2D array (so
       * you can reference a specific layer), but we need to do that for the
       * texture descriptor as well to get our layer.
       */
      if (args.type == FDL_VIEW_TYPE_CUBE)
         args.type = FDL_VIEW_TYPE_2D;

      struct fdl6_view view;
      struct fd_resource *rsc = fd_resource(buf->resource);
      const struct fdl_layout *layouts[3] = { &rsc->layout, NULL, NULL };
      fdl6_view_init(&view, layouts, &args,
                     ctx->screen->info->a6xx.has_z24uint_s8uint);

      memcpy(descriptor, view.storage_descriptor, sizeof(view.storage_descriptor));
   }
}

static struct fd6_descriptor_set *
descriptor_set(struct fd_context *ctx, enum pipe_shader_type shader)
   assert_dt
{
   struct fd6_context *fd6_ctx = fd6_context(ctx);

   if (shader == PIPE_SHADER_COMPUTE)
      return &fd6_ctx->cs_descriptor_set;

   unsigned idx = ir3_shader_descriptor_set(shader);
   assert(idx < ARRAY_SIZE(fd6_ctx->descriptor_sets));
   return &fd6_ctx->descriptor_sets[idx];
}

static void
clear_descriptor(struct fd6_descriptor_set *set, unsigned slot)
{
   /* The 2nd dword of the descriptor contains the width and height.
    * so a non-zero value means the slot was previously valid and
    * must be cleared.  We can't leave dangling descriptors as the
    * shader could use variable indexing into the set of IBOs to
    * get at them.  See piglit arb_shader_image_load_store-invalid.
    */
   if (!set->descriptor[slot][1])
      return;

   fd6_descriptor_set_invalidate(set);

   memset(set->descriptor[slot], 0, sizeof(set->descriptor[slot]));
}

static void
validate_image_descriptor(struct fd_context *ctx, struct fd6_descriptor_set *set,
                          unsigned slot, struct pipe_image_view *img)
{
   struct fd_resource *rsc = fd_resource(img->resource);

   if (!rsc || (rsc->seqno == set->seqno[slot]))
      return;

   fd6_descriptor_set_invalidate(set);

   fd6_image_descriptor(ctx, img, set->descriptor[slot]);
   set->seqno[slot] = rsc->seqno;
}

static void
validate_buffer_descriptor(struct fd_context *ctx, struct fd6_descriptor_set *set,
                           unsigned slot, struct pipe_shader_buffer *buf)
{
   struct fd_resource *rsc = fd_resource(buf->buffer);

   if (!rsc || (rsc->seqno == set->seqno[slot]))
      return;

   fd6_descriptor_set_invalidate(set);

   fd6_ssbo_descriptor(ctx, buf, set->descriptor[slot]);
   set->seqno[slot] = rsc->seqno;
}

/* Build bindless descriptor state, returns ownership of state reference */
template <chip CHIP>
struct fd_ringbuffer *
fd6_build_bindless_state(struct fd_context *ctx, enum pipe_shader_type shader,
                         bool append_fb_read)
{
   struct fd_shaderbuf_stateobj *bufso = &ctx->shaderbuf[shader];
   struct fd_shaderimg_stateobj *imgso = &ctx->shaderimg[shader];
   struct fd6_descriptor_set *set = descriptor_set(ctx, shader);

   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      ctx->batch->submit, 16 * 4, FD_RINGBUFFER_STREAMING);

   /* Don't re-use a previous descriptor set if appending the
    * fb-read descriptor, as that can change across batches.
    * The normal descriptor slots are safe to re-use even if
    * the state is dirtied due to batch flush, but the fb-read
    * slot is not.
    */
   if (unlikely(append_fb_read))
      fd6_descriptor_set_invalidate(set);

   /*
    * Re-validate the descriptor slots, ie. in the case that
    * the resource gets rebound due to use with non-UBWC
    * compatible view format, etc.
    *
    * While we are at it, attach the BOs to the ring.
    */

   u_foreach_bit (b, bufso->enabled_mask) {
      struct pipe_shader_buffer *buf = &bufso->sb[b];
      unsigned idx = b + IR3_BINDLESS_SSBO_OFFSET;
      validate_buffer_descriptor(ctx, set, idx, buf);
   }

   u_foreach_bit (b, imgso->enabled_mask) {
      struct pipe_image_view *img = &imgso->si[b];
      unsigned idx = b + IR3_BINDLESS_IMAGE_OFFSET;
      validate_image_descriptor(ctx, set, idx, img);
   }

   if (!set->bo) {
      set->bo = fd_bo_new(
            ctx->dev, sizeof(set->descriptor),
            /* Use same flags as ringbuffer so hits the same heap,
             * because those will already have the FD_RELOC_DUMP
             * flag set:
             */
            FD_BO_GPUREADONLY | FD_BO_CACHED_COHERENT,
            "%s bindless", _mesa_shader_stage_to_abbrev(shader));
      fd_bo_mark_for_dump(set->bo);

      uint32_t *desc_buf = (uint32_t *)fd_bo_map(set->bo);

      memcpy(desc_buf, set->descriptor, sizeof(set->descriptor));

      if (unlikely(append_fb_read)) {
         /* Reserve A6XX_MAX_RENDER_TARGETS image slots for fb-read */
         unsigned idx = IR3_BINDLESS_DESC_COUNT - 1 - A6XX_MAX_RENDER_TARGETS;

         for (int i = 0; i < ctx->batch->framebuffer.nr_cbufs; i++) {
            /* This is patched with the appropriate descriptor for GMEM or
             * sysmem rendering path in fd6_gmem
             */
            struct fd_cs_patch patch = {
               .cs = &desc_buf[(idx + i) * FDL6_TEX_CONST_DWORDS],
               .val = i,
            };
            util_dynarray_append(&ctx->batch->fb_read_patches,
                                 __typeof__(patch), patch);
         }
      }
   }

   /*
    * Build stateobj emitting reg writes to configure the descriptor
    * set and CP_LOAD_STATE packets to preload the state.
    *
    * Note that unless the app is using the max # of SSBOs there will
    * be a gap between the IBO descriptors used for SSBOs and for images,
    * so emit this as two CP_LOAD_STATE packets:
    */

   unsigned idx = ir3_shader_descriptor_set(shader);

   fd_ringbuffer_attach_bo(ring, set->bo);

   if (shader == PIPE_SHADER_COMPUTE) {
      OUT_REG(ring, HLSQ_INVALIDATE_CMD(CHIP, .cs_bindless = 0x1f));
      OUT_REG(ring, SP_CS_BINDLESS_BASE_DESCRIPTOR(CHIP,
            idx, .desc_size = BINDLESS_DESCRIPTOR_64B, .bo = set->bo,
      ));
      OUT_REG(ring, A6XX_HLSQ_CS_BINDLESS_BASE_DESCRIPTOR(
            idx, .desc_size = BINDLESS_DESCRIPTOR_64B, .bo = set->bo,
      ));

      if (bufso->enabled_mask) {
         OUT_PKT(ring, CP_LOAD_STATE6_FRAG,
            CP_LOAD_STATE6_0(
                  .dst_off     = IR3_BINDLESS_SSBO_OFFSET,
                  .state_type  = ST6_IBO,
                  .state_src   = SS6_BINDLESS,
                  .state_block = SB6_CS_SHADER,
                  .num_unit    = util_last_bit(bufso->enabled_mask),
            ),
            CP_LOAD_STATE6_EXT_SRC_ADDR(
                  /* This isn't actually an address: */
                  .qword = (idx << 28) |
                     IR3_BINDLESS_SSBO_OFFSET * FDL6_TEX_CONST_DWORDS,
            ),
         );
      }

      if (imgso->enabled_mask) {
         OUT_PKT(ring, CP_LOAD_STATE6_FRAG,
            CP_LOAD_STATE6_0(
                  .dst_off     = IR3_BINDLESS_IMAGE_OFFSET,
                  .state_type  = ST6_IBO,
                  .state_src   = SS6_BINDLESS,
                  .state_block = SB6_CS_SHADER,
                  .num_unit    = util_last_bit(imgso->enabled_mask),
            ),
            CP_LOAD_STATE6_EXT_SRC_ADDR(
                  /* This isn't actually an address: */
                  .qword = (idx << 28) |
                     IR3_BINDLESS_IMAGE_OFFSET * FDL6_TEX_CONST_DWORDS,
            ),
         );
      }
   } else {
      OUT_REG(ring, HLSQ_INVALIDATE_CMD(CHIP, .gfx_bindless = 0x1f));
      OUT_REG(ring, SP_BINDLESS_BASE_DESCRIPTOR(CHIP,
            idx, .desc_size = BINDLESS_DESCRIPTOR_64B, .bo = set->bo,
      ));
      OUT_REG(ring, A6XX_HLSQ_BINDLESS_BASE_DESCRIPTOR(
            idx, .desc_size = BINDLESS_DESCRIPTOR_64B, .bo = set->bo,
      ));

      if (bufso->enabled_mask) {
         OUT_PKT(ring, CP_LOAD_STATE6,
            CP_LOAD_STATE6_0(
                  .dst_off     = IR3_BINDLESS_SSBO_OFFSET,
                  .state_type  = ST6_SHADER,
                  .state_src   = SS6_BINDLESS,
                  .state_block = SB6_IBO,
                  .num_unit    = util_last_bit(bufso->enabled_mask),
            ),
            CP_LOAD_STATE6_EXT_SRC_ADDR(
                  /* This isn't actually an address: */
                  .qword = (idx << 28) |
                     IR3_BINDLESS_SSBO_OFFSET * FDL6_TEX_CONST_DWORDS,
            ),
         );
      }

      if (imgso->enabled_mask) {
         OUT_PKT(ring, CP_LOAD_STATE6,
            CP_LOAD_STATE6_0(
                  .dst_off     = IR3_BINDLESS_IMAGE_OFFSET,
                  .state_type  = ST6_SHADER,
                  .state_src   = SS6_BINDLESS,
                  .state_block = SB6_IBO,
                  .num_unit    = util_last_bit(imgso->enabled_mask),
            ),
            CP_LOAD_STATE6_EXT_SRC_ADDR(
                  /* This isn't actually an address: */
                  .qword = (idx << 28) |
                     IR3_BINDLESS_IMAGE_OFFSET * FDL6_TEX_CONST_DWORDS,
            ),
         );
      }
   }

   return ring;
}

template struct fd_ringbuffer *fd6_build_bindless_state<A6XX>(struct fd_context *ctx, enum pipe_shader_type shader, bool append_fb_read);
template struct fd_ringbuffer *fd6_build_bindless_state<A7XX>(struct fd_context *ctx, enum pipe_shader_type shader, bool append_fb_read);

static void
fd6_set_shader_buffers(struct pipe_context *pctx, enum pipe_shader_type shader,
                       unsigned start, unsigned count,
                       const struct pipe_shader_buffer *buffers,
                       unsigned writable_bitmask)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_shaderbuf_stateobj *so = &ctx->shaderbuf[shader];
   struct fd6_descriptor_set *set = descriptor_set(ctx, shader);

   fd_set_shader_buffers(pctx, shader, start, count, buffers, writable_bitmask);

   for (unsigned i = 0; i < count; i++) {
      unsigned n = i + start;
      unsigned slot = n + IR3_BINDLESS_SSBO_OFFSET;
      struct pipe_shader_buffer *buf = &so->sb[n];

      /* invalidate descriptor: */
      set->seqno[slot] = 0;

      if (!buf->buffer) {
         clear_descriptor(set, slot);
         continue;
      }

      /* update descriptor: */
      validate_buffer_descriptor(ctx, set, slot, buf);
   }
}

static void
fd6_set_shader_images(struct pipe_context *pctx, enum pipe_shader_type shader,
                      unsigned start, unsigned count,
                      unsigned unbind_num_trailing_slots,
                      const struct pipe_image_view *images)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_shaderimg_stateobj *so = &ctx->shaderimg[shader];
   struct fd6_descriptor_set *set = descriptor_set(ctx, shader);

   fd_set_shader_images(pctx, shader, start, count, unbind_num_trailing_slots,
                        images);

   for (unsigned i = 0; i < count; i++) {
      unsigned n = i + start;
      unsigned slot = n + IR3_BINDLESS_IMAGE_OFFSET;
      struct pipe_image_view *buf = &so->si[n];

      /* invalidate descriptor: */
      set->seqno[slot] = 0;

      if (!buf->resource) {
         clear_descriptor(set, slot);
         continue;
      }

      struct fd_resource *rsc = fd_resource(buf->resource);

      if (buf->shader_access & (PIPE_IMAGE_ACCESS_COHERENT |
                                PIPE_IMAGE_ACCESS_VOLATILE)) {
         /* UBWC compression cannot be used with coherent/volatile access
          * due to the extra caching (CCU) involved:
          */
         if (rsc->layout.ubwc) {
            bool linear =
                  fd6_check_valid_format(rsc, buf->format) == DEMOTE_TO_LINEAR;

            perf_debug_ctx(ctx,
                           "%" PRSC_FMT ": demoted to %suncompressed due to coherent/volatile use as %s",
                           PRSC_ARGS(&rsc->b.b), linear ? "linear+" : "",
                           util_format_short_name(buf->format));

            fd_resource_uncompress(ctx, rsc, linear);
         }
      } else {
         fd6_validate_format(ctx, rsc, buf->format);
      }

      /* update descriptor: */
      validate_image_descriptor(ctx, set, slot, buf);
   }

   for (unsigned i = 0; i < unbind_num_trailing_slots; i++) {
      unsigned slot = i + start + count + IR3_BINDLESS_IMAGE_OFFSET;

      set->seqno[slot] = 0;
      clear_descriptor(set, slot);
   }
}

void
fd6_image_init(struct pipe_context *pctx)
{
   pctx->set_shader_buffers = fd6_set_shader_buffers;
   pctx->set_shader_images = fd6_set_shader_images;
}
