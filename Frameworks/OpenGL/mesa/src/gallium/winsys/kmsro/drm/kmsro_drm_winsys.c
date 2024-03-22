/*
 * Copyright (C) 2016 Christian Gmeiner <christian.gmeiner@gmail.com>
 * Copyright (C) 2017 Broadcom
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

#include <fcntl.h>
#include <unistd.h>

#include "kmsro_drm_public.h"
#include "v3d/drm/v3d_drm_public.h"
#include "vc4/drm/vc4_drm_public.h"
#include "etnaviv/drm/etnaviv_drm_public.h"
#include "freedreno/drm/freedreno_drm_public.h"
#include "panfrost/drm/panfrost_drm_public.h"
#include "lima/drm/lima_drm_public.h"
#include "asahi/drm/asahi_drm_public.h"
#include "xf86drm.h"

#include "pipe/p_screen.h"
#include "pipe-loader/pipe_loader.h"
#include "renderonly/renderonly.h"
#include "util/u_memory.h"

#include "loader.h"

static void kmsro_ro_destroy(struct renderonly *ro)
{
   if (ro->gpu_fd >= 0)
      close(ro->gpu_fd);

   util_sparse_array_finish(&ro->bo_map);

   FREE(ro);
}

struct pipe_screen *kmsro_drm_screen_create(int kms_fd,
                                            const struct pipe_screen_config *config)
{
   struct pipe_screen *screen = NULL;
   struct renderonly *ro = CALLOC_STRUCT(renderonly);
   char *render_dev_name = NULL;

   if (!ro)
      return NULL;

   ro->kms_fd = kms_fd;
   ro->gpu_fd = pipe_loader_get_compatible_render_capable_device_fd(kms_fd);
   if (ro->gpu_fd < 0) {
      FREE(ro);
      return NULL;
   }

   render_dev_name = loader_get_kernel_driver_name(ro->gpu_fd);
   if (!render_dev_name) {
      close(ro->gpu_fd);
      FREE(ro);
      return NULL;
   }

   ro->destroy = kmsro_ro_destroy;
   util_sparse_array_init(&ro->bo_map, sizeof(struct renderonly_scanout), 64);
   simple_mtx_init(&ro->bo_map_lock, mtx_plain);

   if (strcmp(render_dev_name, "asahi") == 0) {
#if defined(GALLIUM_ASAHI)
      ro->create_for_resource = renderonly_create_gpu_import_for_resource;
      screen = asahi_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   }
   else if (strcmp(render_dev_name, "etnaviv") == 0) {
#if defined(GALLIUM_ETNAVIV)
      ro->create_for_resource = renderonly_create_kms_dumb_buffer_for_resource;
      screen = etna_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   } else if (strcmp(render_dev_name, "msm") == 0) {
#if defined(GALLIUM_FREEDRENO)
      ro->create_for_resource = renderonly_create_kms_dumb_buffer_for_resource;
      screen = fd_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   } else if (strcmp(render_dev_name, "lima") == 0) {
#if defined(GALLIUM_LIMA)
      ro->create_for_resource = renderonly_create_kms_dumb_buffer_for_resource;
      screen = lima_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   } else if (strcmp(render_dev_name, "panfrost") == 0) {
#if defined(GALLIUM_PANFROST)
      ro->create_for_resource = panfrost_create_kms_dumb_buffer_for_resource;
      screen = panfrost_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   } else if (strcmp(render_dev_name, "v3d") == 0) {
#if defined(GALLIUM_V3D)
      ro->create_for_resource = renderonly_create_kms_dumb_buffer_for_resource;
      screen = v3d_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   } else if (strcmp(render_dev_name, "vc4") == 0) {
#if defined(GALLIUM_VC4)
      /* Passes the vc4-allocated BO through to the KMS-only DRM device using
       * PRIME buffer sharing.  The VC4 BO must be linear, which the SCANOUT
       * flag on allocation will have ensured.
       */
      ro->create_for_resource = renderonly_create_gpu_import_for_resource;
      screen = vc4_drm_screen_create_renderonly(ro->gpu_fd, ro, config);
#endif
   }

   free(render_dev_name);

   return screen;
}
