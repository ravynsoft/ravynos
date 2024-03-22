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

#include "d3d12_resource.h"

#include "d3d12_blit.h"
#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_screen.h"
#include "d3d12_debug.h"

#include "pipebuffer/pb_bufmgr.h"
#include "util/slab.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/format/u_format_zs.h"

#include "frontend/sw_winsys.h"

#include <dxguids/dxguids.h>
#include <memory>

#ifndef _GAMING_XBOX
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

#ifndef GENERIC_ALL
 // This is only added to winadapter.h in newer DirectX-Headers
#define GENERIC_ALL 0x10000000L
#endif

static bool
can_map_directly(struct pipe_resource *pres)
{
   return pres->target == PIPE_BUFFER &&
          pres->usage != PIPE_USAGE_DEFAULT &&
          pres->usage != PIPE_USAGE_IMMUTABLE;
}

static void
init_valid_range(struct d3d12_resource *res)
{
   if (can_map_directly(&res->base.b))
      util_range_init(&res->valid_buffer_range);
}

static void
d3d12_resource_destroy(struct pipe_screen *pscreen,
                       struct pipe_resource *presource)
{
   struct d3d12_resource *resource = d3d12_resource(presource);

   // When instanciating a planar d3d12_resource, the same resource->dt pointer
   // is copied to all their planes linked list resources
   // Different instances of objects like d3d12_surface, can be pointing
   // to different planes of the same overall (ie. NV12) planar d3d12_resource
   // sharing the same dt, so keep a refcount when destroying them
   // and only destroy it on the last plane being destroyed
   if (resource->dt_refcount > 0)
      resource->dt_refcount--;
   if ((resource->dt_refcount == 0) && resource->dt)
   {
      struct d3d12_screen *screen = d3d12_screen(pscreen);
      screen->winsys->displaytarget_destroy(screen->winsys, resource->dt);
   }

   threaded_resource_deinit(presource);
   if (can_map_directly(presource))
      util_range_destroy(&resource->valid_buffer_range);
   if (resource->bo)
      d3d12_bo_unreference(resource->bo);
   FREE(resource);
}

static bool
resource_is_busy(struct d3d12_context *ctx,
                 struct d3d12_resource *res,
                 bool want_to_write)
{
   if (d3d12_batch_has_references(d3d12_current_batch(ctx), res->bo, want_to_write))
      return true;

   bool busy = false;
   d3d12_foreach_submitted_batch(ctx, batch) {
      if (!d3d12_reset_batch(ctx, batch, 0))
         busy |= d3d12_batch_has_references(batch, res->bo, want_to_write);
   }
   return busy;
}

void
d3d12_resource_wait_idle(struct d3d12_context *ctx,
                         struct d3d12_resource *res,
                         bool want_to_write)
{
   if (d3d12_batch_has_references(d3d12_current_batch(ctx), res->bo, want_to_write)) {
      d3d12_flush_cmdlist_and_wait(ctx);
   } else {
      d3d12_foreach_submitted_batch(ctx, batch) {
         if (d3d12_batch_has_references(batch, res->bo, want_to_write))
            d3d12_reset_batch(ctx, batch, OS_TIMEOUT_INFINITE);
      }
   }
}

void
d3d12_resource_release(struct d3d12_resource *resource)
{
   if (!resource->bo)
      return;
   d3d12_bo_unreference(resource->bo);
   resource->bo = NULL;
}

static bool
init_buffer(struct d3d12_screen *screen,
            struct d3d12_resource *res,
            const struct pipe_resource *templ)
{
   struct pb_desc buf_desc;
   struct pb_manager *bufmgr;
   struct pb_buffer *buf;

   /* Assert that we don't want to create a buffer with one of the emulated
    * formats, these are (currently) only supported when passing the vertex
    * element state */
   assert(templ->format == d3d12_emulated_vtx_format(templ->format));

   if ((templ->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) &&
       res->base.b.usage == PIPE_USAGE_DEFAULT)
   {
      res->base.b.usage = PIPE_USAGE_STAGING;
   }
   switch (res->base.b.usage) {
   case PIPE_USAGE_DEFAULT:
   case PIPE_USAGE_IMMUTABLE:
      bufmgr = screen->cache_bufmgr;
      buf_desc.usage = (pb_usage_flags)PB_USAGE_GPU_READ_WRITE;
      break;
   case PIPE_USAGE_DYNAMIC:
   case PIPE_USAGE_STREAM:
      bufmgr = screen->slab_bufmgr;
      buf_desc.usage = (pb_usage_flags)(PB_USAGE_CPU_WRITE | PB_USAGE_GPU_READ);
      break;
   case PIPE_USAGE_STAGING:
      bufmgr = screen->readback_slab_bufmgr;
      buf_desc.usage = (pb_usage_flags)(PB_USAGE_GPU_WRITE | PB_USAGE_CPU_READ_WRITE);
      break;
   default:
      unreachable("Invalid pipe usage");
   }

   /* We can't suballocate buffers that might be bound as a sampler view, *only*
    * because in the case of R32G32B32 formats (12 bytes per pixel), it's not possible
    * to guarantee the offset will be divisible.
    */
   if (templ->bind & PIPE_BIND_SAMPLER_VIEW)
      bufmgr = screen->cache_bufmgr;

   buf_desc.alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
   res->dxgi_format = DXGI_FORMAT_UNKNOWN;
   buf = bufmgr->create_buffer(bufmgr, templ->width0, &buf_desc);
   if (!buf)
      return false;
   res->bo = d3d12_bo_wrap_buffer(screen, buf);

   return true;
}

static bool
init_texture(struct d3d12_screen *screen,
             struct d3d12_resource *res,
             const struct pipe_resource *templ,
             ID3D12Heap *heap,
             uint64_t placed_offset)
{
   ID3D12Resource *d3d12_res;

   res->mip_levels = templ->last_level + 1;
   res->dxgi_format = d3d12_get_format(templ->format);

   D3D12_RESOURCE_DESC desc;
   desc.Format = res->dxgi_format;
   desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   desc.Width = templ->width0;
   desc.Height = templ->height0;
   desc.DepthOrArraySize = templ->array_size;
   desc.MipLevels = templ->last_level + 1;

   desc.SampleDesc.Count = MAX2(templ->nr_samples, 1);
   desc.SampleDesc.Quality = 0;

   desc.Flags = D3D12_RESOURCE_FLAG_NONE;
   desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

   switch (templ->target) {
   case PIPE_BUFFER:
      desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      break;

   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
      break;

   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_RECT:
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      break;

   case PIPE_TEXTURE_3D:
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
      desc.DepthOrArraySize = templ->depth0;
      break;

   default:
      unreachable("Invalid texture type");
   }

   if (templ->bind & PIPE_BIND_SHADER_BUFFER)
      desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

   if (templ->bind & PIPE_BIND_RENDER_TARGET)
      desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

   if (templ->bind & PIPE_BIND_DEPTH_STENCIL) {
      desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

      /* Sadly, we can't set D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE in the
       * case where PIPE_BIND_SAMPLER_VIEW isn't set, because that would
       * prevent us from using the resource with u_blitter, which requires
       * sneaking in sampler-usage throught the back-door.
       */
   }

   const DXGI_FORMAT *format_cast_list = NULL;
   uint32_t num_castable_formats = 0;

   if (screen->opts12.RelaxedFormatCastingSupported) {
      /* All formats that fall into a cast set need to be castable and accessible as a shader image. */
      format_cast_list = d3d12_get_format_cast_list(templ->format, &num_castable_formats);
      if (format_cast_list != nullptr && !util_format_is_compressed(templ->format) &&
          screen->support_shader_images && templ->nr_samples <= 1) {
         desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      }
   } else {
      /* The VA frontend VaFourccToPipeFormat chooses _UNORM types for RGBx formats as typeless formats
       * such as DXGI_R8G8B8A8_TYPELESS are not supported as Video Processor input/output as specified in:
       * https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/hardware-support-for-direct3d-12-1-formats
       * PIPE_BIND_CUSTOM is used by the video frontend to hint this resource will be used in video and the
       * original format must be not converted to _TYPELESS
      */
      if (((templ->bind & PIPE_BIND_CUSTOM) == 0) &&
          (screen->support_shader_images && templ->nr_samples <= 1)) {
         /* Ideally, we'd key off of PIPE_BIND_SHADER_IMAGE for this, but it doesn't
          * seem to be set properly. So, all UAV-capable resources need the UAV flag.
          */
         D3D12_FEATURE_DATA_FORMAT_SUPPORT support = { desc.Format };
         if (SUCCEEDED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
             (support.Support2 & (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) ==
             (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) {
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            desc.Format = d3d12_get_typeless_format(templ->format);
         }
      }
   }

   if (templ->bind & (PIPE_BIND_SCANOUT | PIPE_BIND_LINEAR))
      desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

   HRESULT hres = E_FAIL;
   enum d3d12_residency_status init_residency;

   if (screen->opts12.RelaxedFormatCastingSupported) {
      D3D12_RESOURCE_DESC1 desc1 = {
         desc.Dimension,
         desc.Alignment,
         desc.Width,
         desc.Height,
         desc.DepthOrArraySize,
         desc.MipLevels,
         desc.Format,
         desc.SampleDesc,
         desc.Layout,
         desc.Flags,
      };
      if (heap) {
         init_residency = d3d12_permanently_resident;
         hres = screen->dev10->CreatePlacedResource2(heap,
                                                     placed_offset,
                                                     &desc1,
                                                     D3D12_BARRIER_LAYOUT_COMMON,
                                                     nullptr,
                                                     num_castable_formats,
                                                     format_cast_list,
                                                     IID_PPV_ARGS(&d3d12_res));
      }
      else {
         D3D12_HEAP_PROPERTIES heap_pris = GetCustomHeapProperties(screen->dev, D3D12_HEAP_TYPE_DEFAULT);

         D3D12_HEAP_FLAGS heap_flags = screen->support_create_not_resident ?
            D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT : D3D12_HEAP_FLAG_NONE;
         init_residency = screen->support_create_not_resident ? d3d12_evicted : d3d12_resident;

         hres = screen->dev10->CreateCommittedResource3(&heap_pris,
                                                        heap_flags,
                                                        &desc1,
                                                        D3D12_BARRIER_LAYOUT_COMMON,
                                                        nullptr,
                                                        nullptr,
                                                        num_castable_formats,
                                                        format_cast_list,
                                                        IID_PPV_ARGS(&d3d12_res));
      }
   } else {
      if (heap) {
         init_residency = d3d12_permanently_resident;
         hres = screen->dev->CreatePlacedResource(heap,
                                                  placed_offset,
                                                  &desc,
                                                  D3D12_RESOURCE_STATE_COMMON,
                                                  nullptr,
                                                  IID_PPV_ARGS(&d3d12_res));
      } else {
         D3D12_HEAP_PROPERTIES heap_pris = GetCustomHeapProperties(screen->dev, D3D12_HEAP_TYPE_DEFAULT);

         D3D12_HEAP_FLAGS heap_flags = screen->support_create_not_resident ?
            D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT : D3D12_HEAP_FLAG_NONE;
         init_residency = screen->support_create_not_resident ? d3d12_evicted : d3d12_resident;

         hres = screen->dev->CreateCommittedResource(&heap_pris,
                                                     heap_flags,
                                                     &desc,
                                                     D3D12_RESOURCE_STATE_COMMON,
                                                     NULL,
                                                     IID_PPV_ARGS(&d3d12_res));
      }
   }

   if (FAILED(hres))
      return false;

   if (screen->winsys && (templ->bind & PIPE_BIND_DISPLAY_TARGET)) {
      struct sw_winsys *winsys = screen->winsys;
      res->dt = winsys->displaytarget_create(screen->winsys,
                                             res->base.b.bind,
                                             res->base.b.format,
                                             templ->width0,
                                             templ->height0,
                                             64, NULL,
                                             &res->dt_stride);
      res->dt_refcount = 1;
   }

   res->bo = d3d12_bo_wrap_res(screen, d3d12_res, init_residency);

   return true;
}

static void
convert_planar_resource(struct d3d12_resource *res)
{
   unsigned num_planes = util_format_get_num_planes(res->base.b.format);
   if (num_planes <= 1 || res->base.b.next || !res->bo)
      return;

   struct pipe_resource *next = nullptr;
   struct pipe_resource *planes[3] = {
      &res->base.b, nullptr, nullptr
   };
   for (int plane = num_planes - 1; plane >= 0; --plane) {
      struct d3d12_resource *plane_res = d3d12_resource(planes[plane]);
      if (!plane_res) {
         plane_res = CALLOC_STRUCT(d3d12_resource);
         *plane_res = *res;
         plane_res->dt_refcount = num_planes;
         d3d12_bo_reference(plane_res->bo);
         pipe_reference_init(&plane_res->base.b.reference, 1);
         threaded_resource_init(&plane_res->base.b, false);
      }

      plane_res->base.b.next = next;
      next = &plane_res->base.b;

      plane_res->plane_slice = plane;
      plane_res->base.b.format = util_format_get_plane_format(res->base.b.format, plane);
      plane_res->base.b.width0 = util_format_get_plane_width(res->base.b.format, plane, res->base.b.width0);
      plane_res->base.b.height0 = util_format_get_plane_height(res->base.b.format, plane, res->base.b.height0);

#if DEBUG
      struct d3d12_screen *screen = d3d12_screen(res->base.b.screen);
      D3D12_RESOURCE_DESC desc = GetDesc(res->bo->res);
      desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed_footprint = {};
      D3D12_SUBRESOURCE_FOOTPRINT *footprint = &placed_footprint.Footprint;
      unsigned subresource = plane * desc.MipLevels * desc.DepthOrArraySize;
      screen->dev->GetCopyableFootprints(&desc, subresource, 1, 0, &placed_footprint, nullptr, nullptr, nullptr);
      assert(plane_res->base.b.width0 == footprint->Width);
      assert(plane_res->base.b.height0 == footprint->Height);
      assert(plane_res->base.b.depth0 == footprint->Depth);
      assert(plane_res->first_plane == &res->base.b);
#endif
   }
}

static struct pipe_resource *
d3d12_resource_create_or_place(struct d3d12_screen *screen,
                               struct d3d12_resource *res,
                               const struct pipe_resource *templ,
                               ID3D12Heap *heap,
                               uint64_t placed_offset)
{
   bool ret;

   res->base.b = *templ;

   res->overall_format = templ->format;
   res->plane_slice = 0;
   res->first_plane = &res->base.b;

   if (D3D12_DEBUG_RESOURCE & d3d12_debug) {
      debug_printf("D3D12: Create %sresource %s@%d %dx%dx%d as:%d mip:%d\n",
                   templ->usage == PIPE_USAGE_STAGING ? "STAGING " :"",
                   util_format_name(templ->format), templ->nr_samples,
                   templ->width0, templ->height0, templ->depth0,
                   templ->array_size, templ->last_level);
   }

   pipe_reference_init(&res->base.b.reference, 1);
   res->base.b.screen = &screen->base;

   if (templ->target == PIPE_BUFFER && !heap) {
      ret = init_buffer(screen, res, templ);
   } else {
      ret = init_texture(screen, res, templ, heap, placed_offset);
   }

   if (!ret) {
      FREE(res);
      return NULL;
   }

   init_valid_range(res);
   threaded_resource_init(&res->base.b,
      templ->usage == PIPE_USAGE_DEFAULT &&
      templ->target == PIPE_BUFFER);

   memset(&res->bind_counts, 0, sizeof(d3d12_resource::bind_counts));

   convert_planar_resource(res);

   return &res->base.b;
}

static struct pipe_resource *
d3d12_resource_create(struct pipe_screen *pscreen,
                      const struct pipe_resource *templ)
{
   struct d3d12_resource *res = CALLOC_STRUCT(d3d12_resource);
   if (!res)
      return NULL;

   return d3d12_resource_create_or_place(d3d12_screen(pscreen), res, templ, nullptr, 0);
}

static struct pipe_resource *
d3d12_resource_from_handle(struct pipe_screen *pscreen,
                          const struct pipe_resource *templ,
                          struct winsys_handle *handle, unsigned usage)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   if (handle->type != WINSYS_HANDLE_TYPE_D3D12_RES &&
       handle->type != WINSYS_HANDLE_TYPE_FD &&
       handle->type != WINSYS_HANDLE_TYPE_WIN32_NAME)
      return NULL;

   struct d3d12_resource *res = CALLOC_STRUCT(d3d12_resource);
   if (!res)
      return NULL;

   if (templ && templ->next) {
      struct d3d12_resource* next = d3d12_resource(templ->next);
      if (next->bo) {
         res->base.b = *templ;
         res->bo = next->bo;
         d3d12_bo_reference(res->bo);
      }
   }

#ifdef _WIN32
   HANDLE d3d_handle = handle->handle;
#else
   HANDLE d3d_handle = (HANDLE) (intptr_t) handle->handle;
#endif

#ifndef _GAMING_XBOX
   if (handle->type == WINSYS_HANDLE_TYPE_D3D12_RES) {
      ComPtr<IUnknown> screen_device;
      ComPtr<IUnknown> res_device;
      screen->dev->QueryInterface(screen_device.GetAddressOf());
      ((ID3D12DeviceChild *)handle->com_obj)->GetDevice(IID_PPV_ARGS(res_device.GetAddressOf()));

      if (screen_device.Get() != res_device.Get()) {
         debug_printf("d3d12: Importing resource - Resource's parent device (%p) does not"
                      " match d3d12 device (%p) instance from this pipe_screen."
                      " Attempting to re-import via NT Handle...\n", screen_device.Get(), res_device.Get());

         handle->type = WINSYS_HANDLE_TYPE_FD;
         HRESULT hr = screen->dev->CreateSharedHandle(((ID3D12DeviceChild *)handle->com_obj),
               nullptr,
               GENERIC_ALL,
               nullptr,
               &d3d_handle);

         if (FAILED(hr)) {
            debug_printf("d3d12: Error %x - Couldn't export incoming resource com_obj "
                         "(%p) via shared NT handle.\n", hr, handle->com_obj);
            return NULL;
         }
      }
   }
#endif

#ifdef _WIN32
   HANDLE d3d_handle_to_close = nullptr;
   if (handle->type == WINSYS_HANDLE_TYPE_WIN32_NAME) {
      screen->dev->OpenSharedHandleByName((LPCWSTR)handle->name, GENERIC_ALL, &d3d_handle_to_close);
      d3d_handle = d3d_handle_to_close;
   }
#endif

   ID3D12Resource *d3d12_res = nullptr;
   ID3D12Heap *d3d12_heap = nullptr;
   if (res->bo) {
      d3d12_res = res->bo->res;
   } else if (handle->type == WINSYS_HANDLE_TYPE_D3D12_RES) {
      if (handle->modifier == 1) {
         d3d12_heap = (ID3D12Heap *) handle->com_obj;
      } else {
         d3d12_res = (ID3D12Resource *) handle->com_obj;
      }
   } else {
      screen->dev->OpenSharedHandle(d3d_handle, IID_PPV_ARGS(&d3d12_res));
   }

#ifdef _WIN32
   if (d3d_handle_to_close) {
      CloseHandle(d3d_handle_to_close);
   }
#endif

   D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed_footprint = {};
   D3D12_SUBRESOURCE_FOOTPRINT *footprint = &placed_footprint.Footprint;
   D3D12_RESOURCE_DESC incoming_res_desc;

   if (!d3d12_res && !d3d12_heap)
      goto invalid;

   if (d3d12_heap) {
      assert(templ);
      assert(!res->bo);
      assert(!d3d12_res);
      return d3d12_resource_create_or_place(screen, res, templ, d3d12_heap, handle->offset);
   }

   pipe_reference_init(&res->base.b.reference, 1);
   res->base.b.screen = pscreen;
   incoming_res_desc = GetDesc(d3d12_res);

   /* Get a description for this plane */
   if (templ && handle->format != templ->format) {
      unsigned subresource = handle->plane * incoming_res_desc.MipLevels * incoming_res_desc.DepthOrArraySize;
      auto temp_desc = incoming_res_desc;
      temp_desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      screen->dev->GetCopyableFootprints(&temp_desc, subresource, 1, 0, &placed_footprint, nullptr, nullptr, nullptr);
   } else {
      footprint->Format = incoming_res_desc.Format;
      footprint->Width = incoming_res_desc.Width;
      footprint->Height = incoming_res_desc.Height;
      footprint->Depth = incoming_res_desc.DepthOrArraySize;
   }

   if (footprint->Width > UINT32_MAX ||
       footprint->Height > UINT16_MAX) {
      debug_printf("d3d12: Importing resource too large\n");
      goto invalid;
   }
   res->base.b.width0 = incoming_res_desc.Width;
   res->base.b.height0 = incoming_res_desc.Height;
   res->base.b.depth0 = 1;
   res->base.b.array_size = 1;

   switch (incoming_res_desc.Dimension) {
   case D3D12_RESOURCE_DIMENSION_BUFFER:
      res->base.b.target = PIPE_BUFFER;
      res->base.b.bind = PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_CONSTANT_BUFFER |
         PIPE_BIND_INDEX_BUFFER | PIPE_BIND_STREAM_OUTPUT | PIPE_BIND_SHADER_BUFFER |
         PIPE_BIND_COMMAND_ARGS_BUFFER | PIPE_BIND_QUERY_BUFFER;
      break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
      res->base.b.target = incoming_res_desc.DepthOrArraySize > 1 ?
         PIPE_TEXTURE_1D_ARRAY : PIPE_TEXTURE_1D;
      res->base.b.array_size = incoming_res_desc.DepthOrArraySize;
      break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
      res->base.b.target = incoming_res_desc.DepthOrArraySize > 1 ?
         PIPE_TEXTURE_2D_ARRAY : PIPE_TEXTURE_2D;
      res->base.b.array_size = incoming_res_desc.DepthOrArraySize;
      break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
      res->base.b.target = PIPE_TEXTURE_3D;
      res->base.b.depth0 = footprint->Depth;
      break;
   default:
      unreachable("Invalid dimension");
      break;
   }
   res->base.b.nr_samples = incoming_res_desc.SampleDesc.Count;
   res->base.b.last_level = incoming_res_desc.MipLevels - 1;
   res->base.b.usage = PIPE_USAGE_DEFAULT;
   res->base.b.bind |= PIPE_BIND_SHARED;
   if (incoming_res_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
      res->base.b.bind |= PIPE_BIND_RENDER_TARGET | PIPE_BIND_BLENDABLE | PIPE_BIND_DISPLAY_TARGET;
   if (incoming_res_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
      res->base.b.bind |= PIPE_BIND_DEPTH_STENCIL;
   if (incoming_res_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
      res->base.b.bind |= PIPE_BIND_SHADER_IMAGE;
   if ((incoming_res_desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == D3D12_RESOURCE_FLAG_NONE)
      res->base.b.bind |= PIPE_BIND_SAMPLER_VIEW;

   if (templ) {
      if (res->base.b.target == PIPE_TEXTURE_2D_ARRAY &&
            (templ->target == PIPE_TEXTURE_CUBE ||
             templ->target == PIPE_TEXTURE_CUBE_ARRAY)) {
         if (res->base.b.array_size < 6) {
            debug_printf("d3d12: Importing cube resource with too few array layers\n");
            goto invalid;
         }
         res->base.b.target = templ->target;
         res->base.b.array_size /= 6;
      }
      unsigned templ_samples = MAX2(templ->nr_samples, 1);
      if (res->base.b.target != templ->target ||
          footprint->Width != templ->width0 ||
          footprint->Height != templ->height0 ||
          footprint->Depth != templ->depth0 ||
          res->base.b.array_size != templ->array_size ||
          incoming_res_desc.SampleDesc.Count != templ_samples ||
          res->base.b.last_level != templ->last_level) {
         debug_printf("d3d12: Importing resource with mismatched dimensions: "
            "plane: %d, target: %d vs %d, width: %d vs %d, height: %d vs %d, "
            "depth: %d vs %d, array_size: %d vs %d, samples: %d vs %d, mips: %d vs %d\n",
            handle->plane,
            res->base.b.target, templ->target,
            footprint->Width, templ->width0,
            footprint->Height, templ->height0,
            footprint->Depth, templ->depth0,
            res->base.b.array_size, templ->array_size,
            incoming_res_desc.SampleDesc.Count, templ_samples,
            res->base.b.last_level + 1, templ->last_level + 1);
         goto invalid;
      }
      if (templ->target != PIPE_BUFFER) {
         if ((footprint->Format != d3d12_get_format(templ->format) &&
              footprint->Format != d3d12_get_typeless_format(templ->format)) ||
             (incoming_res_desc.Format != d3d12_get_format((enum pipe_format)handle->format) &&
              incoming_res_desc.Format != d3d12_get_typeless_format((enum pipe_format)handle->format))) {
            debug_printf("d3d12: Importing resource with mismatched format: "
               "plane could be DXGI format %d or %d, but is %d, "
               "overall could be DXGI format %d or %d, but is %d\n",
               d3d12_get_format(templ->format),
               d3d12_get_typeless_format(templ->format),
               footprint->Format,
               d3d12_get_format((enum pipe_format)handle->format),
               d3d12_get_typeless_format((enum pipe_format)handle->format),
               incoming_res_desc.Format);
            goto invalid;
         }
      }
      /* In an ideal world we'd be able to validate this, but gallium's use of bind
       * flags during resource creation is pretty bad: some bind flags are always set
       * (like PIPE_BIND_RENDER_TARGET) while others are never set (PIPE_BIND_SHADER_BUFFER)
       * 
      if (templ->bind & ~res->base.b.bind) {
         debug_printf("d3d12: Imported resource doesn't have necessary bind flags\n");
         goto invalid;
      } */

      res->base.b.format = templ->format;
      res->overall_format = (enum pipe_format)handle->format;
   } else {
      /* Search the pipe format lookup table for an entry */
      res->base.b.format = d3d12_get_pipe_format(incoming_res_desc.Format);

      if (res->base.b.format == PIPE_FORMAT_NONE) {
         /* Convert from typeless to a reasonable default */
         if (incoming_res_desc.Format == DXGI_FORMAT_UNKNOWN)
            res->base.b.format = PIPE_FORMAT_R8_UNORM;
         else
            res->base.b.format = d3d12_get_default_pipe_format(incoming_res_desc.Format);

         if (res->base.b.format == PIPE_FORMAT_NONE) {
            debug_printf("d3d12: Unable to deduce non-typeless resource format %d\n", incoming_res_desc.Format);
            goto invalid;
         }
      }

      res->overall_format = res->base.b.format;
   }

   if (!templ)
      handle->format = res->overall_format;

   res->dxgi_format = d3d12_get_format(res->overall_format);
   res->plane_slice = handle->plane;
   res->first_plane = &res->base.b;

   if (!res->bo) {
      res->bo = d3d12_bo_wrap_res(screen, d3d12_res, d3d12_permanently_resident);
   }
   init_valid_range(res);

   threaded_resource_init(&res->base.b, false);
   convert_planar_resource(res);

   return &res->base.b;

invalid:
   if (res->bo)
      d3d12_bo_unreference(res->bo);
   else if (d3d12_res)
      d3d12_res->Release();
   FREE(res);
   return NULL;
}

static bool
d3d12_resource_get_handle(struct pipe_screen *pscreen,
                          struct pipe_context *pcontext,
                          struct pipe_resource *pres,
                          struct winsys_handle *handle,
                          unsigned usage)
{
   struct d3d12_resource *res = d3d12_resource(pres);
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   switch (handle->type) {
   case WINSYS_HANDLE_TYPE_D3D12_RES:
      handle->com_obj = d3d12_resource_resource(res);
      return true;
   case WINSYS_HANDLE_TYPE_FD: {
      HANDLE d3d_handle = nullptr;

      screen->dev->CreateSharedHandle(d3d12_resource_resource(res),
                                      nullptr,
                                      GENERIC_ALL,
                                      nullptr,
                                      &d3d_handle);
      if (!d3d_handle)
         return false;
      
#ifdef _WIN32
      handle->handle = d3d_handle;
#else
      handle->handle = (int)(intptr_t)d3d_handle;
#endif
      handle->format = pres->format;
      handle->modifier = ~0ull;
      return true;
   }
   default:
      return false;
   }
}

struct pipe_resource *
d3d12_resource_from_resource(struct pipe_screen *pscreen,
                              ID3D12Resource* input_res)
{
    D3D12_RESOURCE_DESC input_desc = GetDesc(input_res);
    struct winsys_handle handle;
    memset(&handle, 0, sizeof(handle));
    handle.type = WINSYS_HANDLE_TYPE_D3D12_RES;
    handle.format = d3d12_get_pipe_format(input_desc.Format);
    handle.com_obj = input_res;
    input_res->AddRef();

    struct pipe_resource templ;
    memset(&templ, 0, sizeof(templ));
    if(input_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
       templ.target = PIPE_BUFFER;
    } else {
      templ.target = (input_desc.DepthOrArraySize > 1) ? PIPE_TEXTURE_2D_ARRAY : PIPE_TEXTURE_2D;
    }
    
    templ.format = d3d12_get_pipe_format(input_desc.Format);
    templ.width0 = input_desc.Width;
    templ.height0 = input_desc.Height;
    templ.depth0 = input_desc.DepthOrArraySize;
    templ.array_size = input_desc.DepthOrArraySize;
    templ.flags = 0;

    return d3d12_resource_from_handle(
        pscreen,
        &templ,
        &handle,
        PIPE_USAGE_DEFAULT
    );
}

/**
 * On Map/Unmap operations, we readback or flush all the underlying planes
 * of planar resources. The map/unmap operation from the caller is 
 * expected to be done for res->plane_slice plane only, but some
 * callers expect adjacent allocations for next contiguous plane access
 * 
 * In this function, we take the res and box the caller passed, and the plane_* properties
 * that are currently being readback/flushed, and adjust the d3d12_transfer ptrans
 * accordingly for the GPU copy operation between planes.
 */
static void d3d12_adjust_transfer_dimensions_for_plane(const struct d3d12_resource *res,
                                                       unsigned plane_slice,
                                                       unsigned plane_stride,
                                                       unsigned plane_layer_stride,
                                                       unsigned plane_offset,
                                                       const struct pipe_box* original_box,
                                                       struct pipe_transfer *ptrans/*inout*/)
{
   /* Adjust strides, offsets to the corresponding plane*/
   ptrans->stride = plane_stride;
   ptrans->layer_stride = plane_layer_stride;
   ptrans->offset = plane_offset;

   /* Find multipliers such that:*/
   /* first_plane.width = width_multiplier * planes[res->plane_slice].width*/
   /* first_plane.height = height_multiplier * planes[res->plane_slice].height*/
   float width_multiplier = res->first_plane->width0 / (float) util_format_get_plane_width(res->overall_format, res->plane_slice, res->first_plane->width0);
   float height_multiplier = res->first_plane->height0 / (float) util_format_get_plane_height(res->overall_format, res->plane_slice, res->first_plane->height0);
   
   /* Normalize box back to overall dimensions (first plane)*/
   ptrans->box.width = width_multiplier * original_box->width;
   ptrans->box.height = height_multiplier * original_box->height;
   ptrans->box.x = width_multiplier * original_box->x;
   ptrans->box.y = height_multiplier * original_box->y;

   /* Now adjust dimensions to plane_slice*/
   ptrans->box.width = util_format_get_plane_width(res->overall_format, plane_slice, ptrans->box.width);
   ptrans->box.height = util_format_get_plane_height(res->overall_format, plane_slice, ptrans->box.height);
   ptrans->box.x = util_format_get_plane_width(res->overall_format, plane_slice, ptrans->box.x);
   ptrans->box.y = util_format_get_plane_height(res->overall_format, plane_slice, ptrans->box.y);
}

static
void d3d12_resource_get_planes_info(pipe_resource *pres,
                                    unsigned num_planes,
                                    pipe_resource **planes,
                                    unsigned *strides,
                                    unsigned *layer_strides,
                                    unsigned *offsets,
                                    unsigned *staging_res_size)
{
   struct d3d12_resource* res = d3d12_resource(pres);
   *staging_res_size = 0;
   struct pipe_resource *cur_plane_resource = res->first_plane;
   for (uint plane_slice = 0; plane_slice < num_planes; ++plane_slice) {
      planes[plane_slice] = cur_plane_resource;
      int width = util_format_get_plane_width(res->base.b.format, plane_slice, res->first_plane->width0);
      int height = util_format_get_plane_height(res->base.b.format, plane_slice, res->first_plane->height0);

      strides[plane_slice] = align(util_format_get_stride(cur_plane_resource->format, width),
                           D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

      layer_strides[plane_slice] = align(util_format_get_2d_size(cur_plane_resource->format,
                                                   strides[plane_slice],
                                                   height),
                                 D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

      offsets[plane_slice] = *staging_res_size;
      *staging_res_size += layer_strides[plane_slice];
      cur_plane_resource = cur_plane_resource->next;
   }
}

static constexpr unsigned d3d12_max_planes = 3;

/**
 * Get stride and offset for the given pipe resource without the need to get
 * a winsys_handle.
 */
void
d3d12_resource_get_info(struct pipe_screen *pscreen,
                        struct pipe_resource *pres,
                        unsigned *stride,
                        unsigned *offset)
{

   struct d3d12_resource* res = d3d12_resource(pres);
   unsigned num_planes = util_format_get_num_planes(res->overall_format);

   pipe_resource *planes[d3d12_max_planes];
   unsigned int strides[d3d12_max_planes];
   unsigned int layer_strides[d3d12_max_planes];
   unsigned int offsets[d3d12_max_planes];
   unsigned staging_res_size = 0;
   d3d12_resource_get_planes_info(
      pres,
      num_planes,
      planes,
      strides,
      layer_strides,
      offsets,
      &staging_res_size
   );

   if(stride) {
      *stride = strides[res->plane_slice];
   }

   if(offset) {
      *offset = offsets[res->plane_slice];
   }
}

static struct pipe_memory_object *
d3d12_memobj_create_from_handle(struct pipe_screen *pscreen, struct winsys_handle *handle, bool dedicated)
{
   if (handle->type != WINSYS_HANDLE_TYPE_WIN32_HANDLE &&
       handle->type != WINSYS_HANDLE_TYPE_WIN32_NAME) {
      debug_printf("d3d12: Unsupported memobj handle type\n");
      return NULL;
   }

   struct d3d12_screen *screen = d3d12_screen(pscreen);
#ifdef _GAMING_XBOX
   IGraphicsUnknown
#else
   IUnknown
#endif
      *obj;
#ifdef _WIN32
      HANDLE d3d_handle = handle->handle;
#else
      HANDLE d3d_handle = (HANDLE)(intptr_t)handle->handle;
#endif

#ifdef _WIN32
      HANDLE d3d_handle_to_close = nullptr;
      if (handle->type == WINSYS_HANDLE_TYPE_WIN32_NAME) {
         screen->dev->OpenSharedHandleByName((LPCWSTR) handle->name, GENERIC_ALL, &d3d_handle_to_close);
         d3d_handle = d3d_handle_to_close;
      }
#endif

   screen->dev->OpenSharedHandle(d3d_handle, IID_PPV_ARGS(&obj));

#ifdef _WIN32
   if (d3d_handle_to_close) {
      CloseHandle(d3d_handle_to_close);
   }
#endif

   if (!obj) {
      debug_printf("d3d12: Failed to open memobj handle as anything\n");
      return NULL;
   }

   struct d3d12_memory_object *memobj = CALLOC_STRUCT(d3d12_memory_object);
   if (!memobj) {
      obj->Release();
      return NULL;
   }
   memobj->base.dedicated = dedicated;

   obj->AddRef();
   if (handle->modifier == 1) {
      memobj->heap = (ID3D12Heap *) obj;
   } else {
      memobj->res = (ID3D12Resource *) obj;
   }

   obj->Release();
   if (!memobj->res && !memobj->heap) {
      debug_printf("d3d12: Memory object isn't a resource or heap\n");
      free(memobj);
      return NULL;
   }

   bool expect_dedicated = memobj->res != nullptr;
   if (dedicated != expect_dedicated)
      debug_printf("d3d12: Expected dedicated to be %s for imported %s\n",
                   expect_dedicated ? "true" : "false",
                   expect_dedicated ? "resource" : "heap");

   return &memobj->base;
}

static void
d3d12_memobj_destroy(struct pipe_screen *pscreen, struct pipe_memory_object *pmemobj)
{
   struct d3d12_memory_object *memobj = d3d12_memory_object(pmemobj);
   if (memobj->res)
      memobj->res->Release();
   if (memobj->heap)
      memobj->heap->Release();
   free(memobj);
}

static pipe_resource *
d3d12_resource_from_memobj(struct pipe_screen *pscreen,
                           const struct pipe_resource *templ,
                           struct pipe_memory_object *pmemobj,
                           uint64_t offset)
{
   struct d3d12_memory_object *memobj = d3d12_memory_object(pmemobj);

   struct winsys_handle whandle = {};
   whandle.type = WINSYS_HANDLE_TYPE_D3D12_RES;
   whandle.com_obj = memobj->res ? (void *) memobj->res : (void *) memobj->heap;
   whandle.offset = offset;
   whandle.format = templ->format;
   whandle.modifier = memobj->res ? 0 : 1;

   // WINSYS_HANDLE_TYPE_D3D12_RES implies taking ownership of the reference
   ((IUnknown *)whandle.com_obj)->AddRef();
   return d3d12_resource_from_handle(pscreen, templ, &whandle, 0);
}

void
d3d12_screen_resource_init(struct pipe_screen *pscreen)
{
   pscreen->resource_create = d3d12_resource_create;
   pscreen->resource_from_handle = d3d12_resource_from_handle;
   pscreen->resource_get_handle = d3d12_resource_get_handle;
   pscreen->resource_destroy = d3d12_resource_destroy;
   pscreen->resource_get_info = d3d12_resource_get_info;

   pscreen->memobj_create_from_handle = d3d12_memobj_create_from_handle;
   pscreen->memobj_destroy = d3d12_memobj_destroy;
   pscreen->resource_from_memobj = d3d12_resource_from_memobj;
}

unsigned int
get_subresource_id(struct d3d12_resource *res, unsigned resid,
                   unsigned z, unsigned base_level)
{
   unsigned resource_stride = (res->base.b.last_level + 1) * res->base.b.array_size;
   unsigned layer_stride = res->base.b.last_level + 1;

   return resid * resource_stride + z * layer_stride +
         base_level + res->plane_slice * resource_stride;
}

static D3D12_TEXTURE_COPY_LOCATION
fill_texture_location(struct d3d12_resource *res,
                      struct d3d12_transfer *trans, unsigned resid, unsigned z)
{
   D3D12_TEXTURE_COPY_LOCATION tex_loc = {0};
   int subres = get_subresource_id(res, resid, z, trans->base.b.level);

   tex_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
   tex_loc.SubresourceIndex = subres;
   tex_loc.pResource = d3d12_resource_resource(res);
   return tex_loc;
}

static D3D12_TEXTURE_COPY_LOCATION
fill_buffer_location(struct d3d12_context *ctx,
                     struct d3d12_resource *res,
                     struct d3d12_resource *staging_res,
                     struct d3d12_transfer *trans,
                     unsigned depth,
                     unsigned resid, unsigned z)
{
   D3D12_TEXTURE_COPY_LOCATION buf_loc = {0};
   D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
   uint64_t offset = 0;
   auto descr = GetDesc(d3d12_resource_underlying(res, &offset));
   descr.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   ID3D12Device* dev = screen->dev;

   unsigned sub_resid = get_subresource_id(res, resid, z, trans->base.b.level);
   dev->GetCopyableFootprints(&descr, sub_resid, 1, 0, &footprint, nullptr, nullptr, nullptr);

   buf_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
   buf_loc.pResource = d3d12_resource_underlying(staging_res, &offset);
   buf_loc.PlacedFootprint = footprint;
   buf_loc.PlacedFootprint.Offset = offset;
   buf_loc.PlacedFootprint.Offset += trans->base.b.offset;

   if (util_format_has_depth(util_format_description(res->base.b.format)) &&
       screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
      buf_loc.PlacedFootprint.Footprint.Width = res->base.b.width0;
      buf_loc.PlacedFootprint.Footprint.Height = res->base.b.height0;
      buf_loc.PlacedFootprint.Footprint.Depth = res->base.b.depth0;
   } else {
      buf_loc.PlacedFootprint.Footprint.Width = ALIGN(trans->base.b.box.width,
                                                      util_format_get_blockwidth(res->base.b.format));
      buf_loc.PlacedFootprint.Footprint.Height = ALIGN(trans->base.b.box.height,
                                                       util_format_get_blockheight(res->base.b.format));
      buf_loc.PlacedFootprint.Footprint.Depth = ALIGN(depth,
                                                      util_format_get_blockdepth(res->base.b.format));
   }

   buf_loc.PlacedFootprint.Footprint.RowPitch = trans->base.b.stride;

   return buf_loc;
}

struct copy_info {
   struct d3d12_resource *dst;
   D3D12_TEXTURE_COPY_LOCATION dst_loc;
   UINT dst_x, dst_y, dst_z;
   struct d3d12_resource *src;
   D3D12_TEXTURE_COPY_LOCATION src_loc;
   D3D12_BOX *src_box;
};


static void
copy_texture_region(struct d3d12_context *ctx,
                    struct copy_info& info)
{
   auto batch = d3d12_current_batch(ctx);

   d3d12_batch_reference_resource(batch, info.src, false);
   d3d12_batch_reference_resource(batch, info.dst, true);
   d3d12_transition_resource_state(ctx, info.src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_transition_resource_state(ctx, info.dst, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_apply_resource_states(ctx, false);
   ctx->cmdlist->CopyTextureRegion(&info.dst_loc, info.dst_x, info.dst_y, info.dst_z,
                                   &info.src_loc, info.src_box);
}

static void
transfer_buf_to_image_part(struct d3d12_context *ctx,
                           struct d3d12_resource *res,
                           struct d3d12_resource *staging_res,
                           struct d3d12_transfer *trans,
                           int z, int depth, int start_z, int dest_z,
                           int resid)
{
   if (D3D12_DEBUG_RESOURCE & d3d12_debug) {
      debug_printf("D3D12: Copy %dx%dx%d + %dx%dx%d from buffer %s to image %s\n",
                   trans->base.b.box.x, trans->base.b.box.y, trans->base.b.box.z,
                   trans->base.b.box.width, trans->base.b.box.height, trans->base.b.box.depth,
                   util_format_name(staging_res->base.b.format),
                   util_format_name(res->base.b.format));
   }

   struct d3d12_screen *screen = d3d12_screen(res->base.b.screen);
   struct copy_info copy_info;
   copy_info.src = staging_res;
   copy_info.src_loc = fill_buffer_location(ctx, res, staging_res, trans, depth, resid, z);
   copy_info.src_loc.PlacedFootprint.Offset += (z  - start_z) * trans->base.b.layer_stride;
   copy_info.src_box = nullptr;
   copy_info.dst = res;
   copy_info.dst_loc = fill_texture_location(res, trans, resid, z);
   if (util_format_has_depth(util_format_description(res->base.b.format)) &&
       screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
      copy_info.dst_x = 0;
      copy_info.dst_y = 0;
   } else {
      copy_info.dst_x = trans->base.b.box.x;
      copy_info.dst_y = trans->base.b.box.y;
   }
   copy_info.dst_z = res->base.b.target == PIPE_TEXTURE_CUBE ? 0 : dest_z;
   copy_info.src_box = nullptr;

   copy_texture_region(ctx, copy_info);
}

static bool
transfer_buf_to_image(struct d3d12_context *ctx,
                      struct d3d12_resource *res,
                      struct d3d12_resource *staging_res,
                      struct d3d12_transfer *trans, int resid)
{
   if (res->base.b.target == PIPE_TEXTURE_3D) {
      assert(resid == 0);
      transfer_buf_to_image_part(ctx, res, staging_res, trans,
                                 0, trans->base.b.box.depth, 0,
                                 trans->base.b.box.z, 0);
   } else {
      int num_layers = trans->base.b.box.depth;
      int start_z = trans->base.b.box.z;

      for (int z = start_z; z < start_z + num_layers; ++z) {
         transfer_buf_to_image_part(ctx, res, staging_res, trans,
                                           z, 1, start_z, 0, resid);
      }
   }
   return true;
}

static void
transfer_image_part_to_buf(struct d3d12_context *ctx,
                           struct d3d12_resource *res,
                           struct d3d12_resource *staging_res,
                           struct d3d12_transfer *trans,
                           unsigned resid, int z, int start_layer,
                           int start_box_z, int depth)
{
   struct pipe_box *box = &trans->base.b.box;
   D3D12_BOX src_box = {};

   struct d3d12_screen *screen = d3d12_screen(res->base.b.screen);
   struct copy_info copy_info;
   copy_info.src_box = nullptr;
   copy_info.src = res;
   copy_info.src_loc = fill_texture_location(res, trans, resid, z);
   copy_info.dst = staging_res;
   copy_info.dst_loc = fill_buffer_location(ctx, res, staging_res, trans,
                                            depth, resid, z);
   copy_info.dst_loc.PlacedFootprint.Offset += (z  - start_layer) * trans->base.b.layer_stride;
   copy_info.dst_x = copy_info.dst_y = copy_info.dst_z = 0;

   bool whole_resource = util_texrange_covers_whole_level(&res->base.b, trans->base.b.level,
                                                          box->x, box->y, start_box_z,
                                                          box->width, box->height, depth);
   if (util_format_has_depth(util_format_description(res->base.b.format)) &&
       screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED)
      whole_resource = true;
   if (!whole_resource) {
      src_box.left = box->x;
      src_box.right = box->x + box->width;
      src_box.top = box->y;
      src_box.bottom = box->y + box->height;
      src_box.front = start_box_z;
      src_box.back = start_box_z + depth;
      copy_info.src_box = &src_box;
   }

   copy_texture_region(ctx, copy_info);
}

static bool
transfer_image_to_buf(struct d3d12_context *ctx,
                            struct d3d12_resource *res,
                            struct d3d12_resource *staging_res,
                            struct d3d12_transfer *trans,
                            unsigned resid)
{

   /* We only suppport loading from either an texture array
    * or a ZS texture, so either resid is zero, or num_layers == 1)
    */
   assert(resid == 0 || trans->base.b.box.depth == 1);

   if (D3D12_DEBUG_RESOURCE & d3d12_debug) {
      debug_printf("D3D12: Copy %dx%dx%d + %dx%dx%d from %s@%d to %s\n",
                   trans->base.b.box.x, trans->base.b.box.y, trans->base.b.box.z,
                   trans->base.b.box.width, trans->base.b.box.height, trans->base.b.box.depth,
                   util_format_name(res->base.b.format), resid,
                   util_format_name(staging_res->base.b.format));
   }

   struct pipe_resource *resolved_resource = nullptr;
   if (res->base.b.nr_samples > 1) {
      struct pipe_resource tmpl = res->base.b;
      tmpl.nr_samples = 0;
      resolved_resource = d3d12_resource_create(ctx->base.screen, &tmpl);
      struct pipe_blit_info resolve_info = {};
      struct pipe_box box = {0,0,0, (int)res->base.b.width0, (int16_t)res->base.b.height0, (int16_t)res->base.b.depth0};
      resolve_info.dst.resource = resolved_resource;
      resolve_info.dst.box = box;
      resolve_info.dst.format = res->base.b.format;
      resolve_info.src.resource = &res->base.b;
      resolve_info.src.box = box;
      resolve_info.src.format = res->base.b.format;
      resolve_info.filter = PIPE_TEX_FILTER_NEAREST;
      resolve_info.mask = util_format_get_mask(tmpl.format);



      d3d12_blit(&ctx->base, &resolve_info);
      res = (struct d3d12_resource *)resolved_resource;
   }


   if (res->base.b.target == PIPE_TEXTURE_3D) {
      transfer_image_part_to_buf(ctx, res, staging_res, trans, resid,
                                 0, 0, trans->base.b.box.z, trans->base.b.box.depth);
   } else {
      int start_layer = trans->base.b.box.z;
      for (int z = start_layer; z < start_layer + trans->base.b.box.depth; ++z) {
         transfer_image_part_to_buf(ctx, res, staging_res, trans, resid,
                                    z, start_layer, 0, 1);
      }
   }

   pipe_resource_reference(&resolved_resource, NULL);

   return true;
}

static void
transfer_buf_to_buf(struct d3d12_context *ctx,
                    struct d3d12_resource *src,
                    struct d3d12_resource *dst,
                    uint64_t src_offset,
                    uint64_t dst_offset,
                    uint64_t width)
{
   auto batch = d3d12_current_batch(ctx);

   d3d12_batch_reference_resource(batch, src, false);
   d3d12_batch_reference_resource(batch, dst, true);

   uint64_t src_offset_suballoc = 0;
   uint64_t dst_offset_suballoc = 0;
   auto src_d3d12 = d3d12_resource_underlying(src, &src_offset_suballoc);
   auto dst_d3d12 = d3d12_resource_underlying(dst, &dst_offset_suballoc);
   src_offset += src_offset_suballoc;
   dst_offset += dst_offset_suballoc;

   // Same-resource copies not supported, since the resource would need to be in both states
   assert(src_d3d12 != dst_d3d12);
   d3d12_transition_resource_state(ctx, src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_transition_resource_state(ctx, dst, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_apply_resource_states(ctx, false);
   ctx->cmdlist->CopyBufferRegion(dst_d3d12, dst_offset,
                                  src_d3d12, src_offset,
                                  width);
}

static unsigned
linear_offset(int x, int y, int z, unsigned stride, unsigned layer_stride)
{
   return x +
          y * stride +
          z * layer_stride;
}

static D3D12_RANGE
linear_range(const struct pipe_box *box, unsigned stride, unsigned layer_stride)
{
   D3D12_RANGE range;

   range.Begin = linear_offset(box->x, box->y, box->z,
                               stride, layer_stride);
   range.End = linear_offset(box->x + box->width,
                             box->y + box->height - 1,
                             box->z + box->depth - 1,
                             stride, layer_stride);

   return range;
}

static bool
synchronize(struct d3d12_context *ctx,
            struct d3d12_resource *res,
            unsigned usage,
            D3D12_RANGE *range)
{
   assert(can_map_directly(&res->base.b));

   /* Check whether that range contains valid data; if not, we might not need to sync */
   if (!(usage & PIPE_MAP_UNSYNCHRONIZED) &&
       usage & PIPE_MAP_WRITE &&
       !util_ranges_intersect(&res->valid_buffer_range, range->Begin, range->End)) {
      usage |= PIPE_MAP_UNSYNCHRONIZED;
   }

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED) && resource_is_busy(ctx, res, usage & PIPE_MAP_WRITE)) {
      if (usage & PIPE_MAP_DONTBLOCK) {
         if (d3d12_batch_has_references(d3d12_current_batch(ctx), res->bo, usage & PIPE_MAP_WRITE))
            d3d12_flush_cmdlist(ctx);
         return false;
      }

      d3d12_resource_wait_idle(ctx, res, usage & PIPE_MAP_WRITE);
   }

   if (usage & PIPE_MAP_WRITE)
      util_range_add(&res->base.b, &res->valid_buffer_range,
                     range->Begin, range->End);

   return true;
}

/* A wrapper to make sure local resources are freed and unmapped with
 * any exit path */
struct local_resource {
   local_resource(pipe_screen *s, struct pipe_resource *tmpl) :
      mapped(false)
   {
      res = d3d12_resource(d3d12_resource_create(s, tmpl));
   }

   ~local_resource() {
      if (res) {
         if (mapped)
            d3d12_bo_unmap(res->bo, nullptr);
         pipe_resource_reference((struct pipe_resource **)&res, NULL);
      }
   }

   void *
   map() {
      void *ptr;
      ptr = d3d12_bo_map(res->bo, nullptr);
      if (ptr)
         mapped = true;
      return ptr;
   }

   void unmap()
   {
      if (mapped)
         d3d12_bo_unmap(res->bo, nullptr);
      mapped = false;
   }

   operator struct d3d12_resource *() {
      return res;
   }

   bool operator !() {
      return !res;
   }
private:
   struct d3d12_resource *res;
   bool mapped;
};

/* Combined depth-stencil needs a special handling for reading back: DX handled
 * depth and stencil parts as separate resources and handles copying them only
 * by using seperate texture copy calls with different formats. So create two
 * buffers, read back both resources and interleave the data.
 */
static void
prepare_zs_layer_strides(struct d3d12_screen *screen,
                         struct d3d12_resource *res,
                         const struct pipe_box *box,
                         struct d3d12_transfer *trans)
{
   bool copy_whole_resource = screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED;
   int width = copy_whole_resource ? res->base.b.width0 : box->width;
   int height = copy_whole_resource ? res->base.b.height0 : box->height;

   trans->base.b.stride = align(util_format_get_stride(res->base.b.format, width),
                                D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
   trans->base.b.layer_stride = util_format_get_2d_size(res->base.b.format,
                                                        trans->base.b.stride,
                                                        height);

   if (copy_whole_resource) {
      trans->zs_cpu_copy_stride = align(util_format_get_stride(res->base.b.format, box->width),
                                        D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
      trans->zs_cpu_copy_layer_stride = util_format_get_2d_size(res->base.b.format,
                                                                trans->base.b.stride,
                                                                box->height);
   } else {
      trans->zs_cpu_copy_stride = trans->base.b.stride;
      trans->zs_cpu_copy_layer_stride = trans->base.b.layer_stride;
   }
}

static void *
read_zs_surface(struct d3d12_context *ctx, struct d3d12_resource *res,
                const struct pipe_box *box,
                struct d3d12_transfer *trans)
{
   pipe_screen *pscreen = ctx->base.screen;
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   prepare_zs_layer_strides(screen, res, box, trans);

   struct pipe_resource tmpl;
   memset(&tmpl, 0, sizeof tmpl);
   tmpl.target = PIPE_BUFFER;
   tmpl.format = PIPE_FORMAT_R32_UNORM;
   tmpl.bind = 0;
   tmpl.usage = PIPE_USAGE_STAGING;
   tmpl.flags = 0;
   tmpl.width0 = trans->base.b.layer_stride;
   tmpl.height0 = 1;
   tmpl.depth0 = 1;
   tmpl.array_size = 1;

   local_resource depth_buffer(pscreen, &tmpl);
   if (!depth_buffer) {
      debug_printf("Allocating staging buffer for depth failed\n");
      return NULL;
   }

   if (!transfer_image_to_buf(ctx, res, depth_buffer, trans, 0))
      return NULL;

   tmpl.format = PIPE_FORMAT_R8_UINT;

   local_resource stencil_buffer(pscreen, &tmpl);
   if (!stencil_buffer) {
      debug_printf("Allocating staging buffer for stencilfailed\n");
      return NULL;
   }

   if (!transfer_image_to_buf(ctx, res, stencil_buffer, trans, 1))
      return NULL;

   d3d12_flush_cmdlist_and_wait(ctx);

   uint8_t *depth_ptr = (uint8_t *)depth_buffer.map();
   if (!depth_ptr) {
      debug_printf("Mapping staging depth buffer failed\n");
      return NULL;
   }

   uint8_t *stencil_ptr =  (uint8_t *)stencil_buffer.map();
   if (!stencil_ptr) {
      debug_printf("Mapping staging stencil buffer failed\n");
      return NULL;
   }

   uint8_t *buf = (uint8_t *)malloc(trans->zs_cpu_copy_layer_stride);
   if (!buf)
      return NULL;

   trans->data = buf;

   switch (res->base.b.format) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
         depth_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
         stencil_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
      }
      util_format_z24_unorm_s8_uint_pack_separate(buf, trans->zs_cpu_copy_stride,
                                                  (uint32_t *)depth_ptr, trans->base.b.stride,
                                                  stencil_ptr, trans->base.b.stride,
                                                  trans->base.b.box.width, trans->base.b.box.height);
      break;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
         depth_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
         stencil_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x;
      }
      util_format_z32_float_s8x24_uint_pack_z_float(buf, trans->zs_cpu_copy_stride,
                                                    (float *)depth_ptr, trans->base.b.stride,
                                                    trans->base.b.box.width, trans->base.b.box.height);
      util_format_z32_float_s8x24_uint_pack_s_8uint(buf, trans->zs_cpu_copy_stride,
                                                    stencil_ptr, trans->base.b.stride,
                                                    trans->base.b.box.width, trans->base.b.box.height);
      break;
   default:
      unreachable("Unsupported depth steancil format");
   };

   return trans->data;
}

static void *
prepare_write_zs_surface(struct d3d12_resource *res,
                         const struct pipe_box *box,
                         struct d3d12_transfer *trans)
{
   struct d3d12_screen *screen = d3d12_screen(res->base.b.screen);
   prepare_zs_layer_strides(screen, res, box, trans);
   uint32_t *buf = (uint32_t *)malloc(trans->base.b.layer_stride);
   if (!buf)
      return NULL;

   trans->data = buf;
   return trans->data;
}

static void
write_zs_surface(struct pipe_context *pctx, struct d3d12_resource *res,
                 struct d3d12_transfer *trans)
{
   struct d3d12_screen *screen = d3d12_screen(res->base.b.screen);
   struct pipe_resource tmpl;
   memset(&tmpl, 0, sizeof tmpl);
   tmpl.target = PIPE_BUFFER;
   tmpl.format = PIPE_FORMAT_R32_UNORM;
   tmpl.bind = 0;
   tmpl.usage = PIPE_USAGE_STAGING;
   tmpl.flags = 0;
   tmpl.width0 = trans->base.b.layer_stride;
   tmpl.height0 = 1;
   tmpl.depth0 = 1;
   tmpl.array_size = 1;

   local_resource depth_buffer(pctx->screen, &tmpl);
   if (!depth_buffer) {
      debug_printf("Allocating staging buffer for depth failed\n");
      return;
   }

   local_resource stencil_buffer(pctx->screen, &tmpl);
   if (!stencil_buffer) {
      debug_printf("Allocating staging buffer for depth failed\n");
      return;
   }

   uint8_t *depth_ptr = (uint8_t *)depth_buffer.map();
   if (!depth_ptr) {
      debug_printf("Mapping staging depth buffer failed\n");
      return;
   }

   uint8_t *stencil_ptr =  (uint8_t *)stencil_buffer.map();
   if (!stencil_ptr) {
      debug_printf("Mapping staging stencil buffer failed\n");
      return;
   }

   switch (res->base.b.format) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
         depth_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
         stencil_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
      }
      util_format_z32_unorm_unpack_z_32unorm((uint32_t *)depth_ptr, trans->base.b.stride, (uint8_t*)trans->data,
                                             trans->zs_cpu_copy_stride, trans->base.b.box.width,
                                             trans->base.b.box.height);
      util_format_z24_unorm_s8_uint_unpack_s_8uint(stencil_ptr, trans->base.b.stride, (uint8_t*)trans->data,
                                                   trans->zs_cpu_copy_stride, trans->base.b.box.width,
                                                   trans->base.b.box.height);
      break;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
         depth_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x * 4;
         stencil_ptr += trans->base.b.box.y * trans->base.b.stride + trans->base.b.box.x;
      }
      util_format_z32_float_s8x24_uint_unpack_z_float((float *)depth_ptr, trans->base.b.stride, (uint8_t*)trans->data,
                                                      trans->zs_cpu_copy_stride, trans->base.b.box.width,
                                                      trans->base.b.box.height);
      util_format_z32_float_s8x24_uint_unpack_s_8uint(stencil_ptr, trans->base.b.stride, (uint8_t*)trans->data,
                                                      trans->zs_cpu_copy_stride, trans->base.b.box.width,
                                                      trans->base.b.box.height);
      break;
   default:
      unreachable("Unsupported depth steancil format");
   };

   stencil_buffer.unmap();
   depth_buffer.unmap();

   transfer_buf_to_image(d3d12_context(pctx), res, depth_buffer, trans, 0);
   transfer_buf_to_image(d3d12_context(pctx), res, stencil_buffer, trans, 1);
}

#define BUFFER_MAP_ALIGNMENT 64

static void *
d3d12_transfer_map(struct pipe_context *pctx,
                   struct pipe_resource *pres,
                   unsigned level,
                   unsigned usage,
                   const struct pipe_box *box,
                   struct pipe_transfer **transfer)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_resource *res = d3d12_resource(pres);
   struct d3d12_screen *screen = d3d12_screen(pres->screen);

   if (usage & PIPE_MAP_DIRECTLY || !res->bo)
      return NULL;

   slab_child_pool* transfer_pool = (usage & TC_TRANSFER_MAP_THREADED_UNSYNC) ?
      &ctx->transfer_pool_unsync : &ctx->transfer_pool;
   struct d3d12_transfer *trans = (struct d3d12_transfer *)slab_zalloc(transfer_pool);
   struct pipe_transfer *ptrans = &trans->base.b;
   if (!trans)
      return NULL;

   ptrans->level = level;
   ptrans->usage = (enum pipe_map_flags)usage;
   ptrans->box = *box;

   D3D12_RANGE range;
   range.Begin = 0;

   void *ptr;
   if (can_map_directly(&res->base.b)) {
      if (pres->target == PIPE_BUFFER) {
         ptrans->stride = 0;
         ptrans->layer_stride = 0;
      } else {
         ptrans->stride = util_format_get_stride(pres->format, box->width);
         ptrans->layer_stride = util_format_get_2d_size(pres->format,
                                                        ptrans->stride,
                                                        box->height);
      }

      range = linear_range(box, ptrans->stride, ptrans->layer_stride);
      if (!synchronize(ctx, res, usage, &range)) {
         slab_free(transfer_pool, trans);
         return NULL;
      }
      ptr = d3d12_bo_map(res->bo, &range);
   } else if (unlikely(pres->format == PIPE_FORMAT_Z24_UNORM_S8_UINT ||
                       pres->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)) {
      if (usage & PIPE_MAP_READ) {
         ptr = read_zs_surface(ctx, res, box, trans);
      } else if (usage & PIPE_MAP_WRITE){
         ptr = prepare_write_zs_surface(res, box, trans);
      } else {
         ptr = nullptr;
      }
   } else if(util_format_is_yuv(res->overall_format)) {

      /* Get planes information*/

      unsigned num_planes = util_format_get_num_planes(res->overall_format);
      pipe_resource *planes[d3d12_max_planes];
      unsigned int strides[d3d12_max_planes];
      unsigned int layer_strides[d3d12_max_planes];
      unsigned int offsets[d3d12_max_planes];
      unsigned staging_res_size = 0;

      d3d12_resource_get_planes_info(
         pres,
         num_planes,
         planes,
         strides,
         layer_strides,
         offsets,
         &staging_res_size
      );
      
      /* Allocate a buffer for all the planes to fit in adjacent memory*/

      pipe_resource_usage staging_usage = (usage & (PIPE_MAP_READ | PIPE_MAP_READ_WRITE)) ?
         PIPE_USAGE_STAGING : PIPE_USAGE_STREAM;
      trans->staging_res = pipe_buffer_create(pctx->screen, 0,
                                              staging_usage,
                                              staging_res_size);
      if (!trans->staging_res)
         return NULL;

      struct d3d12_resource *staging_res = d3d12_resource(trans->staging_res);

      /* Readback contents into the buffer allocation now if map was intended for read*/

      /* Read all planes if readback needed*/
      if (usage & PIPE_MAP_READ) {
         pipe_box original_box = ptrans->box;
         for (uint plane_slice = 0; plane_slice < num_planes; ++plane_slice) {
            /* Adjust strides, offsets, box to the corresponding plane for the copytexture operation*/
            d3d12_adjust_transfer_dimensions_for_plane(res,
                                                       plane_slice,
                                                       strides[plane_slice],
                                                       layer_strides[plane_slice],
                                                       offsets[plane_slice],
                                                       &original_box,
                                                       ptrans/*inout*/);
            /* Perform the readback*/
            if(!transfer_image_to_buf(ctx, d3d12_resource(planes[plane_slice]), staging_res, trans, 0)){
               return NULL;
            }
         }
         ptrans->box = original_box;
         d3d12_flush_cmdlist_and_wait(ctx);
      }

      /* Map the whole staging buffer containing all the planes contiguously*/
      /* Just offset the resulting ptr to the according plane offset*/

      range.End = staging_res_size - range.Begin;
      uint8_t* all_planes_map = (uint8_t*) d3d12_bo_map(staging_res->bo, &range);

      ptrans->stride = strides[res->plane_slice];
      ptrans->layer_stride = layer_strides[res->plane_slice];
      ptr = all_planes_map + offsets[res->plane_slice];

   } else {
      ptrans->stride = align(util_format_get_stride(pres->format, box->width),
                              D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
      ptrans->layer_stride = util_format_get_2d_size(pres->format,
                                                     ptrans->stride,
                                                     box->height);

      if (res->base.b.target != PIPE_TEXTURE_3D)
         ptrans->layer_stride = align(ptrans->layer_stride,
                                      D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

      if (util_format_has_depth(util_format_description(pres->format)) &&
          screen->opts2.ProgrammableSamplePositionsTier == D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED) {
         trans->zs_cpu_copy_stride = ptrans->stride;
         trans->zs_cpu_copy_layer_stride = ptrans->layer_stride;
         
         ptrans->stride = align(util_format_get_stride(pres->format, pres->width0),
                                D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
         ptrans->layer_stride = util_format_get_2d_size(pres->format,
                                                        ptrans->stride,
                                                        pres->height0);

         range.Begin = box->y * ptrans->stride +
            box->x * util_format_get_blocksize(pres->format);
      }

      unsigned staging_res_size = ptrans->layer_stride * box->depth;
      if (res->base.b.target == PIPE_BUFFER) {
         /* To properly support ARB_map_buffer_alignment, we need to return a pointer
          * that's appropriately offset from a 64-byte-aligned base address.
          */
         assert(box->x >= 0);
         unsigned aligned_x = (unsigned)box->x % BUFFER_MAP_ALIGNMENT;
         staging_res_size = align(box->width + aligned_x,
                                  D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
         range.Begin = aligned_x;
      }

      pipe_resource_usage staging_usage = (usage & (PIPE_MAP_DISCARD_RANGE | PIPE_MAP_DISCARD_WHOLE_RESOURCE)) ?
         PIPE_USAGE_STREAM : PIPE_USAGE_STAGING;

      trans->staging_res = pipe_buffer_create(pctx->screen, 0,
                                              staging_usage,
                                              staging_res_size);
      if (!trans->staging_res) {
         slab_free(transfer_pool, trans);
         return NULL;
      }

      struct d3d12_resource *staging_res = d3d12_resource(trans->staging_res);

      if ((usage & (PIPE_MAP_DISCARD_RANGE | PIPE_MAP_DISCARD_WHOLE_RESOURCE | TC_TRANSFER_MAP_THREADED_UNSYNC)) == 0) {
         bool ret = true;
         if (pres->target == PIPE_BUFFER) {
            uint64_t src_offset = box->x;
            uint64_t dst_offset = src_offset % BUFFER_MAP_ALIGNMENT;
            transfer_buf_to_buf(ctx, res, staging_res, src_offset, dst_offset, box->width);
         } else
            ret = transfer_image_to_buf(ctx, res, staging_res, trans, 0);
         if (!ret)
            return NULL;
         d3d12_flush_cmdlist_and_wait(ctx);
      }

      range.End = staging_res_size - range.Begin;

      ptr = d3d12_bo_map(staging_res->bo, &range);
   }

   pipe_resource_reference(&ptrans->resource, pres);
   *transfer = ptrans;
   return ptr;
}

static void
d3d12_transfer_unmap(struct pipe_context *pctx,
                     struct pipe_transfer *ptrans)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_resource *res = d3d12_resource(ptrans->resource);
   struct d3d12_transfer *trans = (struct d3d12_transfer *)ptrans;
   D3D12_RANGE range = { 0, 0 };

   if (trans->data != nullptr) {
      if (trans->base.b.usage & PIPE_MAP_WRITE)
         write_zs_surface(pctx, res, trans);
      free(trans->data);
   } else if (trans->staging_res) {
      if(util_format_is_yuv(res->overall_format)) {

         /* Get planes information*/
         unsigned num_planes = util_format_get_num_planes(res->overall_format);
         pipe_resource *planes[d3d12_max_planes];
         unsigned int strides[d3d12_max_planes];
         unsigned int layer_strides[d3d12_max_planes];
         unsigned int offsets[d3d12_max_planes];
         unsigned staging_res_size = 0;

         d3d12_resource_get_planes_info(
            ptrans->resource,
            num_planes,
            planes,
            strides,
            layer_strides,
            offsets,
            &staging_res_size
         );      

         /* Flush the changed contents into the GPU texture*/

         /* In theory we should just flush only the contents for the plane*/
         /* requested in res->plane_slice, but the VAAPI frontend has this*/
         /* behaviour in which they assume that mapping the first plane of*/
         /* NV12, P010, etc resources will will give them a buffer containing*/
         /* both Y and UV planes contigously in vaDeriveImage and then vaMapBuffer*/
         /* so, flush them all*/
         
         struct d3d12_resource *staging_res = d3d12_resource(trans->staging_res);
         if (trans->base.b.usage & PIPE_MAP_WRITE) {
            assert(ptrans->box.x >= 0);
            range.Begin = res->base.b.target == PIPE_BUFFER ?
               (unsigned)ptrans->box.x % BUFFER_MAP_ALIGNMENT : 0;
            range.End = staging_res->base.b.width0 - range.Begin;
            
            d3d12_bo_unmap(staging_res->bo, &range);
            pipe_box original_box = ptrans->box;
            for (uint plane_slice = 0; plane_slice < num_planes; ++plane_slice) {
               /* Adjust strides, offsets to the corresponding plane for the copytexture operation*/
               d3d12_adjust_transfer_dimensions_for_plane(res,
                                                          plane_slice,
                                                          strides[plane_slice],
                                                          layer_strides[plane_slice],
                                                          offsets[plane_slice],
                                                          &original_box,
                                                          ptrans/*inout*/);  

               transfer_buf_to_image(ctx, d3d12_resource(planes[plane_slice]), staging_res, trans, 0);
            }
            ptrans->box = original_box;
         }

         pipe_resource_reference(&trans->staging_res, NULL);
      } else {
         struct d3d12_resource *staging_res = d3d12_resource(trans->staging_res);
         if (trans->base.b.usage & PIPE_MAP_WRITE) {
            assert(ptrans->box.x >= 0);
            range.Begin = res->base.b.target == PIPE_BUFFER ?
               (unsigned)ptrans->box.x % BUFFER_MAP_ALIGNMENT : 0;
            range.End = staging_res->base.b.width0 - range.Begin;
         }
         d3d12_bo_unmap(staging_res->bo, &range);

         if (trans->base.b.usage & PIPE_MAP_WRITE) {
            struct d3d12_context *ctx = d3d12_context(pctx);
            if (res->base.b.target == PIPE_BUFFER) {
               uint64_t dst_offset = trans->base.b.box.x;
               uint64_t src_offset = dst_offset % BUFFER_MAP_ALIGNMENT;
               transfer_buf_to_buf(ctx, staging_res, res, src_offset, dst_offset, ptrans->box.width);
            } else
               transfer_buf_to_image(ctx, res, staging_res, trans, 0);
         }

         pipe_resource_reference(&trans->staging_res, NULL);
      }
   } else {
      if (trans->base.b.usage & PIPE_MAP_WRITE) {
         range.Begin = ptrans->box.x;
         range.End = ptrans->box.x + ptrans->box.width;
      }
      d3d12_bo_unmap(res->bo, &range);
   }

   pipe_resource_reference(&ptrans->resource, NULL);
   slab_free(&d3d12_context(pctx)->transfer_pool, ptrans);
}

void
d3d12_context_resource_init(struct pipe_context *pctx)
{
   pctx->buffer_map = d3d12_transfer_map;
   pctx->buffer_unmap = d3d12_transfer_unmap;
   pctx->texture_map = d3d12_transfer_map;
   pctx->texture_unmap = d3d12_transfer_unmap;

   pctx->transfer_flush_region = u_default_transfer_flush_region;
   pctx->buffer_subdata = u_default_buffer_subdata;
   pctx->texture_subdata = u_default_texture_subdata;
}
