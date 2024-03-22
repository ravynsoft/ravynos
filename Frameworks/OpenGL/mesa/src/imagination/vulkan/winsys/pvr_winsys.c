/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fcntl.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <xf86drm.h>

#include "powervr/pvr_drm_public.h"
#include "pvr_private.h"
#include "pvr_winsys.h"
#include "vk_log.h"

#if defined(PVR_SUPPORT_SERVICES_DRIVER)
#   include "pvrsrvkm/pvr_srv_public.h"
#endif

void pvr_winsys_destroy(struct pvr_winsys *ws)
{
   const int display_fd = ws->display_fd;
   const int render_fd = ws->render_fd;

   ws->ops->destroy(ws);

   if (display_fd >= 0)
      close(display_fd);

   if (render_fd >= 0)
      close(render_fd);
}

VkResult pvr_winsys_create(const char *render_path,
                           const char *display_path,
                           const VkAllocationCallbacks *alloc,
                           struct pvr_winsys **const ws_out)
{
   drmVersionPtr version;
   VkResult result;
   int display_fd;
   int render_fd;

   render_fd = open(render_path, O_RDWR | O_CLOEXEC);
   if (render_fd < 0) {
      result = vk_errorf(NULL,
                         VK_ERROR_INITIALIZATION_FAILED,
                         "Failed to open render device %s",
                         render_path);
      goto err_out;
   }

   if (display_path) {
      display_fd = open(display_path, O_RDWR | O_CLOEXEC);
      if (display_fd < 0) {
         result = vk_errorf(NULL,
                            VK_ERROR_INITIALIZATION_FAILED,
                            "Failed to open display device %s",
                            display_path);
         goto err_close_render_fd;
      }
   } else {
      display_fd = -1;
   }

   version = drmGetVersion(render_fd);
   if (!version) {
      result = vk_errorf(NULL,
                         VK_ERROR_INCOMPATIBLE_DRIVER,
                         "Failed to query kernel driver version for device.");
      goto err_close_display_fd;
   }

   if (strcmp(version->name, "powervr") == 0) {
      result = pvr_drm_winsys_create(render_fd, display_fd, alloc, ws_out);
#if defined(PVR_SUPPORT_SERVICES_DRIVER)
   } else if (strcmp(version->name, "pvr") == 0) {
      result = pvr_srv_winsys_create(render_fd, display_fd, alloc, ws_out);
#endif
   } else {
      result = vk_errorf(
         NULL,
         VK_ERROR_INCOMPATIBLE_DRIVER,
         "Device does not use any of the supported pvrsrvkm or powervr kernel driver.");
   }

   drmFreeVersion(version);

   if (result != VK_SUCCESS)
      goto err_close_display_fd;

   return VK_SUCCESS;

err_close_display_fd:
   if (display_fd >= 0)
      close(display_fd);

err_close_render_fd:
   close(render_fd);

err_out:
   return result;
}
