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

#include "d3d12_video_buffer.h"
#include "d3d12_resource.h"
#include "d3d12_video_dec.h"
#include "d3d12_residency.h"

#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_video_buffer.h"
#include "util/u_sampler.h"
#include "frontend/winsys_handle.h"
#include "d3d12_format.h"
#include "d3d12_screen.h"

static struct pipe_video_buffer *
d3d12_video_buffer_create_impl(struct pipe_context *pipe,
                              const struct pipe_video_buffer *tmpl,
                              struct winsys_handle *handle,
                              unsigned usage)
{
   assert(pipe);
   assert(tmpl);

   ///
   /// Initialize d3d12_video_buffer
   ///

   // Not using new doesn't call ctor and the initializations in the class declaration are lost
   struct d3d12_video_buffer *pD3D12VideoBuffer = new d3d12_video_buffer;

   // Fill base template
   pD3D12VideoBuffer->base               = *tmpl;
   pD3D12VideoBuffer->base.buffer_format = tmpl->buffer_format;
   pD3D12VideoBuffer->base.context       = pipe;
   pD3D12VideoBuffer->base.width         = tmpl->width;
   pD3D12VideoBuffer->base.height        = tmpl->height;
   pD3D12VideoBuffer->base.interlaced    = tmpl->interlaced;
   pD3D12VideoBuffer->base.associated_data = nullptr;
   pD3D12VideoBuffer->base.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_CUSTOM;

   // Fill vtable
   pD3D12VideoBuffer->base.destroy                     = d3d12_video_buffer_destroy;
   pD3D12VideoBuffer->base.get_resources               = d3d12_video_buffer_resources;
   pD3D12VideoBuffer->base.get_sampler_view_planes     = d3d12_video_buffer_get_sampler_view_planes;
   pD3D12VideoBuffer->base.get_sampler_view_components = d3d12_video_buffer_get_sampler_view_components;
   pD3D12VideoBuffer->base.get_surfaces                = d3d12_video_buffer_get_surfaces;
   pD3D12VideoBuffer->base.destroy_associated_data     = d3d12_video_buffer_destroy_associated_data;

   struct pipe_resource templ;
   memset(&templ, 0, sizeof(templ));
   templ.target     = PIPE_TEXTURE_2D;
   templ.bind       = pD3D12VideoBuffer->base.bind;
   templ.format     = pD3D12VideoBuffer->base.buffer_format;
   if (handle)
   {
      // YUV 4:2:0 formats in D3D12 always require multiple of 2 dimensions
      // We must respect the input dimensions of the imported resource handle (e.g no extra aligning)
      templ.width0     = align(pD3D12VideoBuffer->base.width, 2);
      templ.height0    = align(pD3D12VideoBuffer->base.height, 2);
   }
   else
   {
      // When creating (e.g not importing) resources we allocate
      // with a higher alignment to maximize HW compatibility
      templ.width0     = align(pD3D12VideoBuffer->base.width, 2);
      templ.height0    = align(pD3D12VideoBuffer->base.height, 16);
   }
   templ.depth0     = 1;
   templ.array_size = 1;
   templ.flags      = 0;

   // This calls d3d12_create_resource as the function ptr is set in d3d12_screen.resource_create
   if(handle)
   {
      // WINSYS_HANDLE_TYPE_D3D12_RES implies taking ownership of the reference
      if(handle->type == WINSYS_HANDLE_TYPE_D3D12_RES)
         ((IUnknown *)handle->com_obj)->AddRef();
      pD3D12VideoBuffer->texture = (struct d3d12_resource *) pipe->screen->resource_from_handle(pipe->screen, &templ, handle, usage);
   }
   else
      pD3D12VideoBuffer->texture = (struct d3d12_resource *) pipe->screen->resource_create(pipe->screen, &templ);

   if (pD3D12VideoBuffer->texture == nullptr) {
      debug_printf("[d3d12_video_buffer] d3d12_video_buffer_create - Call to resource_create() to create "
                      "d3d12_resource failed\n");
      goto failed;
   }

   d3d12_promote_to_permanent_residency((struct d3d12_screen*) pipe->screen, pD3D12VideoBuffer->texture);

   pD3D12VideoBuffer->num_planes = util_format_get_num_planes(pD3D12VideoBuffer->texture->overall_format);
   return &pD3D12VideoBuffer->base;

failed:
   d3d12_video_buffer_destroy((struct pipe_video_buffer *) pD3D12VideoBuffer);

   return nullptr;
}

/**
 * creates a video buffer from a handle
 */
struct pipe_video_buffer *
d3d12_video_buffer_from_handle(struct pipe_context *pipe,
                               const struct pipe_video_buffer *tmpl,
                               struct winsys_handle *handle,
                               unsigned usage)
{
   struct pipe_video_buffer updated_template = {};
   if ((handle->format == PIPE_FORMAT_NONE) || (tmpl == nullptr) || (tmpl->buffer_format == PIPE_FORMAT_NONE) ||
       (tmpl->width == 0) || (tmpl->height == 0)) {
      ID3D12Resource *d3d12_res = nullptr;
      if (handle->type == WINSYS_HANDLE_TYPE_D3D12_RES) {
         d3d12_res = (ID3D12Resource *) handle->com_obj;
      } else if (handle->type == WINSYS_HANDLE_TYPE_FD) {
#ifdef _WIN32
         HANDLE d3d_handle = handle->handle;
#else
         HANDLE d3d_handle = (HANDLE) (intptr_t) handle->handle;
#endif
         if (FAILED(d3d12_screen(pipe->screen)->dev->OpenSharedHandle(d3d_handle, IID_PPV_ARGS(&d3d12_res)))) {
            return NULL;
         }
      }
      D3D12_RESOURCE_DESC res_desc = GetDesc(d3d12_res);
      updated_template.width = res_desc.Width;
      updated_template.height = res_desc.Height;
      updated_template.buffer_format = d3d12_get_pipe_format(res_desc.Format);
      handle->format = updated_template.buffer_format;

      // if passed an external com_ptr (e.g WINSYS_HANDLE_TYPE_D3D12_RES) do not release it
      if (handle->type == WINSYS_HANDLE_TYPE_FD)
         d3d12_res->Release();
   } else {
      updated_template = *tmpl;
   }

   return d3d12_video_buffer_create_impl(pipe, &updated_template, handle, usage);
}

/**
 * creates a video buffer
 */
struct pipe_video_buffer *
d3d12_video_buffer_create(struct pipe_context *pipe, const struct pipe_video_buffer *tmpl)
{
   return d3d12_video_buffer_create_impl(pipe, tmpl, NULL, 0);
}

/**
 * destroy this video buffer
 */
void
d3d12_video_buffer_destroy(struct pipe_video_buffer *buffer)
{
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) buffer;

   // Destroy pD3D12VideoBuffer->texture (if any)
   if (pD3D12VideoBuffer->texture) {
      pipe_resource *pBaseResource = &pD3D12VideoBuffer->texture->base.b;
      pipe_resource_reference(&pBaseResource, NULL);
   }

   // Destroy associated data (if any)
   if (pD3D12VideoBuffer->base.associated_data != nullptr) {
      d3d12_video_buffer_destroy_associated_data(pD3D12VideoBuffer->base.associated_data);
      // Set to nullptr after cleanup, no dangling pointers
      pD3D12VideoBuffer->base.associated_data = nullptr;
   }

   for (uint i = 0; i < pD3D12VideoBuffer->surfaces.size(); ++i) {
      if (pD3D12VideoBuffer->surfaces[i] != NULL) {
         pipe_surface_reference(&pD3D12VideoBuffer->surfaces[i], NULL);
      }
   }

   for (uint i = 0; i < pD3D12VideoBuffer->sampler_view_planes.size(); ++i) {
      if (pD3D12VideoBuffer->sampler_view_planes[i] != NULL) {
         pipe_sampler_view_reference(&pD3D12VideoBuffer->sampler_view_planes[i], NULL);
      }
   }

   for (uint i = 0; i < pD3D12VideoBuffer->sampler_view_components.size(); ++i) {
      if (pD3D12VideoBuffer->sampler_view_components[i] != NULL) {
         pipe_sampler_view_reference(&pD3D12VideoBuffer->sampler_view_components[i], NULL);
      }
   }

   delete pD3D12VideoBuffer;
}

/*
 * destroy the associated data
 */
void
d3d12_video_buffer_destroy_associated_data(void *associated_data)
{ }

/**
 * get an individual surfaces for each plane
 */
struct pipe_surface **
d3d12_video_buffer_get_surfaces(struct pipe_video_buffer *buffer)
{
   assert(buffer);
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) buffer;
   struct pipe_context *      pipe              = pD3D12VideoBuffer->base.context;
   struct pipe_surface        surface_template  = {};

   // Some video frameworks iterate over [0..VL_MAX_SURFACES) and ignore the nullptr entries
   // So we have to null initialize the other surfaces not used from [num_planes..VL_MAX_SURFACES)
   // Like in src/gallium/frontends/va/surface.c
   pD3D12VideoBuffer->surfaces.resize(VL_MAX_SURFACES, nullptr);

   // pCurPlaneResource refers to the planar resource, not the overall resource.
   // in d3d12_resource this is handled by having a linked list of planes with
   // d3dRes->base.next ptr to next plane resource
   // starting with the plane 0 being the overall resource
   struct pipe_resource *pCurPlaneResource = &pD3D12VideoBuffer->texture->base.b;

   for (uint PlaneSlice = 0; PlaneSlice < pD3D12VideoBuffer->num_planes; ++PlaneSlice) {
      if (!pD3D12VideoBuffer->surfaces[PlaneSlice]) {
         memset(&surface_template, 0, sizeof(surface_template));
         surface_template.format =
            util_format_get_plane_format(pD3D12VideoBuffer->texture->overall_format, PlaneSlice);

         pD3D12VideoBuffer->surfaces[PlaneSlice] =
            pipe->create_surface(pipe, pCurPlaneResource, &surface_template);

         if (!pD3D12VideoBuffer->surfaces[PlaneSlice]) {
            goto error;
         }
      }
      pCurPlaneResource = pCurPlaneResource->next;
   }

   return pD3D12VideoBuffer->surfaces.data();

error:
   for (uint PlaneSlice = 0; PlaneSlice < pD3D12VideoBuffer->num_planes; ++PlaneSlice) {
      pipe_surface_reference(&pD3D12VideoBuffer->surfaces[PlaneSlice], NULL);
   }

   return nullptr;
}

/**
 * get an individual resource for each plane,
 * only returns existing resources by reference
 */
void
d3d12_video_buffer_resources(struct pipe_video_buffer *buffer,
                             struct pipe_resource **resources)
{
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) buffer;
   assert(pD3D12VideoBuffer);

   // pCurPlaneResource refers to the planar resource, not the overall resource.
   // in d3d12_resource this is handled by having a linked list of planes with
   // d3dRes->base.next ptr to next plane resource
   // starting with the plane 0 being the overall resource
   struct pipe_resource *pCurPlaneResource = &pD3D12VideoBuffer->texture->base.b;

   for (uint i = 0; i < pD3D12VideoBuffer->num_planes; ++i) {
      assert(pCurPlaneResource); // the d3d12_resource has a linked list with the exact name of number of elements
                                 // as planes

      resources[i] = pCurPlaneResource;
      pCurPlaneResource = pCurPlaneResource->next;
   }
}

/**
 * get an individual sampler view for each plane
 */
struct pipe_sampler_view **
d3d12_video_buffer_get_sampler_view_planes(struct pipe_video_buffer *buffer)
{
   assert(buffer);
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) buffer;
   struct pipe_context *      pipe              = pD3D12VideoBuffer->base.context;
   struct pipe_sampler_view   samplerViewTemplate;

   // Some video frameworks iterate over [0..VL_MAX_SURFACES) and ignore the nullptr entries
   // So we have to null initialize the other surfaces not used from [num_planes..VL_MAX_SURFACES)
   // Like in src/gallium/frontends/vdpau/surface.c
   pD3D12VideoBuffer->sampler_view_planes.resize(VL_MAX_SURFACES, nullptr);

   // pCurPlaneResource refers to the planar resource, not the overall resource.
   // in d3d12_resource this is handled by having a linked list of planes with
   // d3dRes->base.next ptr to next plane resource
   // starting with the plane 0 being the overall resource
   struct pipe_resource *pCurPlaneResource = &pD3D12VideoBuffer->texture->base.b;

   for (uint i = 0; i < pD3D12VideoBuffer->num_planes; ++i) {
      if (!pD3D12VideoBuffer->sampler_view_planes[i]) {
         assert(pCurPlaneResource);   // the d3d12_resource has a linked list with the exact name of number of elements
                                      // as planes

         memset(&samplerViewTemplate, 0, sizeof(samplerViewTemplate));
         u_sampler_view_default_template(&samplerViewTemplate, pCurPlaneResource, pCurPlaneResource->format);

         pD3D12VideoBuffer->sampler_view_planes[i] =
            pipe->create_sampler_view(pipe, pCurPlaneResource, &samplerViewTemplate);

         if (!pD3D12VideoBuffer->sampler_view_planes[i]) {
            goto error;
         }
      }

      pCurPlaneResource = pCurPlaneResource->next;
   }

   return pD3D12VideoBuffer->sampler_view_planes.data();

error:
   for (uint i = 0; i < pD3D12VideoBuffer->num_planes; ++i) {
      pipe_sampler_view_reference(&pD3D12VideoBuffer->sampler_view_planes[i], NULL);
   }

   return nullptr;
}

/**
 * get an individual sampler view for each component
 */
struct pipe_sampler_view **
d3d12_video_buffer_get_sampler_view_components(struct pipe_video_buffer *buffer)
{
   assert(buffer);
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) buffer;
   struct pipe_context *      pipe              = pD3D12VideoBuffer->base.context;
   struct pipe_sampler_view   samplerViewTemplate;

   // pCurPlaneResource refers to the planar resource, not the overall resource.
   // in d3d12_resource this is handled by having a linked list of planes with
   // d3dRes->base.next ptr to next plane resource
   // starting with the plane 0 being the overall resource
   struct pipe_resource *pCurPlaneResource = &pD3D12VideoBuffer->texture->base.b;

   const uint32_t MAX_NUM_COMPONENTS = 4; // ie. RGBA formats
   // At the end of the loop, "component" will have the total number of items valid in sampler_view_components
   // since component can end up being <= MAX_NUM_COMPONENTS, we assume MAX_NUM_COMPONENTS first and then resize/adjust to
   // fit the container size pD3D12VideoBuffer->sampler_view_components to the actual components number
   pD3D12VideoBuffer->sampler_view_components.resize(MAX_NUM_COMPONENTS, nullptr);
   uint component = 0;

   for (uint i = 0; i < pD3D12VideoBuffer->num_planes; ++i) {
      // For example num_components would be 1 for the Y plane (R8 in NV12), 2 for the UV plane (R8G8 in NV12)
      unsigned num_components = util_format_get_nr_components(pCurPlaneResource->format);

      for (uint j = 0; j < num_components; ++j, ++component) {

         if (!pD3D12VideoBuffer->sampler_view_components[component]) {
            memset(&samplerViewTemplate, 0, sizeof(samplerViewTemplate));
            u_sampler_view_default_template(&samplerViewTemplate, pCurPlaneResource, pCurPlaneResource->format);
            samplerViewTemplate.swizzle_r = samplerViewTemplate.swizzle_g = samplerViewTemplate.swizzle_b =
               PIPE_SWIZZLE_X + j;
            samplerViewTemplate.swizzle_a = PIPE_SWIZZLE_1;

            pD3D12VideoBuffer->sampler_view_components[component] =
               pipe->create_sampler_view(pipe, pCurPlaneResource, &samplerViewTemplate);
            if (!pD3D12VideoBuffer->sampler_view_components[component]) {
               goto error;
            }
         }
      }

      pCurPlaneResource = pCurPlaneResource->next;
   }

   // Adjust size to fit component <= VL_NUM_COMPONENTS
   pD3D12VideoBuffer->sampler_view_components.resize(component);

   return pD3D12VideoBuffer->sampler_view_components.data();

error:
   for (uint i = 0; i < pD3D12VideoBuffer->num_planes; ++i) {
      pipe_sampler_view_reference(&pD3D12VideoBuffer->sampler_view_components[i], NULL);
   }

   return nullptr;
}
