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

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <vulkan/vulkan.h>
#include <xf86drm.h>

#include "pvr_srv_bridge.h"
#include "pvr_srv_job_common.h"
#include "vk_log.h"

VkResult pvr_srv_create_timeline(int render_fd, int *const fd_out)
{
   const char *render_path;
   VkResult result;
   int fd;

   render_path = drmGetRenderDeviceNameFromFd(render_fd);
   if (!render_path) {
      return vk_errorf(NULL,
                       VK_ERROR_UNKNOWN,
                       "Could not get render path from fd.");
   }

   fd = open(render_path, O_CLOEXEC | O_RDWR);
   drmFree((void *)render_path);
   if (fd == -1) {
      return vk_errorf(NULL,
                       VK_ERROR_UNKNOWN,
                       "Could not create timeline fd, errno: %s",
                       strerror(errno));
   }

   result = pvr_srv_init_module(fd, PVR_SRVKM_MODULE_TYPE_SYNC);
   if (result != VK_SUCCESS) {
      close(fd);
      return result;
   }

   *fd_out = fd;

   return VK_SUCCESS;
}
