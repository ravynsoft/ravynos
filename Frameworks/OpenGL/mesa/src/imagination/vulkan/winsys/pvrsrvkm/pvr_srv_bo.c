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
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "pvr_private.h"
#include "pvr_srv.h"
#include "pvr_srv_bo.h"
#include "pvr_srv_bridge.h"
#include "pvr_types.h"
#include "pvr_winsys_helper.h"
#include "util/u_atomic.h"
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "vk_log.h"

/* Note: This function does not have an associated pvr_srv_free_display_pmr
 * function, use pvr_srv_free_pmr instead.
 */
static VkResult pvr_srv_alloc_display_pmr(struct pvr_srv_winsys *srv_ws,
                                          uint64_t size,
                                          uint64_t srv_flags,
                                          void **const pmr_out,
                                          uint32_t *const handle_out)
{
   uint64_t aligment_out;
   uint64_t size_out;
   VkResult result;
   uint32_t handle;
   int ret;
   int fd;

   result =
      pvr_winsys_helper_display_buffer_create(&srv_ws->base, size, &handle);
   if (result != VK_SUCCESS)
      return result;

   ret = drmPrimeHandleToFD(srv_ws->base.display_fd, handle, O_CLOEXEC, &fd);
   if (ret) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_display_buffer_destroy;
   }

   result = pvr_srv_physmem_import_dmabuf(srv_ws->base.render_fd,
                                          fd,
                                          srv_flags,
                                          pmr_out,
                                          &size_out,
                                          &aligment_out);

   assert(size_out >= size);
   assert(aligment_out == srv_ws->base.page_size);

   /* close fd, not needed anymore */
   close(fd);

   if (result != VK_SUCCESS)
      goto err_display_buffer_destroy;

   *handle_out = handle;

   return VK_SUCCESS;

err_display_buffer_destroy:
   pvr_winsys_helper_display_buffer_destroy(&srv_ws->base, handle);

   return result;
}

static void buffer_acquire(struct pvr_srv_winsys_bo *srv_bo)
{
   p_atomic_inc(&srv_bo->ref_count);
}

static void buffer_release(struct pvr_srv_winsys_bo *srv_bo)
{
   struct pvr_winsys *ws;

   /* If all references were dropped the pmr can be freed and unlocked */
   if (p_atomic_dec_return(&srv_bo->ref_count) != 0)
      return;

   ws = srv_bo->base.ws;
   pvr_srv_free_pmr(ws->render_fd, srv_bo->pmr);

   if (srv_bo->is_display_buffer)
      pvr_winsys_helper_display_buffer_destroy(ws, srv_bo->handle);

   vk_free(ws->alloc, srv_bo);
}

static uint64_t pvr_srv_get_alloc_flags(uint32_t ws_flags)
{
   /* TODO: For now we assume that buffers should always be accessible to the
    * kernel and that the PVR_WINSYS_BO_FLAG_CPU_ACCESS flag only applies to
    * userspace mappings. Check to see if there's any situations where we
    * wouldn't want this to be the case.
    */
   uint64_t srv_flags =
      PVR_SRV_MEMALLOCFLAG_GPU_READABLE | PVR_SRV_MEMALLOCFLAG_GPU_WRITEABLE |
      PVR_SRV_MEMALLOCFLAG_KERNEL_CPU_MAPPABLE |
      PVR_SRV_MEMALLOCFLAG_CPU_UNCACHED_WC | PVR_SRV_MEMALLOCFLAG_ZERO_ON_ALLOC;

   if (ws_flags & PVR_WINSYS_BO_FLAG_CPU_ACCESS) {
      srv_flags |= PVR_SRV_MEMALLOCFLAG_CPU_READABLE |
                   PVR_SRV_MEMALLOCFLAG_CPU_WRITEABLE;
   }

   if (ws_flags & PVR_WINSYS_BO_FLAG_GPU_UNCACHED)
      srv_flags |= PVR_SRV_MEMALLOCFLAG_GPU_UNCACHED;
   else
      srv_flags |= PVR_SRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT;

   if (ws_flags & PVR_WINSYS_BO_FLAG_PM_FW_PROTECT)
      srv_flags |= PVR_SRV_MEMALLOCFLAG_DEVICE_FLAG(PM_FW_PROTECT);

   return srv_flags;
}

VkResult pvr_srv_winsys_buffer_create(struct pvr_winsys *ws,
                                      uint64_t size,
                                      uint64_t alignment,
                                      enum pvr_winsys_bo_type type,
                                      uint32_t ws_flags,
                                      struct pvr_winsys_bo **const bo_out)
{
   const uint64_t srv_flags = pvr_srv_get_alloc_flags(ws_flags);
   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(ws);
   struct pvr_srv_winsys_bo *srv_bo;
   VkResult result;

   assert(util_is_power_of_two_nonzero(alignment));

   /* Kernel will page align the size, we do the same here so we have access to
    * all the allocated memory.
    */
   alignment = MAX2(alignment, ws->page_size);
   size = ALIGN_POT(size, alignment);

   srv_bo = vk_zalloc(ws->alloc,
                      sizeof(*srv_bo),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!srv_bo)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   srv_bo->is_display_buffer = (type == PVR_WINSYS_BO_TYPE_DISPLAY);
   if (srv_bo->is_display_buffer) {
      result = pvr_srv_alloc_display_pmr(srv_ws,
                                         size,
                                         srv_flags &
                                            PVR_SRV_MEMALLOCFLAGS_PMRFLAGSMASK,
                                         &srv_bo->pmr,
                                         &srv_bo->handle);

      srv_bo->base.is_imported = true;
   } else {
      result =
         pvr_srv_alloc_pmr(ws->render_fd,
                           size,
                           size,
                           1,
                           1,
                           ws->log2_page_size,
                           (srv_flags & PVR_SRV_MEMALLOCFLAGS_PMRFLAGSMASK),
                           getpid(),
                           &srv_bo->pmr);
   }

   if (result != VK_SUCCESS)
      goto err_vk_free_srv_bo;

   srv_bo->base.size = size;
   srv_bo->base.ws = ws;
   srv_bo->flags = srv_flags;

   p_atomic_set(&srv_bo->ref_count, 1);

   *bo_out = &srv_bo->base;

   return VK_SUCCESS;

err_vk_free_srv_bo:
   vk_free(ws->alloc, srv_bo);

   return result;
}

VkResult
pvr_srv_winsys_buffer_create_from_fd(struct pvr_winsys *ws,
                                     int fd,
                                     struct pvr_winsys_bo **const bo_out)
{
   /* FIXME: PVR_SRV_MEMALLOCFLAG_CPU_UNCACHED_WC should be changed to
    * PVR_SRV_MEMALLOCFLAG_CPU_CACHE_INCOHERENT, as dma-buf is always mapped
    * as cacheable by the exporter. Flags are not passed to the exporter and it
    * doesn't really change the behavior, but these can be used for internal
    * checking so it should reflect the correct cachability of the buffer.
    * Ref: pvr_GetMemoryFdPropertiesKHR
    * 	    https://www.kernel.org/doc/html/latest/driver-api/dma-buf.html#c.dma_buf_ops
    */
   static const uint64_t srv_flags =
      PVR_SRV_MEMALLOCFLAG_CPU_READABLE | PVR_SRV_MEMALLOCFLAG_CPU_WRITEABLE |
      PVR_SRV_MEMALLOCFLAG_CPU_UNCACHED_WC | PVR_SRV_MEMALLOCFLAG_GPU_READABLE |
      PVR_SRV_MEMALLOCFLAG_GPU_WRITEABLE |
      PVR_SRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT;
   struct pvr_srv_winsys_bo *srv_bo;
   uint64_t aligment_out;
   uint64_t size_out;
   VkResult result;

   srv_bo = vk_zalloc(ws->alloc,
                      sizeof(*srv_bo),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!srv_bo)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = pvr_srv_physmem_import_dmabuf(ws->render_fd,
                                          fd,
                                          srv_flags,
                                          &srv_bo->pmr,
                                          &size_out,
                                          &aligment_out);
   if (result != VK_SUCCESS)
      goto err_vk_free_srv_bo;

   assert(aligment_out == ws->page_size);

   srv_bo->base.ws = ws;
   srv_bo->base.size = size_out;
   srv_bo->base.is_imported = true;
   srv_bo->flags = srv_flags;

   p_atomic_set(&srv_bo->ref_count, 1);

   *bo_out = &srv_bo->base;

   return VK_SUCCESS;

err_vk_free_srv_bo:
   vk_free(ws->alloc, srv_bo);

   return result;
}

void pvr_srv_winsys_buffer_destroy(struct pvr_winsys_bo *bo)
{
   struct pvr_srv_winsys_bo *srv_bo = to_pvr_srv_winsys_bo(bo);

   buffer_release(srv_bo);
}

VkResult pvr_srv_winsys_buffer_get_fd(struct pvr_winsys_bo *bo,
                                      int *const fd_out)
{
   struct pvr_srv_winsys_bo *srv_bo = to_pvr_srv_winsys_bo(bo);
   struct pvr_winsys *ws = bo->ws;
   int ret;

   if (!srv_bo->is_display_buffer)
      return pvr_srv_physmem_export_dmabuf(ws->render_fd, srv_bo->pmr, fd_out);

   /* For display buffers, export using saved buffer handle */
   ret = drmPrimeHandleToFD(ws->display_fd, srv_bo->handle, O_CLOEXEC, fd_out);
   if (ret)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   return VK_SUCCESS;
}

VkResult pvr_srv_winsys_buffer_map(struct pvr_winsys_bo *bo)
{
   struct pvr_srv_winsys_bo *srv_bo = to_pvr_srv_winsys_bo(bo);
   struct pvr_winsys *ws = bo->ws;
   const int prot =
      (srv_bo->flags & PVR_SRV_MEMALLOCFLAG_CPU_WRITEABLE ? PROT_WRITE : 0) |
      (srv_bo->flags & PVR_SRV_MEMALLOCFLAG_CPU_READABLE ? PROT_READ : 0);
   VkResult result;

   /* assert if memory is already mapped */
   assert(!bo->map);

   /* Map the full PMR to CPU space */
   result = pvr_mmap(bo->size,
                     prot,
                     MAP_SHARED,
                     ws->render_fd,
                     (off_t)srv_bo->pmr << ws->log2_page_size,
                     &bo->map);
   if (result != VK_SUCCESS) {
      bo->map = NULL;
      return result;
   }

   VG(VALGRIND_MALLOCLIKE_BLOCK(bo->map, bo->size, 0, true));

   buffer_acquire(srv_bo);

   return VK_SUCCESS;
}

void pvr_srv_winsys_buffer_unmap(struct pvr_winsys_bo *bo)
{
   struct pvr_srv_winsys_bo *srv_bo = to_pvr_srv_winsys_bo(bo);

   /* output error if trying to unmap memory that is not previously mapped */
   assert(bo->map);

   VG(VALGRIND_FREELIKE_BLOCK(bo->map, 0));

   /* Unmap the whole PMR from CPU space */
   pvr_munmap(bo->map, bo->size);

   bo->map = NULL;

   buffer_release(srv_bo);
}

/* This function must be used to allocate from a heap carveout and must only be
 * used within the winsys code. This also means whoever is using it, must know
 * what they are doing.
 */
VkResult pvr_srv_heap_alloc_carveout(struct pvr_winsys_heap *heap,
                                     const pvr_dev_addr_t carveout_dev_addr,
                                     uint64_t size,
                                     uint64_t alignment,
                                     struct pvr_winsys_vma **const vma_out)
{
   struct pvr_srv_winsys_heap *srv_heap = to_pvr_srv_winsys_heap(heap);
   struct pvr_winsys *ws = heap->ws;
   struct pvr_srv_winsys_vma *srv_vma;
   VkResult result;

   assert(util_is_power_of_two_nonzero(alignment));

   /* pvr_srv_winsys_buffer_create() page aligns the size. We must do the same
    * here to ensure enough heap space is allocated to be able to map the
    * buffer to the GPU.
    */
   alignment = MAX2(alignment, heap->ws->page_size);
   size = ALIGN_POT(size, alignment);

   srv_vma = vk_alloc(ws->alloc,
                      sizeof(*srv_vma),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!srv_vma) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   /* Just check address is correct and aligned, locking is not required as
    * user is responsible to provide a distinct address.
    */
   if (carveout_dev_addr.addr < heap->base_addr.addr ||
       carveout_dev_addr.addr + size >
          heap->base_addr.addr + heap->static_data_carveout_size ||
       carveout_dev_addr.addr & ((ws->page_size) - 1)) {
      result = vk_error(NULL, VK_ERROR_INITIALIZATION_FAILED);
      goto err_vk_free_srv_vma;
   }

   /* Reserve the virtual range in the MMU and create a mapping structure */
   result = pvr_srv_int_reserve_addr(ws->render_fd,
                                     srv_heap->server_heap,
                                     carveout_dev_addr,
                                     size,
                                     &srv_vma->reservation);
   if (result != VK_SUCCESS)
      goto err_vk_free_srv_vma;

   srv_vma->base.dev_addr = carveout_dev_addr;
   srv_vma->base.bo = NULL;
   srv_vma->base.heap = heap;
   srv_vma->base.size = size;

   p_atomic_inc(&srv_heap->base.ref_count);

   *vma_out = &srv_vma->base;

   return VK_SUCCESS;

err_vk_free_srv_vma:
   vk_free(ws->alloc, srv_vma);

err_out:
   return result;
}

VkResult pvr_srv_winsys_heap_alloc(struct pvr_winsys_heap *heap,
                                   uint64_t size,
                                   uint64_t alignment,
                                   struct pvr_winsys_vma **const vma_out)
{
   struct pvr_srv_winsys_heap *const srv_heap = to_pvr_srv_winsys_heap(heap);
   struct pvr_srv_winsys *const srv_ws = to_pvr_srv_winsys(heap->ws);
   struct pvr_srv_winsys_vma *srv_vma;
   VkResult result;

   srv_vma = vk_alloc(srv_ws->base.alloc,
                      sizeof(*srv_vma),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!srv_vma) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   result = pvr_winsys_helper_heap_alloc(heap, size, alignment, &srv_vma->base);
   if (result != VK_SUCCESS)
      goto err_pvr_srv_free_vma;

   /* Reserve the virtual range in the MMU and create a mapping structure. */
   result = pvr_srv_int_reserve_addr(srv_ws->base.render_fd,
                                     srv_heap->server_heap,
                                     srv_vma->base.dev_addr,
                                     srv_vma->base.size,
                                     &srv_vma->reservation);
   if (result != VK_SUCCESS)
      goto err_pvr_srv_free_allocation;

   *vma_out = &srv_vma->base;

   return VK_SUCCESS;

err_pvr_srv_free_allocation:
   pvr_winsys_helper_heap_free(&srv_vma->base);

err_pvr_srv_free_vma:
   vk_free(srv_ws->base.alloc, srv_vma);

err_out:
   return result;
}

void pvr_srv_winsys_heap_free(struct pvr_winsys_vma *vma)
{
   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(vma->heap->ws);
   struct pvr_srv_winsys_vma *srv_vma = to_pvr_srv_winsys_vma(vma);

   /* A vma with an existing device mapping should not be freed. */
   assert(!vma->bo);

   /* Remove mapping handle and underlying reservation. */
   pvr_srv_int_unreserve_addr(srv_ws->base.render_fd, srv_vma->reservation);

   /* Check if we are dealing with carveout address range. */
   if (vma->dev_addr.addr <
       (vma->heap->base_addr.addr + vma->heap->static_data_carveout_size)) {
      /* For the carveout addresses just decrement the reference count. */
      p_atomic_dec(&vma->heap->ref_count);
   } else {
      /* Free allocated virtual space. */
      pvr_winsys_helper_heap_free(vma);
   }

   vk_free(srv_ws->base.alloc, srv_vma);
}

/* * We assume the vma has been allocated with extra space to accommodate the
 *   offset.
 * * The offset passed in is unchanged and can be used to calculate the extra
 *   size that needs to be mapped and final device virtual address.
 */
VkResult pvr_srv_winsys_vma_map(struct pvr_winsys_vma *vma,
                                struct pvr_winsys_bo *bo,
                                uint64_t offset,
                                uint64_t size,
                                pvr_dev_addr_t *const dev_addr_out)
{
   struct pvr_srv_winsys_vma *srv_vma = to_pvr_srv_winsys_vma(vma);
   struct pvr_srv_winsys_bo *srv_bo = to_pvr_srv_winsys_bo(bo);
   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(bo->ws);
   const uint64_t srv_flags = srv_bo->flags &
                              PVR_SRV_MEMALLOCFLAGS_VIRTUAL_MASK;
   const uint32_t virt_offset = offset & (vma->heap->page_size - 1);
   const uint64_t aligned_virt_size =
      ALIGN_POT(virt_offset + size, vma->heap->page_size);
   VkResult result;

   /* Address should not be mapped already */
   assert(!vma->bo);

   if (srv_bo->is_display_buffer) {
      struct pvr_srv_winsys_heap *srv_heap = to_pvr_srv_winsys_heap(vma->heap);

      /* In case of display buffers, we only support to map whole PMR */
      if (offset != 0 || bo->size != ALIGN_POT(size, srv_ws->base.page_size) ||
          vma->size != bo->size) {
         return vk_error(NULL, VK_ERROR_MEMORY_MAP_FAILED);
      }

      /* Map the requested pmr */
      result = pvr_srv_int_map_pmr(srv_ws->base.render_fd,
                                   srv_heap->server_heap,
                                   srv_vma->reservation,
                                   srv_bo->pmr,
                                   srv_flags,
                                   &srv_vma->mapping);

   } else {
      const uint32_t phys_page_offset = (offset - virt_offset) >>
                                        srv_ws->base.log2_page_size;
      const uint32_t phys_page_count = aligned_virt_size >>
                                       srv_ws->base.log2_page_size;

      /* Check if bo and vma can accommodate the given size and offset */
      if (ALIGN_POT(offset + size, vma->heap->page_size) > bo->size ||
          aligned_virt_size > vma->size) {
         return vk_error(NULL, VK_ERROR_MEMORY_MAP_FAILED);
      }

      /* Map the requested pages */
      result = pvr_srv_int_map_pages(srv_ws->base.render_fd,
                                     srv_vma->reservation,
                                     srv_bo->pmr,
                                     phys_page_count,
                                     phys_page_offset,
                                     srv_flags,
                                     vma->dev_addr);
   }

   if (result != VK_SUCCESS)
      return result;

   buffer_acquire(srv_bo);

   vma->bo = bo;
   vma->bo_offset = offset;
   vma->mapped_size = aligned_virt_size;

   if (dev_addr_out)
      *dev_addr_out = PVR_DEV_ADDR_OFFSET(vma->dev_addr, virt_offset);

   return VK_SUCCESS;
}

void pvr_srv_winsys_vma_unmap(struct pvr_winsys_vma *vma)
{
   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(vma->heap->ws);
   struct pvr_srv_winsys_vma *srv_vma = to_pvr_srv_winsys_vma(vma);
   struct pvr_srv_winsys_bo *srv_bo;

   /* Address should be mapped */
   assert(vma->bo);

   srv_bo = to_pvr_srv_winsys_bo(vma->bo);

   if (srv_bo->is_display_buffer) {
      /* Unmap the requested pmr */
      pvr_srv_int_unmap_pmr(srv_ws->base.render_fd, srv_vma->mapping);
   } else {
      /* Unmap requested pages */
      pvr_srv_int_unmap_pages(srv_ws->base.render_fd,
                              srv_vma->reservation,
                              vma->dev_addr,
                              vma->mapped_size >> srv_ws->base.log2_page_size);
   }

   buffer_release(srv_bo);

   vma->bo = NULL;
}
