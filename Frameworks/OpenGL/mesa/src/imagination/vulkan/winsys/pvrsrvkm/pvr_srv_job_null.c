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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#include "pvr_srv_job_null.h"
#include "pvr_srv_sync.h"
#include "pvr_winsys.h"
#include "util/libsync.h"
#include "vk_log.h"
#include "vk_sync.h"

VkResult pvr_srv_winsys_null_job_submit(struct pvr_winsys *ws,
                                        struct vk_sync_wait *waits,
                                        uint32_t wait_count,
                                        struct vk_sync_signal *signal)
{
   int fd = -1;

   /* Services doesn't support timeline syncs.
    * Timeline syncs should be emulated by the Vulkan runtime and converted
    * to binary syncs before this point.
    */
   assert((signal->signal_value == 0) &&
          !(signal->sync->flags & VK_SYNC_IS_TIMELINE));

   for (uint32_t wait_idx = 0; wait_idx < wait_count; wait_idx++) {
      struct pvr_srv_sync *srv_wait_sync = to_srv_sync(waits[wait_idx].sync);
      int ret;

      if (srv_wait_sync->fd < 0)
         continue;

      assert((waits[wait_idx].wait_value == 0) &&
             !(waits[wait_idx].sync->flags & VK_SYNC_IS_TIMELINE));

      ret = sync_accumulate("", &fd, srv_wait_sync->fd);
      if (ret) {
         if (fd >= 0)
            close(fd);

         return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
   }

   pvr_srv_set_sync_payload(to_srv_sync(signal->sync), fd);

   return VK_SUCCESS;
}
