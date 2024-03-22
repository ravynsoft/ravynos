/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"
#include "d3d12_surface.h"

#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

static D3D12_DSV_DIMENSION
view_dsv_dimension(enum pipe_texture_target target, unsigned samples)
{
   switch (target) {
   case PIPE_TEXTURE_1D: return D3D12_DSV_DIMENSION_TEXTURE1D;
   case PIPE_TEXTURE_1D_ARRAY: return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;

   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      return samples > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS :
                           D3D12_DSV_DIMENSION_TEXTURE2D;

   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return samples > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY :
                           D3D12_DSV_DIMENSION_TEXTURE2DARRAY;

   default:
      unreachable("unexpected target");
   }
}

static D3D12_RTV_DIMENSION
view_rtv_dimension(enum pipe_texture_target target, unsigned samples)
{
   switch (target) {
   case PIPE_BUFFER: return D3D12_RTV_DIMENSION_BUFFER;
   case PIPE_TEXTURE_1D: return D3D12_RTV_DIMENSION_TEXTURE1D;
   case PIPE_TEXTURE_1D_ARRAY: return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;

   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      return samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS :
                           D3D12_RTV_DIMENSION_TEXTURE2D;

   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY :
                           D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

   case PIPE_TEXTURE_3D: return D3D12_RTV_DIMENSION_TEXTURE3D;

   default:
      unreachable("unexpected target");
   }
}

static void
initialize_dsv(struct pipe_context *pctx,
               struct pipe_resource *pres,
               const struct pipe_surface *tpl,
               struct d3d12_descriptor_handle *handle,
               DXGI_FORMAT dxgi_format)
{
   struct d3d12_resource *res = d3d12_resource(pres);
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);

   D3D12_DEPTH_STENCIL_VIEW_DESC desc;
   desc.Format = dxgi_format;
   desc.Flags = D3D12_DSV_FLAG_NONE;

   desc.ViewDimension = view_dsv_dimension(pres->target, pres->nr_samples);
   switch (desc.ViewDimension) {
   case D3D12_DSV_DIMENSION_TEXTURE1D:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 1D DSV from layer %d\n",
                      tpl->u.tex.first_layer);

      desc.Texture1D.MipSlice = tpl->u.tex.level;
      break;

   case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
      desc.Texture1DArray.MipSlice = tpl->u.tex.level;
      desc.Texture1DArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture1DArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   case D3D12_DSV_DIMENSION_TEXTURE2DMS:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 2DMS DSV from layer %d\n",
                      tpl->u.tex.first_layer);

      break;

   case D3D12_DSV_DIMENSION_TEXTURE2D:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 2D DSV from layer %d\n",
                      tpl->u.tex.first_layer);

      desc.Texture2D.MipSlice = tpl->u.tex.level;
      break;

   case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
      desc.Texture2DMSArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture2DMSArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
      desc.Texture2DArray.MipSlice = tpl->u.tex.level;
      desc.Texture2DArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture2DArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   default:
      unreachable("Unhandled DSV dimension");
   }

   mtx_lock(&screen->descriptor_pool_mutex);
   d3d12_descriptor_pool_alloc_handle(screen->dsv_pool, handle);
   mtx_unlock(&screen->descriptor_pool_mutex);

   screen->dev->CreateDepthStencilView(d3d12_resource_resource(res), &desc,
                                       handle->cpu_handle);
}

static void
initialize_rtv(struct pipe_context *pctx,
               struct pipe_resource *pres,
               const struct pipe_surface *tpl,
               struct d3d12_descriptor_handle *handle,
               DXGI_FORMAT dxgi_format)
{
   struct d3d12_resource *res = d3d12_resource(pres);
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);

   D3D12_RENDER_TARGET_VIEW_DESC desc;
   desc.Format = dxgi_format;

   desc.ViewDimension = view_rtv_dimension(pres->target, pres->nr_samples);
   switch (desc.ViewDimension) {
   case D3D12_RTV_DIMENSION_BUFFER:
      desc.Buffer.FirstElement = 0;
      desc.Buffer.NumElements = pres->width0 / util_format_get_blocksize(tpl->format);
      break;

   case D3D12_RTV_DIMENSION_TEXTURE1D:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 1D RTV from layer %d\n",
                      tpl->u.tex.first_layer);

      desc.Texture1D.MipSlice = tpl->u.tex.level;
      break;

   case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
      desc.Texture1DArray.MipSlice = tpl->u.tex.level;
      desc.Texture1DArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture1DArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   case D3D12_RTV_DIMENSION_TEXTURE2DMS:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 2DMS RTV from layer %d\n",
                      tpl->u.tex.first_layer);
      break;

   case D3D12_RTV_DIMENSION_TEXTURE2D:
      if (tpl->u.tex.first_layer > 0)
         debug_printf("D3D12: can't create 2D RTV from layer %d\n",
                      tpl->u.tex.first_layer);

      desc.Texture2D.MipSlice = tpl->u.tex.level;
      desc.Texture2D.PlaneSlice = res->plane_slice;
      break;

   case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
      desc.Texture2DMSArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture2DMSArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
      desc.Texture2DArray.MipSlice = tpl->u.tex.level;
      desc.Texture2DArray.FirstArraySlice = tpl->u.tex.first_layer;
      desc.Texture2DArray.ArraySize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      desc.Texture2DArray.PlaneSlice = 0;
      break;

   case D3D12_RTV_DIMENSION_TEXTURE3D:
      desc.Texture3D.MipSlice = tpl->u.tex.level;
      desc.Texture3D.FirstWSlice = tpl->u.tex.first_layer;
      desc.Texture3D.WSize = tpl->u.tex.last_layer - tpl->u.tex.first_layer + 1;
      break;

   default:
      unreachable("Unhandled RTV dimension");
   }

   mtx_lock(&screen->descriptor_pool_mutex);
   d3d12_descriptor_pool_alloc_handle(screen->rtv_pool, handle);
   mtx_unlock(&screen->descriptor_pool_mutex);

   screen->dev->CreateRenderTargetView(d3d12_resource_resource(res), &desc,
                                       handle->cpu_handle);
}

static struct pipe_surface *
d3d12_create_surface(struct pipe_context *pctx,
                     struct pipe_resource *pres,
                     const struct pipe_surface *tpl)
{
   bool is_depth_or_stencil = util_format_is_depth_or_stencil(tpl->format);
   unsigned bind = is_depth_or_stencil ? PIPE_BIND_DEPTH_STENCIL : PIPE_BIND_RENDER_TARGET;

   /* Don't bother if we don't support the requested format as RT or DS */
   if (!pctx->screen->is_format_supported(pctx->screen, tpl->format, PIPE_TEXTURE_2D,
                                          tpl->nr_samples, tpl->nr_samples,bind))
      return NULL;

   struct d3d12_surface *surface = CALLOC_STRUCT(d3d12_surface);
   if (!surface)
      return NULL;

   pipe_resource_reference(&surface->base.texture, pres);
   pipe_reference_init(&surface->base.reference, 1);
   surface->base.context = pctx;
   surface->base.format = tpl->format;
   surface->base.width = u_minify(pres->width0, tpl->u.tex.level);
   surface->base.height = u_minify(pres->height0, tpl->u.tex.level);
   surface->base.u.tex.level = tpl->u.tex.level;
   surface->base.u.tex.first_layer = tpl->u.tex.first_layer;
   surface->base.u.tex.last_layer = tpl->u.tex.last_layer;

   DXGI_FORMAT dxgi_format = d3d12_get_resource_rt_format(tpl->format);
   if (is_depth_or_stencil)
      initialize_dsv(pctx, pres, tpl, &surface->desc_handle, dxgi_format);
   else
      initialize_rtv(pctx, pres, tpl, &surface->desc_handle, dxgi_format);

   return &surface->base;
}

static void
d3d12_surface_destroy(struct pipe_context *pctx,
                      struct pipe_surface *psurf)
{
   struct d3d12_surface *surface = (struct d3d12_surface*) psurf;
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);

   mtx_lock(&screen->descriptor_pool_mutex);
   d3d12_descriptor_handle_free(&surface->desc_handle);
   if (d3d12_descriptor_handle_is_allocated(&surface->uint_rtv_handle))
      d3d12_descriptor_handle_free(&surface->uint_rtv_handle);
   mtx_unlock(&screen->descriptor_pool_mutex);

   pipe_resource_reference(&psurf->texture, NULL);
   pipe_resource_reference(&surface->rgba_texture, NULL);
   FREE(surface);
}

static void
blit_surface(struct pipe_context *pctx, struct d3d12_surface *surface, bool pre)
{
   struct pipe_blit_info info = {};

   info.src.resource = pre ? surface->base.texture : surface->rgba_texture;
   info.dst.resource = pre ? surface->rgba_texture : surface->base.texture;
   info.src.format = pre ? surface->base.texture->format : PIPE_FORMAT_R8G8B8A8_UNORM;
   info.dst.format = pre ? PIPE_FORMAT_R8G8B8A8_UNORM : surface->base.texture->format;
   info.src.level = info.dst.level = 0;
   info.src.box.x = info.dst.box.x = 0;
   info.src.box.y = info.dst.box.y = 0;
   info.src.box.z = info.dst.box.z = 0;
   info.src.box.width = info.dst.box.width = surface->base.width;
   info.src.box.height = info.dst.box.height = surface->base.height;
   info.src.box.depth = info.dst.box.depth = 0;
   info.mask = PIPE_MASK_RGBA;

   d3d12_blit(pctx, &info);
}

enum d3d12_surface_conversion_mode
d3d12_surface_update_pre_draw(struct pipe_context *pctx,
                              struct d3d12_surface *surface,
                              DXGI_FORMAT format)
{
   struct d3d12_screen *screen = d3d12_screen(surface->base.context->screen);
   struct d3d12_resource *res = d3d12_resource(surface->base.texture);
   DXGI_FORMAT dxgi_format = d3d12_get_resource_rt_format(surface->base.format);
   enum d3d12_surface_conversion_mode mode;

   if (dxgi_format == format)
      return D3D12_SURFACE_CONVERSION_NONE;

   if (dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM ||
       dxgi_format == DXGI_FORMAT_B8G8R8X8_UNORM)
      mode = D3D12_SURFACE_CONVERSION_BGRA_UINT;
   else
      mode = D3D12_SURFACE_CONVERSION_RGBA_UINT;

   if (mode == D3D12_SURFACE_CONVERSION_BGRA_UINT) {
      if (!surface->rgba_texture) {
         struct pipe_resource templ = {};
         struct pipe_resource *src = surface->base.texture;

         templ.format = PIPE_FORMAT_R8G8B8A8_UNORM;
         templ.width0 = src->width0;
         templ.height0 = src->height0;
         templ.depth0 = src->depth0;
         templ.array_size = src->array_size;
         templ.nr_samples = src->nr_samples;
         templ.nr_storage_samples = src->nr_storage_samples;
         templ.usage = PIPE_USAGE_DEFAULT | PIPE_USAGE_STAGING;
         templ.bind = src->bind;
         templ.target = src->target;

         surface->rgba_texture = screen->base.resource_create(&screen->base, &templ);
      }

      blit_surface(pctx, surface, true);
      res = d3d12_resource(surface->rgba_texture);
   }

   if (!d3d12_descriptor_handle_is_allocated(&surface->uint_rtv_handle)) {
      initialize_rtv(surface->base.context, &res->base.b, &surface->base,
                     &surface->uint_rtv_handle, DXGI_FORMAT_R8G8B8A8_UINT);
   }

   return mode;
}

void
d3d12_surface_update_post_draw(struct pipe_context *pctx,
                               struct d3d12_surface *surface,
                               enum d3d12_surface_conversion_mode mode)
{
   if (mode == D3D12_SURFACE_CONVERSION_BGRA_UINT)
      blit_surface(pctx, surface, false);
}

D3D12_CPU_DESCRIPTOR_HANDLE
d3d12_surface_get_handle(struct d3d12_surface *surface,
                         enum d3d12_surface_conversion_mode mode)
{
   if (mode != D3D12_SURFACE_CONVERSION_NONE)
      return surface->uint_rtv_handle.cpu_handle;
   return surface->desc_handle.cpu_handle;
}

void
d3d12_context_surface_init(struct pipe_context *context)
{
   context->create_surface = d3d12_create_surface;
   context->surface_destroy = d3d12_surface_destroy;
}
