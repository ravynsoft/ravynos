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

#ifndef PVR_WINSYS_HELPER_H
#define PVR_WINSYS_HELPER_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <xf86drm.h>

#include "pvr_types.h"
#include "vk_log.h"

struct pvr_winsys;
struct pvr_winsys_heap;
struct pvr_winsys_static_data_offsets;
struct pvr_winsys_vma;

typedef VkResult (*const heap_alloc_carveout_func)(
   struct pvr_winsys_heap *const heap,
   const pvr_dev_addr_t carveout_dev_addr,
   uint64_t size,
   uint64_t alignment,
   struct pvr_winsys_vma **vma_out);

VkResult pvr_winsys_helper_display_buffer_create(struct pvr_winsys *ws,
                                                 uint64_t size,
                                                 uint32_t *const handle_out);
VkResult pvr_winsys_helper_display_buffer_destroy(struct pvr_winsys *ws,
                                                  uint32_t handle);

bool pvr_winsys_helper_winsys_heap_finish(struct pvr_winsys_heap *const heap);

VkResult pvr_winsys_helper_heap_alloc(struct pvr_winsys_heap *const heap,
                                      uint64_t size,
                                      uint64_t alignment,
                                      struct pvr_winsys_vma *const vma);
void pvr_winsys_helper_heap_free(struct pvr_winsys_vma *const vma);

VkResult pvr_winsys_helper_allocate_static_memory(
   struct pvr_winsys *const ws,
   heap_alloc_carveout_func heap_alloc_carveout,
   struct pvr_winsys_heap *const general_heap,
   struct pvr_winsys_heap *const pds_heap,
   struct pvr_winsys_heap *const usc_heap,
   struct pvr_winsys_vma **const general_vma_out,
   struct pvr_winsys_vma **const pds_vma_out,
   struct pvr_winsys_vma **const usc_vma_out);
void pvr_winsys_helper_free_static_memory(
   struct pvr_winsys_vma *const general_vma,
   struct pvr_winsys_vma *const pds_vma,
   struct pvr_winsys_vma *const usc_vma);

VkResult
pvr_winsys_helper_fill_static_memory(struct pvr_winsys *const ws,
                                     struct pvr_winsys_vma *const general_vma,
                                     struct pvr_winsys_vma *const pds_vma,
                                     struct pvr_winsys_vma *const usc_vma);

static inline VkResult pvr_mmap(const size_t len,
                                const int prot,
                                const int flags,
                                const int fd,
                                const off_t offset,
                                void **const map_out)
{
   void *const map = mmap(NULL, len, prot, flags, fd, offset);
   if (map == MAP_FAILED) {
      const int err = errno;
      return vk_errorf(NULL,
                       VK_ERROR_MEMORY_MAP_FAILED,
                       "mmap failed (errno %d: %s)",
                       err,
                       strerror(err));
   }

   *map_out = map;

   return VK_SUCCESS;
}

static inline VkResult pvr_munmap(void *const addr, const size_t len)
{
   const int ret = munmap(addr, len);
   if (ret) {
      const int err = errno;
      return vk_errorf(NULL,
                       VK_ERROR_UNKNOWN,
                       "munmap failed (errno %d: %s)",
                       err,
                       strerror(err));
   }

   return VK_SUCCESS;
}

#define pvr_ioctlf(fd, request, arg, error, fmt, args...) \
   ({                                                     \
      VkResult _result = VK_SUCCESS;                      \
      int _ret;                                           \
                                                          \
      _ret = drmIoctl(fd, request, arg);                  \
      if (_ret) {                                         \
         _ret = errno;                                    \
         _result = vk_errorf(NULL,                        \
                             error,                       \
                             fmt " (errno %d: %s)",       \
                             ##args,                      \
                             _ret,                        \
                             strerror(_ret));             \
      }                                                   \
                                                          \
      _result;                                            \
   })

#define pvr_ioctl(fd, request, arg, error) \
   pvr_ioctlf(fd, request, arg, error, "ioctl " #request " failed")

#endif /* PVR_WINSYS_HELPER_H */
