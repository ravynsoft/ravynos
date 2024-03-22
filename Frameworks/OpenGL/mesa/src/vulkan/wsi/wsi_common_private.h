/*
 * Copyright © 2017 Intel Corporation
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
#ifndef WSI_COMMON_PRIVATE_H
#define WSI_COMMON_PRIVATE_H

#include "wsi_common.h"
#include "util/perf/cpu_trace.h"
#include "vulkan/runtime/vk_object.h"
#include "vulkan/runtime/vk_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wsi_image;
struct wsi_swapchain;

#define WSI_DEBUG_BUFFER      (1ull << 0)
#define WSI_DEBUG_SW          (1ull << 1)
#define WSI_DEBUG_NOSHM       (1ull << 2)
#define WSI_DEBUG_LINEAR      (1ull << 3)
#define WSI_DEBUG_DXGI        (1ull << 4)

extern uint64_t WSI_DEBUG;

enum wsi_image_type {
   WSI_IMAGE_TYPE_CPU,
   WSI_IMAGE_TYPE_DRM,
   WSI_IMAGE_TYPE_DXGI,
};

struct wsi_base_image_params {
   enum wsi_image_type image_type;
};

struct wsi_cpu_image_params {
   struct wsi_base_image_params base;

   uint8_t *(*alloc_shm)(struct wsi_image *image, unsigned size);
};

struct wsi_drm_image_params {
   struct wsi_base_image_params base;

   bool same_gpu;

   uint32_t num_modifier_lists;
   const uint32_t *num_modifiers;
   const uint64_t *const *modifiers;
};

struct wsi_dxgi_image_params {
   struct wsi_base_image_params base;
   bool storage_image;
};

typedef uint32_t (*wsi_memory_type_select_cb)(const struct wsi_device *wsi,
                                              uint32_t type_bits);

struct wsi_image_info {
   VkImageCreateInfo create;
   struct wsi_image_create_info wsi;
   VkExternalMemoryImageCreateInfo ext_mem;
   VkImageFormatListCreateInfo format_list;
   VkImageDrmFormatModifierListCreateInfoEXT drm_mod_list;

   bool prime_use_linear_modifier;

   /* Not really part of VkImageCreateInfo but needed to figure out the
    * number of planes we need to bind.
    */
   uint32_t modifier_prop_count;
   struct VkDrmFormatModifierPropertiesEXT *modifier_props;

   /* For buffer blit images, the linear stride in bytes */
   uint32_t linear_stride;

   /* For buffer blit images, the size of the buffer in bytes */
   uint64_t linear_size;

   wsi_memory_type_select_cb select_image_memory_type;
   wsi_memory_type_select_cb select_blit_dst_memory_type;

   uint8_t *(*alloc_shm)(struct wsi_image *image, unsigned size);

   VkResult (*create_mem)(const struct wsi_swapchain *chain,
                          const struct wsi_image_info *info,
                          struct wsi_image *image);

   VkResult (*finish_create)(const struct wsi_swapchain *chain,
                             const struct wsi_image_info *info,
                             struct wsi_image *image);
};

enum wsi_swapchain_blit_type {
   WSI_SWAPCHAIN_NO_BLIT,
   WSI_SWAPCHAIN_BUFFER_BLIT,
   WSI_SWAPCHAIN_IMAGE_BLIT,
};

struct wsi_image {
   VkImage image;
   VkDeviceMemory memory;

   struct {
      VkBuffer buffer;
      VkImage image;
      VkDeviceMemory memory;
      VkCommandBuffer *cmd_buffers;
   } blit;

#ifndef _WIN32
   uint64_t drm_modifier;
#endif
   int num_planes;
   uint32_t sizes[4];
   uint32_t offsets[4];
   uint32_t row_pitches[4];
#ifndef _WIN32
   int dma_buf_fd;
#endif
   void *cpu_map;
};

struct wsi_swapchain {
   struct vk_object_base base;

   const struct wsi_device *wsi;

   VkDevice device;
   VkAllocationCallbacks alloc;
   VkFence* fences;
   VkPresentModeKHR present_mode;
   VkSemaphore present_id_timeline;

   int signal_dma_buf_from_semaphore;
   VkSemaphore dma_buf_semaphore;

   struct wsi_image_info image_info;
   uint32_t image_count;

   struct {
      enum wsi_swapchain_blit_type type;
      VkSemaphore *semaphores;

      /* If the driver wants to use a special queue to execute the buffer blit,
       * it'll implement the wsi_device::get_blit_queue callback.
       * The created queue will be stored here and will be used to execute the
       * buffer blit instead of using the present queue.
       */
      VkQueue queue;
   } blit;

   bool capture_key_pressed;

   /* Command pools, one per queue family */
   VkCommandPool *cmd_pools;

   VkResult (*destroy)(struct wsi_swapchain *swapchain,
                       const VkAllocationCallbacks *pAllocator);
   struct wsi_image *(*get_wsi_image)(struct wsi_swapchain *swapchain,
                                      uint32_t image_index);
   VkResult (*acquire_next_image)(struct wsi_swapchain *swap_chain,
                                  const VkAcquireNextImageInfoKHR *info,
                                  uint32_t *image_index);
   VkResult (*queue_present)(struct wsi_swapchain *swap_chain,
                             uint32_t image_index,
                             uint64_t present_id,
                             const VkPresentRegionKHR *damage);
   VkResult (*wait_for_present)(struct wsi_swapchain *swap_chain,
                                uint64_t present_id,
                                uint64_t timeout);
   VkResult (*release_images)(struct wsi_swapchain *swap_chain,
                              uint32_t count,
                              const uint32_t *indices);
   void (*set_present_mode)(struct wsi_swapchain *swap_chain,
                            VkPresentModeKHR mode);
};

bool
wsi_device_matches_drm_fd(const struct wsi_device *wsi, int drm_fd);

void
wsi_wl_surface_destroy(VkIcdSurfaceBase *icd_surface, VkInstance _instance,
                       const VkAllocationCallbacks *pAllocator);

void
wsi_win32_surface_destroy(VkIcdSurfaceBase *icd_surface, VkInstance _instance,
                          const VkAllocationCallbacks *pAllocator);

VkResult
wsi_swapchain_init(const struct wsi_device *wsi,
                   struct wsi_swapchain *chain,
                   VkDevice device,
                   const VkSwapchainCreateInfoKHR *pCreateInfo,
                   const struct wsi_base_image_params *image_params,
                   const VkAllocationCallbacks *pAllocator);

enum VkPresentModeKHR
wsi_swapchain_get_present_mode(struct wsi_device *wsi,
                               const VkSwapchainCreateInfoKHR *pCreateInfo);

void wsi_swapchain_finish(struct wsi_swapchain *chain);

uint32_t
wsi_select_memory_type(const struct wsi_device *wsi,
                       VkMemoryPropertyFlags req_flags,
                       VkMemoryPropertyFlags deny_flags,
                       uint32_t type_bits);
uint32_t
wsi_select_device_memory_type(const struct wsi_device *wsi,
                              uint32_t type_bits);

bool
wsi_drm_image_needs_buffer_blit(const struct wsi_device *wsi,
                                const struct wsi_drm_image_params *params);

enum wsi_swapchain_blit_type
wsi_dxgi_image_needs_blit(const struct wsi_device *wsi,
                          const struct wsi_dxgi_image_params *params,
                          VkDevice device);

VkResult
wsi_drm_configure_image(const struct wsi_swapchain *chain,
                        const VkSwapchainCreateInfoKHR *pCreateInfo,
                        const struct wsi_drm_image_params *params,
                        struct wsi_image_info *info);

VkResult
wsi_dxgi_configure_image(const struct wsi_swapchain *chain,
                         const VkSwapchainCreateInfoKHR *pCreateInfo,
                         const struct wsi_dxgi_image_params *params,
                         struct wsi_image_info *info);

bool
wsi_cpu_image_needs_buffer_blit(const struct wsi_device *wsi,
                                const struct wsi_cpu_image_params *params);

VkResult
wsi_configure_cpu_image(const struct wsi_swapchain *chain,
                        const VkSwapchainCreateInfoKHR *pCreateInfo,
                        const struct wsi_cpu_image_params *params,
                        struct wsi_image_info *info);

VkResult
wsi_create_buffer_blit_context(const struct wsi_swapchain *chain,
                               const struct wsi_image_info *info,
                               struct wsi_image *image,
                               VkExternalMemoryHandleTypeFlags handle_types,
                               bool implicit_sync);

VkResult
wsi_finish_create_blit_context(const struct wsi_swapchain *chain,
                               const struct wsi_image_info *info,
                               struct wsi_image *image);

void
wsi_configure_buffer_image(UNUSED const struct wsi_swapchain *chain,
                           const VkSwapchainCreateInfoKHR *pCreateInfo,
                           uint32_t stride_align, uint32_t size_align,
                           struct wsi_image_info *info);

void
wsi_configure_image_blit_image(UNUSED const struct wsi_swapchain *chain,
                               struct wsi_image_info *info);

VkResult
wsi_configure_image(const struct wsi_swapchain *chain,
                    const VkSwapchainCreateInfoKHR *pCreateInfo,
                    VkExternalMemoryHandleTypeFlags handle_types,
                    struct wsi_image_info *info);
void
wsi_destroy_image_info(const struct wsi_swapchain *chain,
                       struct wsi_image_info *info);
VkResult
wsi_create_image(const struct wsi_swapchain *chain,
                 const struct wsi_image_info *info,
                 struct wsi_image *image);
void
wsi_image_init(struct wsi_image *image);

void
wsi_destroy_image(const struct wsi_swapchain *chain,
                  struct wsi_image *image);

VkResult
wsi_swapchain_wait_for_present_semaphore(const struct wsi_swapchain *chain,
                                         uint64_t present_id, uint64_t timeout);

#ifdef HAVE_LIBDRM
VkResult
wsi_prepare_signal_dma_buf_from_semaphore(struct wsi_swapchain *chain,
                                          const struct wsi_image *image);
VkResult
wsi_signal_dma_buf_from_semaphore(const struct wsi_swapchain *chain,
                                  const struct wsi_image *image);
VkResult
wsi_create_sync_for_dma_buf_wait(const struct wsi_swapchain *chain,
                                 const struct wsi_image *image,
                                 enum vk_sync_features sync_features,
                                 struct vk_sync **sync_out);
#endif

struct wsi_interface {
   VkResult (*get_support)(VkIcdSurfaceBase *surface,
                           struct wsi_device *wsi_device,
                           uint32_t queueFamilyIndex,
                           VkBool32* pSupported);
   VkResult (*get_capabilities2)(VkIcdSurfaceBase *surface,
                                 struct wsi_device *wsi_device,
                                 const void *info_next,
                                 VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
   VkResult (*get_formats)(VkIcdSurfaceBase *surface,
                           struct wsi_device *wsi_device,
                           uint32_t* pSurfaceFormatCount,
                           VkSurfaceFormatKHR* pSurfaceFormats);
   VkResult (*get_formats2)(VkIcdSurfaceBase *surface,
                            struct wsi_device *wsi_device,
                            const void *info_next,
                            uint32_t* pSurfaceFormatCount,
                            VkSurfaceFormat2KHR* pSurfaceFormats);
   VkResult (*get_present_modes)(VkIcdSurfaceBase *surface,
                                 struct wsi_device *wsi_device,
                                 uint32_t* pPresentModeCount,
                                 VkPresentModeKHR* pPresentModes);
   VkResult (*get_present_rectangles)(VkIcdSurfaceBase *surface,
                                      struct wsi_device *wsi_device,
                                      uint32_t* pRectCount,
                                      VkRect2D* pRects);
   VkResult (*create_swapchain)(VkIcdSurfaceBase *surface,
                                VkDevice device,
                                struct wsi_device *wsi_device,
                                const VkSwapchainCreateInfoKHR* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator,
                                struct wsi_swapchain **swapchain);
};

VkResult wsi_x11_init_wsi(struct wsi_device *wsi_device,
                          const VkAllocationCallbacks *alloc,
                          const struct driOptionCache *dri_options);
void wsi_x11_finish_wsi(struct wsi_device *wsi_device,
                        const VkAllocationCallbacks *alloc);
VkResult wsi_wl_init_wsi(struct wsi_device *wsi_device,
                         const VkAllocationCallbacks *alloc,
                         VkPhysicalDevice physical_device);
void wsi_wl_finish_wsi(struct wsi_device *wsi_device,
                       const VkAllocationCallbacks *alloc);
VkResult wsi_win32_init_wsi(struct wsi_device *wsi_device,
                         const VkAllocationCallbacks *alloc,
                         VkPhysicalDevice physical_device);
void wsi_win32_finish_wsi(struct wsi_device *wsi_device,
                       const VkAllocationCallbacks *alloc);


VkResult
wsi_display_init_wsi(struct wsi_device *wsi_device,
                     const VkAllocationCallbacks *alloc,
                     int display_fd);

void
wsi_display_finish_wsi(struct wsi_device *wsi_device,
                       const VkAllocationCallbacks *alloc);

void
wsi_display_setup_syncobj_fd(struct wsi_device *wsi_device,
                             int fd);

VkResult wsi_headless_init_wsi(struct wsi_device *wsi_device,
                               const VkAllocationCallbacks *alloc,
                               VkPhysicalDevice physical_device);

void wsi_headless_finish_wsi(struct wsi_device *wsi_device,
                             const VkAllocationCallbacks *alloc);

VK_DEFINE_NONDISP_HANDLE_CASTS(wsi_swapchain, base, VkSwapchainKHR,
                               VK_OBJECT_TYPE_SWAPCHAIN_KHR)

#if defined(HAVE_PTHREAD) && !defined(_WIN32)
bool
wsi_init_pthread_cond_monotonic(pthread_cond_t *cond);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WSI_COMMON_PRIVATE_H */
