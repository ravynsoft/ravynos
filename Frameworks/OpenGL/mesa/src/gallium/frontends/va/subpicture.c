/**************************************************************************
 *
 * Copyright 2010 Thomas Balling Sørensen & Orasanu Lucian.
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/u_memory.h"
#include "util/u_handle_table.h"
#include "util/u_sampler.h"

#include "va_private.h"

static VAImageFormat subpic_formats[] = {
   {
   .fourcc = VA_FOURCC_BGRA,
   .byte_order = VA_LSB_FIRST,
   .bits_per_pixel = 32,
   .depth = 32,
   .red_mask   = 0x00ff0000ul,
   .green_mask = 0x0000ff00ul,
   .blue_mask  = 0x000000fful,
   .alpha_mask = 0xff000000ul,
   },
};

VAStatus
vlVaQuerySubpictureFormats(VADriverContextP ctx, VAImageFormat *format_list,
                           unsigned int *flags, unsigned int *num_formats)
{
   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   if (!(format_list && flags && num_formats))
      return VA_STATUS_ERROR_UNKNOWN;

   *num_formats = sizeof(subpic_formats)/sizeof(VAImageFormat);
   memcpy(format_list, subpic_formats, sizeof(subpic_formats));

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaCreateSubpicture(VADriverContextP ctx, VAImageID image,
                     VASubpictureID *subpicture)
{
   vlVaDriver *drv;
   vlVaSubpicture *sub;
   VAImage *img;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);
   img = handle_table_get(drv->htab, image);
   if (!img) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_IMAGE;
   }

   sub = CALLOC(1, sizeof(*sub));
   if (!sub) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_ALLOCATION_FAILED;
   }

   sub->image = img;
   *subpicture = handle_table_add(VL_VA_DRIVER(ctx)->htab, sub);
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaDestroySubpicture(VADriverContextP ctx, VASubpictureID subpicture)
{
   vlVaDriver *drv;
   vlVaSubpicture *sub;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);

   sub = handle_table_get(drv->htab, subpicture);
   if (!sub) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_SUBPICTURE;
   }

   FREE(sub);
   handle_table_remove(drv->htab, subpicture);
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaSubpictureImage(VADriverContextP ctx, VASubpictureID subpicture, VAImageID image)
{
   vlVaDriver *drv;
   vlVaSubpicture *sub;
   VAImage *img;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);

   img = handle_table_get(drv->htab, image);
   if (!img) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_IMAGE;
   }

   sub = handle_table_get(drv->htab, subpicture);
   mtx_unlock(&drv->mutex);
   if (!sub)
      return VA_STATUS_ERROR_INVALID_SUBPICTURE;

   sub->image = img;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaSetSubpictureChromakey(VADriverContextP ctx, VASubpictureID subpicture,
                           unsigned int chromakey_min, unsigned int chromakey_max, unsigned int chromakey_mask)
{
   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
vlVaSetSubpictureGlobalAlpha(VADriverContextP ctx, VASubpictureID subpicture, float global_alpha)
{
   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
vlVaAssociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture,
                        VASurfaceID *target_surfaces, int num_surfaces,
                        short src_x, short src_y, unsigned short src_width,
                        unsigned short src_height, short dest_x, short dest_y,
                        unsigned short dest_width, unsigned short dest_height,
                        unsigned int flags)
{
   vlVaSubpicture *sub;
   struct pipe_resource tex_temp, *tex;
   struct pipe_sampler_view sampler_templ;
   vlVaDriver *drv;
   vlVaSurface *surf;
   int i;
   struct u_rect src_rect = {src_x, src_x + src_width, src_y, src_y + src_height};
   struct u_rect dst_rect = {dest_x, dest_x + dest_width, dest_y, dest_y + dest_height};

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;
   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);

   sub = handle_table_get(drv->htab, subpicture);
   if (!sub) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_SUBPICTURE;
   }

   for (i = 0; i < num_surfaces; i++) {
      surf = handle_table_get(drv->htab, target_surfaces[i]);
      if (!surf) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_INVALID_SURFACE;
      }
   }

   sub->src_rect = src_rect;
   sub->dst_rect = dst_rect;

   memset(&tex_temp, 0, sizeof(tex_temp));
   tex_temp.target = PIPE_TEXTURE_2D;
   tex_temp.format = PIPE_FORMAT_B8G8R8A8_UNORM;
   tex_temp.last_level = 0;
   tex_temp.width0 = src_width;
   tex_temp.height0 = src_height;
   tex_temp.depth0 = 1;
   tex_temp.array_size = 1;
   tex_temp.usage = PIPE_USAGE_DYNAMIC;
   tex_temp.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
   tex_temp.flags = 0;
   if (!drv->pipe->screen->is_format_supported(
          drv->pipe->screen, tex_temp.format, tex_temp.target,
          tex_temp.nr_samples, tex_temp.nr_storage_samples, tex_temp.bind)) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_ALLOCATION_FAILED;
   }

   tex = drv->pipe->screen->resource_create(drv->pipe->screen, &tex_temp);

   memset(&sampler_templ, 0, sizeof(sampler_templ));
   u_sampler_view_default_template(&sampler_templ, tex, tex->format);
   sub->sampler = drv->pipe->create_sampler_view(drv->pipe, tex, &sampler_templ);
   pipe_resource_reference(&tex, NULL);
   if (!sub->sampler) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_ALLOCATION_FAILED;
   }

   for (i = 0; i < num_surfaces; i++) {
      surf = handle_table_get(drv->htab, target_surfaces[i]);
      if (!surf) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_INVALID_SURFACE;
      }
      util_dynarray_append(&surf->subpics, vlVaSubpicture *, sub);
   }
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaDeassociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture,
                          VASurfaceID *target_surfaces, int num_surfaces)
{
   int i;
   int j;
   vlVaSurface *surf;
   vlVaSubpicture *sub, **array;
   vlVaDriver *drv;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;
   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);

   sub = handle_table_get(drv->htab, subpicture);
   if (!sub) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_SUBPICTURE;
   }

   for (i = 0; i < num_surfaces; i++) {
      surf = handle_table_get(drv->htab, target_surfaces[i]);
      if (!surf) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_INVALID_SURFACE;
      }

      array = surf->subpics.data;
      if (!array)
         continue;

      for (j = 0; j < surf->subpics.size/sizeof(vlVaSubpicture *); j++) {
         if (array[j] == sub)
            array[j] = NULL;
      }

      while (surf->subpics.size && util_dynarray_top(&surf->subpics, vlVaSubpicture *) == NULL)
         (void)util_dynarray_pop(&surf->subpics, vlVaSubpicture *);
   }
   pipe_sampler_view_reference(&sub->sampler,NULL);
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}
