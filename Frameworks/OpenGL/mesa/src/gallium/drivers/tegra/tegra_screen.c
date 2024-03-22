/*
 * Copyright © 2014-2018 NVIDIA Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>

#include <sys/stat.h>

#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/tegra_drm.h"
#include <xf86drm.h>

#include "loader/loader.h"
#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"

#include "frontend/drm_driver.h"

#include "nouveau/drm/nouveau_drm_public.h"

#include "tegra_context.h"
#include "tegra_resource.h"
#include "tegra_screen.h"

static void tegra_screen_destroy(struct pipe_screen *pscreen)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   screen->gpu->destroy(screen->gpu);
   free(pscreen);
}

static const char *
tegra_screen_get_name(struct pipe_screen *pscreen)
{
   return "tegra";
}

static const char *
tegra_screen_get_vendor(struct pipe_screen *pscreen)
{
   return "NVIDIA";
}

static const char *
tegra_screen_get_device_vendor(struct pipe_screen *pscreen)
{
   return "NVIDIA";
}

static int
tegra_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_param(screen->gpu, param);
}

static float
tegra_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_paramf(screen->gpu, param);
}

static int
tegra_screen_get_shader_param(struct pipe_screen *pscreen, enum pipe_shader_type shader,
                              enum pipe_shader_cap param)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_shader_param(screen->gpu, shader, param);
}

static int
tegra_screen_get_video_param(struct pipe_screen *pscreen,
                             enum pipe_video_profile profile,
                             enum pipe_video_entrypoint entrypoint,
                             enum pipe_video_cap param)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_video_param(screen->gpu, profile, entrypoint,
                                       param);
}

static int
tegra_screen_get_compute_param(struct pipe_screen *pscreen,
                               enum pipe_shader_ir ir_type,
                               enum pipe_compute_cap param,
                               void *retp)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_compute_param(screen->gpu, ir_type, param,
                                         retp);
}

static uint64_t
tegra_screen_get_timestamp(struct pipe_screen *pscreen)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_timestamp(screen->gpu);
}

static bool
tegra_screen_is_format_supported(struct pipe_screen *pscreen,
                                 enum pipe_format format,
                                 enum pipe_texture_target target,
                                 unsigned sample_count,
                                 unsigned storage_sample_count,
                                 unsigned usage)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->is_format_supported(screen->gpu, format, target,
                                           sample_count, storage_sample_count,
                                           usage);
}

static bool
tegra_screen_is_video_format_supported(struct pipe_screen *pscreen,
                                       enum pipe_format format,
                                       enum pipe_video_profile profile,
                                       enum pipe_video_entrypoint entrypoint)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->is_video_format_supported(screen->gpu, format, profile,
                                                 entrypoint);
}

static bool
tegra_screen_can_create_resource(struct pipe_screen *pscreen,
                                 const struct pipe_resource *template)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->can_create_resource(screen->gpu, template);
}

static int tegra_screen_import_resource(struct tegra_screen *screen,
                                        struct tegra_resource *resource)
{
   struct winsys_handle handle;
   bool status;
   int fd, err;

   memset(&handle, 0, sizeof(handle));
   handle.modifier = DRM_FORMAT_MOD_INVALID;
   handle.type = WINSYS_HANDLE_TYPE_FD;

   status = screen->gpu->resource_get_handle(screen->gpu, NULL, resource->gpu,
                                             &handle, 0);
   if (!status)
      return -EINVAL;

   assert(handle.modifier != DRM_FORMAT_MOD_INVALID);

   if (handle.modifier == DRM_FORMAT_MOD_INVALID) {
      close(handle.handle);
      return -EINVAL;
   }

   resource->modifier = handle.modifier;
   resource->stride = handle.stride;
   fd = handle.handle;

   err = drmPrimeFDToHandle(screen->fd, fd, &resource->handle);
   if (err < 0)
      err = -errno;

   close(fd);

   return err;
}

static struct pipe_resource *
tegra_screen_resource_create(struct pipe_screen *pscreen,
                             const struct pipe_resource *template)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   uint64_t modifier = DRM_FORMAT_MOD_INVALID;
   struct tegra_resource *resource;
   int err;

   resource = calloc(1, sizeof(*resource));
   if (!resource)
      return NULL;

   /*
    * Applications that create scanout resources without modifiers are very
    * unlikely to support modifiers at all. In that case the resources need
    * to be created with a pitch-linear layout so that they can be properly
    * shared with scanout hardware.
    *
    * Technically it is possible for applications to create resources without
    * specifying a modifier but still query the modifier associated with the
    * resource (e.g. using gbm_bo_get_modifier()) before handing it to the
    * framebuffer creation API (such as the DRM_IOCTL_MODE_ADDFB2 IOCTL).
    */
   if (template->bind & PIPE_BIND_SCANOUT)
      modifier = DRM_FORMAT_MOD_LINEAR;

   resource->gpu = screen->gpu->resource_create_with_modifiers(screen->gpu,
                                                               template,
                                                               &modifier, 1);
   if (!resource->gpu)
      goto free;

   /* import scanout buffers for display */
   if (template->bind & PIPE_BIND_SCANOUT) {
      err = tegra_screen_import_resource(screen, resource);
      if (err < 0)
         goto destroy;
   }

   memcpy(&resource->base, resource->gpu, sizeof(*resource->gpu));
   pipe_reference_init(&resource->base.reference, 1);
   resource->base.screen = &screen->base;

   /* use private reference count for wrapped resources */
   resource->gpu->reference.count += 100000000;
   resource->refcount = 100000000;

   return &resource->base;

destroy:
   screen->gpu->resource_destroy(screen->gpu, resource->gpu);
free:
   free(resource);
   return NULL;
}

/* XXX */
static struct pipe_resource *
tegra_screen_resource_create_front(struct pipe_screen *pscreen,
                                   const struct pipe_resource *template,
                                   const void *map_front_private)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct pipe_resource *resource;

   resource = screen->gpu->resource_create_front(screen->gpu, template,
                                                 map_front_private);
   if (resource)
      resource->screen = pscreen;

   return resource;
}

static struct pipe_resource *
tegra_screen_resource_from_handle(struct pipe_screen *pscreen,
                                  const struct pipe_resource *template,
                                  struct winsys_handle *handle,
                                  unsigned usage)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct tegra_resource *resource;

   resource = calloc(1, sizeof(*resource));
   if (!resource)
      return NULL;

   resource->gpu = screen->gpu->resource_from_handle(screen->gpu, template,
                                                     handle, usage);
   if (!resource->gpu) {
      free(resource);
      return NULL;
   }

   memcpy(&resource->base, resource->gpu, sizeof(*resource->gpu));
   pipe_reference_init(&resource->base.reference, 1);
   resource->base.screen = &screen->base;

   return &resource->base;
}

/* XXX */
static struct pipe_resource *
tegra_screen_resource_from_user_memory(struct pipe_screen *pscreen,
                                       const struct pipe_resource *template,
                                       void *buffer)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct pipe_resource *resource;

   resource = screen->gpu->resource_from_user_memory(screen->gpu, template,
                                                     buffer);
   if (resource)
      resource->screen = pscreen;

   return resource;
}

static bool
tegra_screen_resource_get_handle(struct pipe_screen *pscreen,
                                 struct pipe_context *pcontext,
                                 struct pipe_resource *presource,
                                 struct winsys_handle *handle,
                                 unsigned usage)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   bool ret = true;

   /*
    * Assume that KMS handles for scanout resources will only ever be used
    * to pass buffers into Tegra DRM for display. In all other cases, return
    * the Nouveau handle, assuming they will be used for sharing in DRI2/3.
    */
   if (handle->type == WINSYS_HANDLE_TYPE_KMS &&
       presource->bind & PIPE_BIND_SCANOUT) {
      handle->modifier = resource->modifier;
      handle->handle = resource->handle;
      handle->stride = resource->stride;
   } else {
      ret = screen->gpu->resource_get_handle(screen->gpu,
                                             context ? context->gpu : NULL,
                                             resource->gpu, handle, usage);
   }

   return ret;
}

static void
tegra_screen_resource_destroy(struct pipe_screen *pscreen,
                              struct pipe_resource *presource)
{
   struct tegra_resource *resource = to_tegra_resource(presource);

   /* adjust private reference count */
   p_atomic_add(&resource->gpu->reference.count, -resource->refcount);
   pipe_resource_reference(&resource->gpu, NULL);
   free(resource);
}

static void
tegra_screen_flush_frontbuffer(struct pipe_screen *pscreen,
                               struct pipe_context *pcontext,
                               struct pipe_resource *resource,
                               unsigned int level,
                               unsigned int layer,
                               void *winsys_drawable_handle,
                               struct pipe_box *box)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct tegra_context *context = to_tegra_context(pcontext);

   screen->gpu->flush_frontbuffer(screen->gpu,
                                  context ? context->gpu : NULL,
                                  resource, level, layer,
                                  winsys_drawable_handle, box);
}

static void
tegra_screen_fence_reference(struct pipe_screen *pscreen,
                             struct pipe_fence_handle **ptr,
                             struct pipe_fence_handle *fence)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   screen->gpu->fence_reference(screen->gpu, ptr, fence);
}

static bool
tegra_screen_fence_finish(struct pipe_screen *pscreen,
                          struct pipe_context *pcontext,
                          struct pipe_fence_handle *fence,
                          uint64_t timeout)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->fence_finish(screen->gpu,
                                    context ? context->gpu : NULL,
                                    fence, timeout);
}

static int
tegra_screen_fence_get_fd(struct pipe_screen *pscreen,
                          struct pipe_fence_handle *fence)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->fence_get_fd(screen->gpu, fence);
}

static int
tegra_screen_get_driver_query_info(struct pipe_screen *pscreen,
                                   unsigned int index,
                                   struct pipe_driver_query_info *info)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_driver_query_info(screen->gpu, index, info);
}

static int
tegra_screen_get_driver_query_group_info(struct pipe_screen *pscreen,
                                         unsigned int index,
                                         struct pipe_driver_query_group_info *info)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_driver_query_group_info(screen->gpu, index, info);
}

static void
tegra_screen_query_memory_info(struct pipe_screen *pscreen,
                               struct pipe_memory_info *info)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   screen->gpu->query_memory_info(screen->gpu, info);
}

static const void *
tegra_screen_get_compiler_options(struct pipe_screen *pscreen,
                                  enum pipe_shader_ir ir,
                                  enum pipe_shader_type shader)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   const void *options = NULL;

   if (screen->gpu->get_compiler_options)
      options = screen->gpu->get_compiler_options(screen->gpu, ir, shader);

   return options;
}

static struct disk_cache *
tegra_screen_get_disk_shader_cache(struct pipe_screen *pscreen)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_disk_shader_cache(screen->gpu);
}

static struct pipe_resource *
tegra_screen_resource_create_with_modifiers(struct pipe_screen *pscreen,
                                            const struct pipe_resource *template,
                                            const uint64_t *modifiers,
                                            int count)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct pipe_resource tmpl = *template;
   struct tegra_resource *resource;
   int err;

   resource = calloc(1, sizeof(*resource));
   if (!resource)
      return NULL;

   /*
    * Assume that resources created with modifiers will always be used for
    * scanout. This is necessary because some of the APIs that are used to
    * create resources with modifiers (e.g. gbm_bo_create_with_modifiers())
    * can't pass along usage information. Adding that capability might be
    * worth adding to remove this ambiguity. Not all future use-cases that
    * involve modifiers may always be targetting scanout hardware.
    */
   tmpl.bind |= PIPE_BIND_SCANOUT;

   resource->gpu = screen->gpu->resource_create_with_modifiers(screen->gpu,
                                                               &tmpl,
                                                               modifiers,
                                                               count);
   if (!resource->gpu)
      goto free;

   err = tegra_screen_import_resource(screen, resource);
   if (err < 0)
      goto destroy;

   memcpy(&resource->base, resource->gpu, sizeof(*resource->gpu));
   pipe_reference_init(&resource->base.reference, 1);
   resource->base.screen = &screen->base;

   return &resource->base;

destroy:
   screen->gpu->resource_destroy(screen->gpu, resource->gpu);
free:
   free(resource);
   return NULL;
}

static void tegra_screen_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                                                enum pipe_format format,
                                                int max, uint64_t *modifiers,
                                                unsigned int *external_only,
                                                int *count)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   screen->gpu->query_dmabuf_modifiers(screen->gpu, format, max, modifiers,
                                       external_only, count);
}

static bool
tegra_screen_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                          uint64_t modifier,
                                          enum pipe_format format,
                                          bool *external_only)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->is_dmabuf_modifier_supported(screen->gpu, modifier,
                                                    format, external_only);
}

static unsigned int
tegra_screen_get_dmabuf_modifier_planes(struct pipe_screen *pscreen,
                                        uint64_t modifier,
                                        enum pipe_format format)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->get_dmabuf_modifier_planes ?
      screen->gpu->get_dmabuf_modifier_planes(screen->gpu, modifier, format) :
      util_format_get_num_planes(format);
}

static struct pipe_memory_object *
tegra_screen_memobj_create_from_handle(struct pipe_screen *pscreen,
                                       struct winsys_handle *handle,
                                       bool dedicated)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->gpu->memobj_create_from_handle(screen->gpu, handle,
                                                 dedicated);
}

static int
tegra_screen_get_fd(struct pipe_screen *pscreen)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);

   return screen->fd;
}

struct pipe_screen *
tegra_screen_create(int fd)
{
   struct tegra_screen *screen;
   const char * const drivers[] = {"nouveau"};

   screen = calloc(1, sizeof(*screen));
   if (!screen)
      return NULL;

   screen->fd = fd;

   screen->gpu_fd =
      loader_open_render_node_platform_device(drivers, ARRAY_SIZE(drivers));
   if (screen->gpu_fd < 0) {
      if (errno != ENOENT)
         fprintf(stderr, "failed to open GPU device: %s\n", strerror(errno));

      free(screen);
      return NULL;
   }

   screen->gpu = nouveau_drm_screen_create(screen->gpu_fd);
   if (!screen->gpu) {
      fprintf(stderr, "failed to create GPU screen\n");
      close(screen->gpu_fd);
      free(screen);
      return NULL;
   }

   screen->base.destroy = tegra_screen_destroy;
   screen->base.get_name = tegra_screen_get_name;
   screen->base.get_vendor = tegra_screen_get_vendor;
   screen->base.get_device_vendor = tegra_screen_get_device_vendor;
   screen->base.get_screen_fd = tegra_screen_get_fd;
   screen->base.get_param = tegra_screen_get_param;
   screen->base.get_paramf = tegra_screen_get_paramf;
   screen->base.get_shader_param = tegra_screen_get_shader_param;
   screen->base.get_video_param = tegra_screen_get_video_param;
   screen->base.get_compute_param = tegra_screen_get_compute_param;
   screen->base.get_timestamp = tegra_screen_get_timestamp;
   screen->base.context_create = tegra_screen_context_create;
   screen->base.is_format_supported = tegra_screen_is_format_supported;
   screen->base.is_video_format_supported = tegra_screen_is_video_format_supported;

   /* allow fallback implementation if GPU driver doesn't implement it */
   if (screen->gpu->can_create_resource)
      screen->base.can_create_resource = tegra_screen_can_create_resource;

   screen->base.resource_create = tegra_screen_resource_create;
   screen->base.resource_create_front = tegra_screen_resource_create_front;
   screen->base.resource_from_handle = tegra_screen_resource_from_handle;
   screen->base.resource_from_user_memory = tegra_screen_resource_from_user_memory;
   screen->base.resource_get_handle = tegra_screen_resource_get_handle;
   screen->base.resource_destroy = tegra_screen_resource_destroy;

   screen->base.flush_frontbuffer = tegra_screen_flush_frontbuffer;
   screen->base.fence_reference = tegra_screen_fence_reference;
   screen->base.fence_finish = tegra_screen_fence_finish;
   screen->base.fence_get_fd = tegra_screen_fence_get_fd;

   screen->base.get_driver_query_info = tegra_screen_get_driver_query_info;
   screen->base.get_driver_query_group_info = tegra_screen_get_driver_query_group_info;
   screen->base.query_memory_info = tegra_screen_query_memory_info;

   screen->base.get_compiler_options = tegra_screen_get_compiler_options;
   screen->base.get_disk_shader_cache = tegra_screen_get_disk_shader_cache;

   screen->base.resource_create_with_modifiers = tegra_screen_resource_create_with_modifiers;
   screen->base.query_dmabuf_modifiers = tegra_screen_query_dmabuf_modifiers;
   screen->base.is_dmabuf_modifier_supported = tegra_screen_is_dmabuf_modifier_supported;
   screen->base.get_dmabuf_modifier_planes = tegra_screen_get_dmabuf_modifier_planes;
   screen->base.memobj_create_from_handle = tegra_screen_memobj_create_from_handle;

   return &screen->base;
}
