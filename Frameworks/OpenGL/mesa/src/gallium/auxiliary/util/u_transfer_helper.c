/*
 * Copyright © 2017 Red Hat
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
 */

#include "pipe/p_screen.h"

#include "util/u_box.h"
#include "util/format/u_format.h"
#include "util/format/u_format_zs.h"
#include "util/u_inlines.h"
#include "util/u_transfer_helper.h"


struct u_transfer_helper {
   const struct u_transfer_vtbl *vtbl;
   bool separate_z32s8; /**< separate z32 and s8 */
   bool separate_stencil; /**< separate stencil for all formats */
   bool msaa_map;
   bool z24_in_z32f; /* the z24 values are stored in a z32 - translate them. */
   bool interleave_in_place;
};

/* If we need to take the path for PIPE_MAP_DEPTH/STENCIL_ONLY on the parent
 * depth/stencil resource an interleaving those to/from a staging buffer. The
 * other path for z/s interleave is when separate z and s resources are
 * created at resource create time.
 */
static inline bool needs_in_place_zs_interleave(struct u_transfer_helper *helper,
                                                enum pipe_format format)
{
   if (!helper->interleave_in_place)
      return false;
   if (helper->separate_stencil && util_format_is_depth_and_stencil(format))
      return true;
   if (helper->separate_z32s8 && format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
      return true;
   /* this isn't interleaving, but still needs conversions on that path. */
   if (helper->z24_in_z32f && format == PIPE_FORMAT_Z24X8_UNORM)
      return true;
   return false;
}

static inline bool handle_transfer(struct pipe_resource *prsc)
{
   struct u_transfer_helper *helper = prsc->screen->transfer_helper;

   if (helper->vtbl->get_internal_format) {
      enum pipe_format internal_format =
            helper->vtbl->get_internal_format(prsc);
      if (internal_format != prsc->format)
         return true;
   }

   if (helper->msaa_map && (prsc->nr_samples > 1))
      return true;

   if (needs_in_place_zs_interleave(helper, prsc->format))
      return true;

   return false;
}

/* The pipe_transfer ptr could either be the driver's, or u_transfer,
 * depending on whether we are intervening or not.  Check handle_transfer()
 * before dereferencing.
 */
struct u_transfer {
   struct pipe_transfer base;
   /* Note that in case of MSAA resolve for transfer plus z32s8
    * we end up with stacked u_transfer's.  The MSAA resolve case doesn't call
    * helper->vtbl fxns directly, but calls back to pctx->transfer_map()/etc
    * so the format related handling can work in conjunction with MSAA resolve.
    */
   struct pipe_transfer *trans;   /* driver's transfer */
   struct pipe_transfer *trans2;  /* 2nd transfer for s8 stencil buffer in z32s8 */
   void *ptr, *ptr2;              /* ptr to trans, and trans2 */
   void *staging;                 /* staging buffer */
   struct pipe_resource *ss;      /* staging resource for MSAA resolves */
};

static inline struct u_transfer *
u_transfer(struct pipe_transfer *ptrans)
{
   assert(handle_transfer(ptrans->resource));
   return (struct u_transfer *)ptrans;
}

struct pipe_resource *
u_transfer_helper_resource_create(struct pipe_screen *pscreen,
                                  const struct pipe_resource *templ)
{
   struct u_transfer_helper *helper = pscreen->transfer_helper;
   enum pipe_format format = templ->format;
   struct pipe_resource *prsc;

   if (((helper->separate_stencil && util_format_is_depth_and_stencil(format)) ||
        (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT && helper->separate_z32s8)) &&
       !helper->interleave_in_place) {
      struct pipe_resource t = *templ;
      struct pipe_resource *stencil;

      t.format = util_format_get_depth_only(format);

      if (t.format == PIPE_FORMAT_Z24X8_UNORM && helper->z24_in_z32f)
         t.format = PIPE_FORMAT_Z32_FLOAT;

      prsc = helper->vtbl->resource_create(pscreen, &t);
      if (!prsc)
         return NULL;

      prsc->format = format;  /* frob the format back to the "external" format */

      t.format = PIPE_FORMAT_S8_UINT;
      stencil = helper->vtbl->resource_create(pscreen, &t);

      if (!stencil) {
         helper->vtbl->resource_destroy(pscreen, prsc);
         return NULL;
      }

      helper->vtbl->set_stencil(prsc, stencil);
   } else if (format == PIPE_FORMAT_Z24X8_UNORM && helper->z24_in_z32f) {
      struct pipe_resource t = *templ;
      t.format = PIPE_FORMAT_Z32_FLOAT;

      prsc = helper->vtbl->resource_create(pscreen, &t);
      if (!prsc)
         return NULL;

      prsc->format = format;  /* frob the format back to the "external" format */
   } else {
      /* normal case, no special handling: */
      prsc = helper->vtbl->resource_create(pscreen, templ);
      if (!prsc)
         return NULL;
   }

   return prsc;
}

void
u_transfer_helper_resource_destroy(struct pipe_screen *pscreen,
                                   struct pipe_resource *prsc)
{
   struct u_transfer_helper *helper = pscreen->transfer_helper;

   if (helper->vtbl->get_stencil && !helper->interleave_in_place) {
      struct pipe_resource *stencil = helper->vtbl->get_stencil(prsc);

      pipe_resource_reference(&stencil, NULL);
   }

   helper->vtbl->resource_destroy(pscreen, prsc);
}

static bool needs_pack(unsigned usage)
{
   return (usage & PIPE_MAP_READ) &&
      !(usage & (PIPE_MAP_DISCARD_WHOLE_RESOURCE | PIPE_MAP_DISCARD_RANGE));
}

/* In the case of transfer_map of a multi-sample resource, call back into
 * pctx->transfer_map() to map the staging resource, to handle cases of
 * MSAA + separate_z32s8
 */
static void *
transfer_map_msaa(struct pipe_context *pctx,
                  struct pipe_resource *prsc,
                  unsigned level, unsigned usage,
                  const struct pipe_box *box,
                  struct pipe_transfer **pptrans)
{
   struct pipe_screen *pscreen = pctx->screen;
   struct u_transfer *trans = calloc(1, sizeof(*trans));
   if (!trans)
      return NULL;
   struct pipe_transfer *ptrans = &trans->base;

   pipe_resource_reference(&ptrans->resource, prsc);
   ptrans->level = level;
   ptrans->usage = usage;
   ptrans->box = *box;

   struct pipe_resource tmpl = {
         .target = prsc->target,
         .format = prsc->format,
         .width0 = box->width,
         .height0 = box->height,
         .depth0 = 1,
         .array_size = 1,
   };
   if (util_format_is_depth_or_stencil(tmpl.format))
      tmpl.bind |= PIPE_BIND_DEPTH_STENCIL;
   else
      tmpl.bind |= PIPE_BIND_RENDER_TARGET;
   trans->ss = pscreen->resource_create(pscreen, &tmpl);
   if (!trans->ss) {
      free(trans);
      return NULL;
   }

   if (needs_pack(usage)) {
      struct pipe_blit_info blit;
      memset(&blit, 0, sizeof(blit));

      blit.src.resource = ptrans->resource;
      blit.src.format = ptrans->resource->format;
      blit.src.level = ptrans->level;
      blit.src.box = *box;

      blit.dst.resource = trans->ss;
      blit.dst.format = trans->ss->format;
      blit.dst.box.width = box->width;
      blit.dst.box.height = box->height;
      blit.dst.box.depth = 1;

      blit.mask = util_format_get_mask(prsc->format);
      blit.filter = PIPE_TEX_FILTER_NEAREST;

      pctx->blit(pctx, &blit);
   }

   struct pipe_box map_box = *box;
   map_box.x = 0;
   map_box.y = 0;

   void *ss_map = pctx->texture_map(pctx, trans->ss, 0, usage, &map_box,
         &trans->trans);
   if (!ss_map) {
      free(trans);
      return NULL;
   }

   ptrans->stride = trans->trans->stride;
   *pptrans = ptrans;
   return ss_map;
}

void *
u_transfer_helper_transfer_map(struct pipe_context *pctx,
                               struct pipe_resource *prsc,
                               unsigned level, unsigned usage,
                               const struct pipe_box *box,
                               struct pipe_transfer **pptrans)
{
   struct u_transfer_helper *helper = pctx->screen->transfer_helper;
   struct u_transfer *trans;
   struct pipe_transfer *ptrans;
   enum pipe_format format = prsc->format;
   unsigned width = box->width;
   unsigned height = box->height;
   bool in_place_zs_interleave = needs_in_place_zs_interleave(helper, format);

   if (!handle_transfer(prsc))
      return helper->vtbl->transfer_map(pctx, prsc, level, usage, box, pptrans);

   if (helper->msaa_map && (prsc->nr_samples > 1))
      return transfer_map_msaa(pctx, prsc, level, usage, box, pptrans);

   assert(box->depth == 1);

   trans = calloc(1, sizeof(*trans));
   if (!trans)
      return NULL;

   ptrans = &trans->base;
   pipe_resource_reference(&ptrans->resource, prsc);
   ptrans->level = level;
   ptrans->usage = usage;
   ptrans->box   = *box;
   ptrans->stride = util_format_get_stride(format, box->width);
   ptrans->layer_stride = (uint64_t)ptrans->stride * box->height;

   trans->staging = malloc(ptrans->layer_stride);
   if (!trans->staging)
      goto fail;

   trans->ptr = helper->vtbl->transfer_map(pctx, prsc, level,
                                           usage | (in_place_zs_interleave ? PIPE_MAP_DEPTH_ONLY : 0),
                                           box, &trans->trans);
   if (!trans->ptr)
      goto fail;

   if (util_format_is_depth_and_stencil(prsc->format)) {
      struct pipe_resource *stencil;

      if (in_place_zs_interleave)
         stencil = prsc;
     else
         stencil = helper->vtbl->get_stencil(prsc);
      trans->ptr2 = helper->vtbl->transfer_map(pctx, stencil, level,
                                               usage | (in_place_zs_interleave ? PIPE_MAP_STENCIL_ONLY : 0),
                                               box, &trans->trans2);

      if (needs_pack(usage)) {
         switch (prsc->format) {
         case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
            util_format_z32_float_s8x24_uint_pack_z_float(trans->staging,
                                                          ptrans->stride,
                                                          trans->ptr,
                                                          trans->trans->stride,
                                                          width, height);
            util_format_z32_float_s8x24_uint_pack_s_8uint(trans->staging,
                                                          ptrans->stride,
                                                          trans->ptr2,
                                                          trans->trans2->stride,
                                                          width, height);
            break;
         case PIPE_FORMAT_Z24_UNORM_S8_UINT:
            if (in_place_zs_interleave) {
               if (helper->z24_in_z32f) {
                  util_format_z24_unorm_s8_uint_pack_separate_z32(trans->staging,
                                                                  ptrans->stride,
                                                                  trans->ptr,
                                                                  trans->trans->stride,
                                                                  trans->ptr2,
                                                                  trans->trans2->stride,
                                                                  width, height);
               } else {
                  util_format_z24_unorm_s8_uint_pack_separate(trans->staging,
                                                             ptrans->stride,
                                                             trans->ptr,
                                                             trans->trans->stride,
                                                             trans->ptr2,
                                                             trans->trans2->stride,
                                                             width, height);
               }
            } else {
               if (helper->z24_in_z32f) {
                  util_format_z24_unorm_s8_uint_pack_z_float(trans->staging,
                                                             ptrans->stride,
                                                             trans->ptr,
                                                             trans->trans->stride,
                                                             width, height);
                  util_format_z24_unorm_s8_uint_pack_s_8uint(trans->staging,
                                                             ptrans->stride,
                                                             trans->ptr2,
                                                             trans->trans2->stride,
                                                             width, height);
               } else {
                  util_format_z24_unorm_s8_uint_pack_separate(trans->staging,
                                                              ptrans->stride,
                                                              trans->ptr,
                                                              trans->trans->stride,
                                                              trans->ptr2,
                                                              trans->trans2->stride,
                                                              width, height);
               }
            }
            break;
         case PIPE_FORMAT_Z24X8_UNORM:
            assert(helper->z24_in_z32f);
            util_format_z24x8_unorm_pack_z_float(trans->staging, ptrans->stride,
                                                trans->ptr, trans->trans->stride,
                                                width, height);
            break;
         default:
            unreachable("Unexpected format");
         }
      }
   } else if (prsc->format == PIPE_FORMAT_Z24X8_UNORM) {
         assert(helper->z24_in_z32f);
         util_format_z24x8_unorm_pack_z_float(trans->staging, ptrans->stride,
                                              trans->ptr, trans->trans->stride,
                                              width, height);
   } else {
      unreachable("bleh");
   }

   *pptrans = ptrans;
   return trans->staging;

fail:
   if (trans->trans)
      helper->vtbl->transfer_unmap(pctx, trans->trans);
   if (trans->trans2)
      helper->vtbl->transfer_unmap(pctx, trans->trans2);
   pipe_resource_reference(&ptrans->resource, NULL);
   free(trans->staging);
   free(trans);
   return NULL;
}

static void
flush_region(struct pipe_context *pctx, struct pipe_transfer *ptrans,
             const struct pipe_box *box)
{
   struct u_transfer_helper *helper = pctx->screen->transfer_helper;
   /* using the function here hits an assert for the deinterleave cases */
   struct u_transfer *trans = (struct u_transfer *)ptrans;
   enum pipe_format iformat, format = ptrans->resource->format;
   unsigned width = box->width;
   unsigned height = box->height;
   void *src, *dst;

   if (!(ptrans->usage & PIPE_MAP_WRITE))
      return;

   if (trans->ss) {
      struct pipe_blit_info blit;
      memset(&blit, 0, sizeof(blit));

      blit.src.resource = trans->ss;
      blit.src.format = trans->ss->format;
      blit.src.box = *box;

      blit.dst.resource = ptrans->resource;
      blit.dst.format = ptrans->resource->format;
      blit.dst.level = ptrans->level;

      u_box_2d(ptrans->box.x + box->x,
               ptrans->box.y + box->y,
               box->width, box->height,
               &blit.dst.box);

      blit.mask = util_format_get_mask(ptrans->resource->format);
      blit.filter = PIPE_TEX_FILTER_NEAREST;

      pctx->blit(pctx, &blit);

      return;
   }

   iformat = helper->vtbl->get_internal_format(ptrans->resource);

   src = (uint8_t *)trans->staging +
         (box->y * ptrans->stride) +
         (box->x * util_format_get_blocksize(format));
   dst = (uint8_t *)trans->ptr +
         (box->y * trans->trans->stride) +
         (box->x * util_format_get_blocksize(iformat));

   switch (format) {
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      util_format_z32_float_s8x24_uint_unpack_z_float(dst,
                                                      trans->trans->stride,
                                                      src,
                                                      ptrans->stride,
                                                      width, height);
      FALLTHROUGH;
   case PIPE_FORMAT_X32_S8X24_UINT:
      dst = (uint8_t *)trans->ptr2 +
            (box->y * trans->trans2->stride) +
            (box->x * util_format_get_blocksize(PIPE_FORMAT_S8_UINT));

      util_format_z32_float_s8x24_uint_unpack_s_8uint(dst,
                                                      trans->trans2->stride,
                                                      src,
                                                      ptrans->stride,
                                                      width, height);
      break;
   case PIPE_FORMAT_Z24X8_UNORM:
      util_format_z24x8_unorm_unpack_z_float(dst, trans->trans->stride,
                                             src, ptrans->stride,
                                             width, height);
      break;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (helper->z24_in_z32f) {
         util_format_z24_unorm_s8_uint_unpack_z_float(dst, trans->trans->stride,
                                                      src, ptrans->stride,
                                                      width, height);
      } else {
         /* just do a strided 32-bit copy for depth; s8 can become garbage x8 */
         util_format_z32_unorm_unpack_z_32unorm(dst, trans->trans->stride,
                                                src, ptrans->stride,
                                                width, height);
      }
      FALLTHROUGH;
   case PIPE_FORMAT_X24S8_UINT:
      dst = (uint8_t *)trans->ptr2 +
            (box->y * trans->trans2->stride) +
            (box->x * util_format_get_blocksize(PIPE_FORMAT_S8_UINT));

      util_format_z24_unorm_s8_uint_unpack_s_8uint(dst, trans->trans2->stride,
                                                   src, ptrans->stride,
                                                   width, height);
      break;

   default:
      assert(!"Unexpected staging transfer type");
      break;
   }
}

void
u_transfer_helper_transfer_flush_region(struct pipe_context *pctx,
                                        struct pipe_transfer *ptrans,
                                        const struct pipe_box *box)
{
   struct u_transfer_helper *helper = pctx->screen->transfer_helper;

   if (handle_transfer(ptrans->resource)) {
      struct u_transfer *trans = u_transfer(ptrans);

      /* handle MSAA case, since there could be multiple levels of
       * wrapped transfer, call pctx->transfer_flush_region()
       * instead of helper->vtbl->transfer_flush_region()
       */
      if (trans->ss) {
         pctx->transfer_flush_region(pctx, trans->trans, box);
         flush_region(pctx, ptrans, box);
         return;
      }

      flush_region(pctx, ptrans, box);

      helper->vtbl->transfer_flush_region(pctx, trans->trans, box);
      if (trans->trans2)
         helper->vtbl->transfer_flush_region(pctx, trans->trans2, box);

   } else {
      helper->vtbl->transfer_flush_region(pctx, ptrans, box);
   }
}

void
u_transfer_helper_transfer_unmap(struct pipe_context *pctx,
                                 struct pipe_transfer *ptrans)
{
   struct u_transfer_helper *helper = pctx->screen->transfer_helper;

   if (handle_transfer(ptrans->resource)) {
      struct u_transfer *trans = u_transfer(ptrans);

      if (!(ptrans->usage & PIPE_MAP_FLUSH_EXPLICIT)) {
         struct pipe_box box;
         u_box_2d(0, 0, ptrans->box.width, ptrans->box.height, &box);
         if (trans->ss)
            pctx->transfer_flush_region(pctx, trans->trans, &box);
         flush_region(pctx, ptrans, &box);
      }

      /* in MSAA case, there could be multiple levels of wrapping
       * so don't call helper->vtbl->transfer_unmap() directly
       */
      if (trans->ss) {
         pctx->texture_unmap(pctx, trans->trans);
         pipe_resource_reference(&trans->ss, NULL);
      } else {
         helper->vtbl->transfer_unmap(pctx, trans->trans);
         if (trans->trans2)
            helper->vtbl->transfer_unmap(pctx, trans->trans2);
      }

      pipe_resource_reference(&ptrans->resource, NULL);

      free(trans->staging);
      free(trans);
   } else {
      helper->vtbl->transfer_unmap(pctx, ptrans);
   }
}

struct u_transfer_helper *
u_transfer_helper_create(const struct u_transfer_vtbl *vtbl,
                         enum u_transfer_helper_flags flags)
{
   struct u_transfer_helper *helper = calloc(1, sizeof(*helper));

   helper->vtbl = vtbl;
   helper->separate_z32s8 = flags & U_TRANSFER_HELPER_SEPARATE_Z32S8;
   helper->separate_stencil = flags & U_TRANSFER_HELPER_SEPARATE_STENCIL;
   helper->msaa_map = flags & U_TRANSFER_HELPER_MSAA_MAP;
   helper->z24_in_z32f = flags & U_TRANSFER_HELPER_Z24_IN_Z32F;
   helper->interleave_in_place = flags & U_TRANSFER_HELPER_INTERLEAVE_IN_PLACE;

   return helper;
}

void
u_transfer_helper_destroy(struct u_transfer_helper *helper)
{
   free(helper);
}
