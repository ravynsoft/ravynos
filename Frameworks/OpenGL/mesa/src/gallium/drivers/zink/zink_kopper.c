/*
 * Copyright 2020 Red Hat, Inc.
 * Copyright © 2021 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "util/detect_os.h"
#include "driver_trace/tr_screen.h"

#include "zink_context.h"
#include "zink_screen.h"
#include "zink_surface.h"
#include "zink_resource.h"
#include "zink_kopper.h"

static void
zink_kopper_set_present_mode_for_interval(struct kopper_displaytarget *cdt, int interval)
{
#if DETECT_OS_WINDOWS
    // not hooked up yet so let's not sabotage benchmarks
    cdt->present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
#else
   assert(interval >= 0); /* TODO: VK_PRESENT_MODE_FIFO_RELAXED_KHR */
   if (interval == 0) {
      if (cdt->present_modes & BITFIELD_BIT(VK_PRESENT_MODE_IMMEDIATE_KHR))
         cdt->present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      else
         cdt->present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
   } else if (interval > 0) {
      cdt->present_mode = VK_PRESENT_MODE_FIFO_KHR;
   }
   assert(cdt->present_modes & BITFIELD_BIT(cdt->present_mode));
#endif
}

static void
init_dt_type(struct kopper_displaytarget *cdt)
{
   VkStructureType type = cdt->info.bos.sType;
   switch (type) {
#ifdef VK_USE_PLATFORM_XCB_KHR
   case VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR:
      cdt->type = KOPPER_X11;
      break;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   case VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR:
      cdt->type = KOPPER_WAYLAND;
      break;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   case VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR:
      cdt->type = KOPPER_WIN32;
      break;
#endif
   default:
      unreachable("unsupported!");
   }
}

static VkSurfaceKHR
kopper_CreateSurface(struct zink_screen *screen, struct kopper_displaytarget *cdt)
{
   VkSurfaceKHR surface = VK_NULL_HANDLE;
   VkResult error = VK_SUCCESS;

   init_dt_type(cdt);
   VkStructureType type = cdt->info.bos.sType;
   switch (type) {
#ifdef VK_USE_PLATFORM_XCB_KHR
   case VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR: {
      VkXcbSurfaceCreateInfoKHR *xcb = (VkXcbSurfaceCreateInfoKHR *)&cdt->info.bos;
      error = VKSCR(CreateXcbSurfaceKHR)(screen->instance, xcb, NULL, &surface);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   case VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR: {
      VkWaylandSurfaceCreateInfoKHR *wlsci = (VkWaylandSurfaceCreateInfoKHR *)&cdt->info.bos;
      error = VKSCR(CreateWaylandSurfaceKHR)(screen->instance, wlsci, NULL, &surface);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   case VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR: {
      VkWin32SurfaceCreateInfoKHR *win32 = (VkWin32SurfaceCreateInfoKHR *)&cdt->info.bos;
      error = VKSCR(CreateWin32SurfaceKHR)(screen->instance, win32, NULL, &surface);
      break;
   }
#endif
   default:
      unreachable("unsupported!");
   }
   if (error != VK_SUCCESS) {
      return VK_NULL_HANDLE;
   }

   VkBool32 supported;
   error = VKSCR(GetPhysicalDeviceSurfaceSupportKHR)(screen->pdev, screen->gfx_queue, surface, &supported);
   if (!zink_screen_handle_vkresult(screen, error) || !supported)
      goto fail;

   unsigned count = 10;
   VkPresentModeKHR modes[10];
   error = VKSCR(GetPhysicalDeviceSurfacePresentModesKHR)(screen->pdev, surface, &count, modes);
   if (!zink_screen_handle_vkresult(screen, error))
      goto fail;

   for (unsigned i = 0; i < count; i++) {
      /* VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR and VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
      * are not handled
      */
      assert(modes[i] <= VK_PRESENT_MODE_FIFO_RELAXED_KHR);
      if (modes[i] <= VK_PRESENT_MODE_FIFO_RELAXED_KHR)
         cdt->present_modes |= BITFIELD_BIT(modes[i]);
   }

   zink_kopper_set_present_mode_for_interval(cdt, cdt->info.initial_swap_interval);

   return surface;
fail:
   VKSCR(DestroySurfaceKHR)(screen->instance, surface, NULL);
   return VK_NULL_HANDLE;
}

static void
destroy_swapchain(struct zink_screen *screen, struct kopper_swapchain *cswap)
{
   if (!cswap)
      return;
   util_queue_fence_destroy(&cswap->present_fence);
   for (unsigned i = 0; i < cswap->num_images; i++) {
      simple_mtx_lock(&screen->semaphores_lock);
      util_dynarray_append(&screen->semaphores, VkSemaphore, cswap->images[i].acquire);
      simple_mtx_unlock(&screen->semaphores_lock);
      pipe_resource_reference(&cswap->images[i].readback, NULL);
   }
   free(cswap->images);
   hash_table_foreach(cswap->presents, he) {
      struct util_dynarray *arr = he->data;
      simple_mtx_lock(&screen->semaphores_lock);
      util_dynarray_append_dynarray(&screen->semaphores, arr);
      simple_mtx_unlock(&screen->semaphores_lock);
      util_dynarray_fini(arr);
      free(arr);
   }
   _mesa_hash_table_destroy(cswap->presents, NULL);
   VKSCR(DestroySwapchainKHR)(screen->dev, cswap->swapchain, NULL);
   free(cswap);
}

static void
prune_old_swapchains(struct zink_screen *screen, struct kopper_displaytarget *cdt, bool wait)
{
   while (cdt->old_swapchain) {
      struct kopper_swapchain *cswap = cdt->old_swapchain;
      if (cswap->async_presents) {
         if (wait)
            continue;
         return;
      }
      struct zink_batch_usage *u = cswap->batch_uses;
      if (!zink_screen_usage_check_completion(screen, u)) {
         /* these can't ever be pruned */
         if (!wait || zink_batch_usage_is_unflushed(u))
            return;

         zink_screen_timeline_wait(screen, u->usage, UINT64_MAX);
         cswap->batch_uses = NULL;
      }
      cdt->old_swapchain = cswap->next;
      destroy_swapchain(screen, cswap);
   }
}

static struct hash_entry *
find_dt_entry(struct zink_screen *screen, const struct kopper_displaytarget *cdt)
{
   struct hash_entry *he = NULL;
   switch (cdt->type) {
#ifdef VK_USE_PLATFORM_XCB_KHR
   case KOPPER_X11: {
      VkXcbSurfaceCreateInfoKHR *xcb = (VkXcbSurfaceCreateInfoKHR *)&cdt->info.bos;
      he = _mesa_hash_table_search_pre_hashed(&screen->dts, xcb->window, (void*)(uintptr_t)xcb->window);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   case KOPPER_WAYLAND: {
      VkWaylandSurfaceCreateInfoKHR *wlsci = (VkWaylandSurfaceCreateInfoKHR *)&cdt->info.bos;
      he = _mesa_hash_table_search(&screen->dts, wlsci->surface);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   case KOPPER_WIN32: {
      VkWin32SurfaceCreateInfoKHR *win32 = (VkWin32SurfaceCreateInfoKHR *)&cdt->info.bos;
      he = _mesa_hash_table_search(&screen->dts, win32->hwnd);
      break;
   }
#endif
   default:
      unreachable("unsupported!");
   }
   return he;
}

void
zink_kopper_deinit_displaytarget(struct zink_screen *screen, struct kopper_displaytarget *cdt)
{
   if (!cdt->surface)
      return;
   simple_mtx_lock(&screen->dt_lock);
   struct hash_entry *he = find_dt_entry(screen, cdt);
   assert(he);
   /* this deinits the registered entry, which should always be the "right" entry */
   cdt = he->data;
   _mesa_hash_table_remove(&screen->dts, he);
   simple_mtx_unlock(&screen->dt_lock);
   destroy_swapchain(screen, cdt->swapchain);
   prune_old_swapchains(screen, cdt, true);
   VKSCR(DestroySurfaceKHR)(screen->instance, cdt->surface, NULL);
   cdt->swapchain = cdt->old_swapchain = NULL;
   cdt->surface = VK_NULL_HANDLE;
}

static struct kopper_swapchain *
kopper_CreateSwapchain(struct zink_screen *screen, struct kopper_displaytarget *cdt, unsigned w, unsigned h, VkResult *result)
{
   VkResult error = VK_SUCCESS;
   struct kopper_swapchain *cswap = CALLOC_STRUCT(kopper_swapchain);
   if (!cswap) {
      *result = VK_ERROR_OUT_OF_HOST_MEMORY;
      return NULL;
   }
   cswap->last_present_prune = 1;
   util_queue_fence_init(&cswap->present_fence);

   bool has_alpha = cdt->info.has_alpha && (cdt->caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
   if (cdt->swapchain) {
      cswap->scci = cdt->swapchain->scci;
      /* avoid UAF if async present needs to-be-retired swapchain */
      if (cdt->type == KOPPER_WAYLAND && cdt->swapchain->swapchain)
         util_queue_fence_wait(&cdt->swapchain->present_fence);
      cswap->scci.oldSwapchain = cdt->swapchain->swapchain;
   } else {
      cswap->scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      cswap->scci.pNext = NULL;
      cswap->scci.surface = cdt->surface;
      cswap->scci.flags = zink_kopper_has_srgb(cdt) ? VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR : 0;
      cswap->scci.imageFormat = cdt->formats[0];
      cswap->scci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      // TODO: This is where you'd hook up stereo
      cswap->scci.imageArrayLayers = 1;
      cswap->scci.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
      if (cdt->caps.supportedUsageFlags & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)
         cswap->scci.imageUsage |= VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      cswap->scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      cswap->scci.queueFamilyIndexCount = 0;
      cswap->scci.pQueueFamilyIndices = NULL;
      cswap->scci.compositeAlpha = has_alpha ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      cswap->scci.clipped = VK_TRUE;
   }
   cswap->scci.presentMode = cdt->present_mode;
   cswap->scci.minImageCount = cdt->caps.minImageCount;
   cswap->scci.preTransform = cdt->caps.currentTransform;
   if (cdt->formats[1])
      cswap->scci.pNext = &cdt->format_list;

   /* different display platforms have, by vulkan spec, different sizing methodologies */
   switch (cdt->type) {
   case KOPPER_X11:
   case KOPPER_WIN32:
      /* With Xcb, minImageExtent, maxImageExtent, and currentExtent must always equal the window size.
       * ...
       * Due to above restrictions, it is only possible to create a new swapchain on this
       * platform with imageExtent being equal to the current size of the window.
       */
      cswap->scci.imageExtent.width = cdt->caps.currentExtent.width;
      cswap->scci.imageExtent.height = cdt->caps.currentExtent.height;
      break;
   case KOPPER_WAYLAND:
      /* On Wayland, currentExtent is the special value (0xFFFFFFFF, 0xFFFFFFFF), indicating that the
       * surface size will be determined by the extent of a swapchain targeting the surface. Whatever the
       * application sets a swapchain’s imageExtent to will be the size of the window, after the first image is
       * presented.
       */
      cswap->scci.imageExtent.width = w;
      cswap->scci.imageExtent.height = h;
      break;
   default:
      unreachable("unknown display platform");
   }

   error = VKSCR(CreateSwapchainKHR)(screen->dev, &cswap->scci, NULL,
                                &cswap->swapchain);
   if (error == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
      if (util_queue_is_initialized(&screen->flush_queue))
         util_queue_finish(&screen->flush_queue);
      simple_mtx_lock(&screen->queue_lock);
      VkResult result = VKSCR(QueueWaitIdle)(screen->queue);
      simple_mtx_unlock(&screen->queue_lock);
      if (result != VK_SUCCESS)
         mesa_loge("ZINK: vkQueueWaitIdle failed (%s)", vk_Result_to_str(result));
      error = VKSCR(CreateSwapchainKHR)(screen->dev, &cswap->scci, NULL,
                                   &cswap->swapchain);
   }
   if (error != VK_SUCCESS) {
       mesa_loge("CreateSwapchainKHR failed with %s\n", vk_Result_to_str(error));
       free(cswap);
       *result = error;
       return NULL;
   }
   cswap->last_present = UINT32_MAX;

   *result = VK_SUCCESS;
   return cswap;
}

static VkResult
kopper_GetSwapchainImages(struct zink_screen *screen, struct kopper_swapchain *cswap)
{
   VkResult error = VKSCR(GetSwapchainImagesKHR)(screen->dev, cswap->swapchain, &cswap->num_images, NULL);
   zink_screen_handle_vkresult(screen, error);
   if (error != VK_SUCCESS)
      return error;
   cswap->images = calloc(cswap->num_images, sizeof(struct kopper_swapchain_image));
   if (!cswap->images) {
      mesa_loge("ZINK: failed to allocate cswap->images!");
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   cswap->presents = _mesa_hash_table_create_u32_keys(NULL);
   VkImage images[32];
   error = VKSCR(GetSwapchainImagesKHR)(screen->dev, cswap->swapchain, &cswap->num_images, images);
   assert(cswap->num_images <= ARRAY_SIZE(images));
   if (zink_screen_handle_vkresult(screen, error)) {
      for (unsigned i = 0; i < cswap->num_images; i++)
         cswap->images[i].image = images[i];
   }
   cswap->max_acquires = cswap->num_images - cswap->scci.minImageCount + 1;
   return error;
}

static VkResult
update_caps(struct zink_screen *screen, struct kopper_displaytarget *cdt)
{
   VkResult error = VKSCR(GetPhysicalDeviceSurfaceCapabilitiesKHR)(screen->pdev, cdt->surface, &cdt->caps);
   zink_screen_handle_vkresult(screen, error);
   return error;
}

static VkResult
update_swapchain(struct zink_screen *screen, struct kopper_displaytarget *cdt, unsigned w, unsigned h)
{
   VkResult error = update_caps(screen, cdt);
   if (error != VK_SUCCESS)
      return error;
   struct kopper_swapchain *cswap = kopper_CreateSwapchain(screen, cdt, w, h, &error);
   if (!cswap)
      return error;
   prune_old_swapchains(screen, cdt, false);
   struct kopper_swapchain **pswap = &cdt->old_swapchain;
   while (*pswap)
      *pswap = (*pswap)->next;
   *pswap = cdt->swapchain;
   cdt->swapchain = cswap;

   return kopper_GetSwapchainImages(screen, cdt->swapchain);
}

struct kopper_displaytarget *
zink_kopper_displaytarget_create(struct zink_screen *screen, unsigned tex_usage,
                                 enum pipe_format format, unsigned width,
                                 unsigned height, unsigned alignment,
                                 const void *loader_private, unsigned *stride)
{
   struct kopper_displaytarget *cdt;
   const struct kopper_loader_info *info = loader_private;

   {
      struct kopper_displaytarget k;
      struct hash_entry *he = NULL;
      k.info = *info;
      init_dt_type(&k);
      simple_mtx_lock(&screen->dt_lock);
      if (unlikely(!screen->dts.table)) {
         switch (k.type) {
         case KOPPER_X11:
            _mesa_hash_table_init(&screen->dts, screen, NULL, _mesa_key_pointer_equal);
            break;
         case KOPPER_WAYLAND:
         case KOPPER_WIN32:
            _mesa_hash_table_init(&screen->dts, screen, _mesa_hash_pointer, _mesa_key_pointer_equal);
            break;
         default:
            unreachable("unknown kopper type");
         }
      } else {
         he = find_dt_entry(screen, &k);
      }
      simple_mtx_unlock(&screen->dt_lock);
      if (he) {
         cdt = he->data;
         p_atomic_inc(&cdt->refcount);
         *stride = cdt->stride;
         return cdt;
      }
   }

   cdt = CALLOC_STRUCT(kopper_displaytarget);
   if (!cdt)
      return NULL;

   cdt->refcount = 1;
   cdt->loader_private = (void*)loader_private;
   cdt->info = *info;

   enum pipe_format srgb = PIPE_FORMAT_NONE;
   if (screen->info.have_KHR_swapchain_mutable_format) {
      srgb = util_format_is_srgb(format) ? util_format_linear(format) : util_format_srgb(format);
      /* why do these helpers have different default return values? */
      if (srgb == format)
         srgb = PIPE_FORMAT_NONE;
   }
   cdt->formats[0] = zink_get_format(screen, format);
   if (srgb) {
      cdt->format_list.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
      cdt->format_list.pNext = NULL;
      cdt->format_list.viewFormatCount = 2;
      cdt->format_list.pViewFormats = cdt->formats;

      cdt->formats[1] = zink_get_format(screen, srgb);
   }

   cdt->surface = kopper_CreateSurface(screen, cdt);
   if (!cdt->surface)
      goto out;

   if (update_swapchain(screen, cdt, width, height) != VK_SUCCESS)
      goto out;

   simple_mtx_lock(&screen->dt_lock);
   switch (cdt->type) {
#ifdef VK_USE_PLATFORM_XCB_KHR
   case KOPPER_X11: {
      VkXcbSurfaceCreateInfoKHR *xcb = (VkXcbSurfaceCreateInfoKHR *)&cdt->info.bos;
      _mesa_hash_table_insert_pre_hashed(&screen->dts, xcb->window, (void*)(uintptr_t)xcb->window, cdt);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   case KOPPER_WAYLAND: {
      VkWaylandSurfaceCreateInfoKHR *wlsci = (VkWaylandSurfaceCreateInfoKHR *)&cdt->info.bos;
      _mesa_hash_table_insert(&screen->dts, wlsci->surface, cdt);
      break;
   }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   case KOPPER_WIN32: {
      VkWin32SurfaceCreateInfoKHR *win32 = (VkWin32SurfaceCreateInfoKHR *)&cdt->info.bos;
      _mesa_hash_table_insert(&screen->dts, win32->hwnd, cdt);
      break;
   }
#endif
   default:
      unreachable("unsupported!");
   }
   simple_mtx_unlock(&screen->dt_lock);

   *stride = cdt->stride;
   return cdt;

//moar cleanup
out:
   FREE(cdt);
   return NULL;
}

void
zink_kopper_displaytarget_destroy(struct zink_screen *screen, struct kopper_displaytarget *cdt)
{
   if (!p_atomic_dec_zero(&cdt->refcount))
      return;
   zink_kopper_deinit_displaytarget(screen, cdt);
   FREE(cdt);
}

static VkResult
kopper_acquire(struct zink_screen *screen, struct zink_resource *res, uint64_t timeout)
{
   struct kopper_displaytarget *cdt = res->obj->dt;

   /* if:
    * - we don't need a new image
    * - we have a swapchain image
    * - that image is either acquired or acquiring
    *
    * then this is a no-op
    */
   if (!res->obj->new_dt && res->obj->dt_idx != UINT32_MAX &&
       (cdt->swapchain->images[res->obj->dt_idx].acquire || cdt->swapchain->images[res->obj->dt_idx].acquired))
      return VK_SUCCESS;
   VkSemaphore acquire = VK_NULL_HANDLE;

   while (true) {
      if (res->obj->new_dt) {
         VkResult error = update_swapchain(screen, cdt, res->base.b.width0, res->base.b.height0);
         zink_screen_handle_vkresult(screen, error);
         if (error != VK_SUCCESS)
            return error;
         res->obj->new_dt = false;
         res->layout = VK_IMAGE_LAYOUT_UNDEFINED;
         res->obj->access = 0;
         res->obj->access_stage = 0;
      }
      if (timeout == UINT64_MAX && util_queue_is_initialized(&screen->flush_queue) &&
          p_atomic_read_relaxed(&cdt->swapchain->num_acquires) >= cdt->swapchain->max_acquires) {
         util_queue_fence_wait(&cdt->swapchain->present_fence);
      }
      VkResult ret;
      if (!acquire) {
         acquire = zink_create_semaphore(screen);
         assert(acquire);
         if (!acquire)
            return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
      ret = VKSCR(AcquireNextImageKHR)(screen->dev, cdt->swapchain->swapchain, timeout, acquire, VK_NULL_HANDLE, &res->obj->dt_idx);
      if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR) {
         if (ret == VK_ERROR_OUT_OF_DATE_KHR) {
            res->obj->new_dt = true;
            continue;
         }
         VKSCR(DestroySemaphore)(screen->dev, acquire, NULL);
         return ret;
      }
      break;
   }

   cdt->swapchain->images[res->obj->dt_idx].acquire = acquire;
   if (cdt->swapchain->images[res->obj->dt_idx].readback)
      zink_resource(cdt->swapchain->images[res->obj->dt_idx].readback)->valid = false;
   res->obj->image = cdt->swapchain->images[res->obj->dt_idx].image;
   if (!cdt->age_locked)
      zink_kopper_update_last_written(res);
   cdt->swapchain->images[res->obj->dt_idx].acquired = false;
   if (!cdt->swapchain->images[res->obj->dt_idx].init) {
      /* swapchain images are initially in the UNDEFINED layout */
      res->layout = VK_IMAGE_LAYOUT_UNDEFINED;
      cdt->swapchain->images[res->obj->dt_idx].init = true;
   }
   if (timeout == UINT64_MAX) {
      res->obj->indefinite_acquire = true;
      p_atomic_inc(&cdt->swapchain->num_acquires);
   }
   cdt->swapchain->images[res->obj->dt_idx].dt_has_data = false;
   return VK_SUCCESS;
}

static void
kill_swapchain(struct zink_context *ctx, struct zink_resource *res)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   /* dead swapchain */
   mesa_loge("zink: swapchain killed %p\n", res);
   zink_batch_reference_resource(&ctx->batch, res);
   struct pipe_resource *pres = screen->base.resource_create(&screen->base, &res->base.b);
   zink_resource_object_reference(screen, &res->obj, zink_resource(pres)->obj);
   res->layout = VK_IMAGE_LAYOUT_UNDEFINED;
   res->swapchain = false;
   pipe_resource_reference(&pres, NULL);
}

static bool
is_swapchain_kill(VkResult ret)
{
   return ret != VK_SUCCESS &&
          ret != VK_TIMEOUT &&
          ret != VK_NOT_READY &&
          ret != VK_SUBOPTIMAL_KHR;
}

bool
zink_kopper_acquire(struct zink_context *ctx, struct zink_resource *res, uint64_t timeout)
{
   assert(zink_is_swapchain(res));
   struct kopper_displaytarget *cdt = res->obj->dt;
   if (!cdt)
      /* dead swapchain */
      return false;
   if (cdt->is_kill) {
      kill_swapchain(ctx, res);
      return false;
   }
   const struct kopper_swapchain *cswap = cdt->swapchain;
   res->obj->new_dt |= res->base.b.width0 != cswap->scci.imageExtent.width ||
                       res->base.b.height0 != cswap->scci.imageExtent.height;
   VkResult ret = kopper_acquire(zink_screen(trace_screen_unwrap(ctx->base.screen)), res, timeout);
   if (ret == VK_SUCCESS || ret == VK_SUBOPTIMAL_KHR) {
      if (cswap != cdt->swapchain) {
         ctx->swapchain_size = cdt->swapchain->scci.imageExtent;
         res->base.b.width0 = ctx->swapchain_size.width;
         res->base.b.height0 = ctx->swapchain_size.height;
      }
   } else if (is_swapchain_kill(ret)) {
      kill_swapchain(ctx, res);
   }
   bool is_kill = is_swapchain_kill(ret);
   zink_batch_usage_set(&cdt->swapchain->batch_uses, ctx->batch.state);
   return !is_kill;
}

VkSemaphore
zink_kopper_acquire_submit(struct zink_screen *screen, struct zink_resource *res)
{
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;
   assert(res->obj->dt_idx != UINT32_MAX);
   if (cdt->swapchain->images[res->obj->dt_idx].dt_has_data)
      return VK_NULL_HANDLE;
   assert(res->obj->dt_idx != UINT32_MAX);
   if (cdt->swapchain->images[res->obj->dt_idx].acquired) {
      assert(!cdt->swapchain->images[res->obj->dt_idx].acquire);
      return VK_NULL_HANDLE;
   }
   assert(cdt->swapchain->images[res->obj->dt_idx].acquire);
   cdt->swapchain->images[res->obj->dt_idx].acquired = true;
   /* this is now owned by the batch */
   VkSemaphore acquire = cdt->swapchain->images[res->obj->dt_idx].acquire;
   cdt->swapchain->images[res->obj->dt_idx].acquire = VK_NULL_HANDLE;
   cdt->swapchain->images[res->obj->dt_idx].dt_has_data = true;
   return acquire;
}

VkSemaphore
zink_kopper_present(struct zink_screen *screen, struct zink_resource *res)
{
   assert(res->obj->dt);
   assert(!res->obj->present);
   assert(zink_kopper_acquired(res->obj->dt, res->obj->dt_idx));
   res->obj->present = zink_create_semaphore(screen);
   return res->obj->present;
}

struct kopper_present_info {
   VkPresentInfoKHR info;
   uint32_t image;
   struct kopper_swapchain *swapchain;
   struct zink_resource *res;
   VkSemaphore sem;
   bool indefinite_acquire;
};

static void
kopper_present(void *data, void *gdata, int thread_idx)
{
   struct kopper_present_info *cpi = data;
   struct kopper_displaytarget *cdt = cpi->res->obj->dt;
   struct kopper_swapchain *swapchain = cpi->swapchain;
   struct zink_screen *screen = gdata;
   VkResult error = VK_SUCCESS;
   cpi->info.pResults = &error;

   simple_mtx_lock(&screen->queue_lock);
   if (screen->driver_workarounds.implicit_sync && cdt->type != KOPPER_WIN32) {
      if (!screen->fence) {
         VkFenceCreateInfo fci = {0};
         fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
         VKSCR(CreateFence)(screen->dev, &fci, NULL, &screen->fence);
      }
      VKSCR(ResetFences)(screen->dev, 1, &screen->fence);
      VkSubmitInfo si = {0};
      si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      si.waitSemaphoreCount = 1;
      si.pWaitSemaphores = cpi->info.pWaitSemaphores;
      VkPipelineStageFlags stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      si.pWaitDstStageMask = &stages;

      error = VKSCR(QueueSubmit)(screen->queue, 1, &si, screen->fence);
      if (!zink_screen_handle_vkresult(screen, error)) {
         simple_mtx_unlock(&screen->queue_lock);
         VKSCR(DestroySemaphore)(screen->dev, cpi->sem, NULL);
         goto out;
      }
      error = VKSCR(WaitForFences)(screen->dev, 1, &screen->fence, VK_TRUE, UINT64_MAX);
      if (!zink_screen_handle_vkresult(screen, error)) {
         simple_mtx_unlock(&screen->queue_lock);
         VKSCR(DestroySemaphore)(screen->dev, cpi->sem, NULL);
         goto out;
      }
      cpi->info.pWaitSemaphores = NULL;
      cpi->info.waitSemaphoreCount = 0;
   }
   VkResult error2 = VKSCR(QueuePresentKHR)(screen->queue, &cpi->info);
   zink_screen_debug_marker_end(screen, screen->frame_marker_emitted);
   zink_screen_debug_marker_begin(screen, "frame");
   simple_mtx_unlock(&screen->queue_lock);
   swapchain->last_present = cpi->image;
   if (cpi->indefinite_acquire)
      p_atomic_dec(&swapchain->num_acquires);
   if (error2 == VK_SUBOPTIMAL_KHR && cdt->swapchain == swapchain)
      cpi->res->obj->new_dt = true;

   /* it's illegal to destroy semaphores if they're in use by a cmdbuf.
    * but what does "in use" actually mean?
    * in truth, when using timelines, nobody knows. especially not VVL.
    *
    * thus, to avoid infinite error spam and thread-related races,
    * present semaphores need their own free queue based on the
    * last-known completed timeline id so that the semaphore persists through
    * normal cmdbuf submit/signal and then also exists here when it's needed for the present operation
    */
   struct util_dynarray *arr;
   for (; screen->last_finished && swapchain->last_present_prune != screen->last_finished; swapchain->last_present_prune++) {
      struct hash_entry *he = _mesa_hash_table_search(swapchain->presents,
                                                      (void*)(uintptr_t)swapchain->last_present_prune);
      if (he) {
         arr = he->data;
         simple_mtx_lock(&screen->semaphores_lock);
         util_dynarray_append_dynarray(&screen->semaphores, arr);
         simple_mtx_unlock(&screen->semaphores_lock);
         util_dynarray_fini(arr);
         free(arr);
         _mesa_hash_table_remove(swapchain->presents, he);
      }
   }
   /* queue this wait semaphore for deletion on completion of the next batch */
   assert(screen->curr_batch > 0);
   uint32_t next = (uint32_t)screen->curr_batch + 1;
   /* handle overflow */
   next = MAX2(next + 1, 1);
   struct hash_entry *he = _mesa_hash_table_search(swapchain->presents, (void*)(uintptr_t)next);
   if (he)
      arr = he->data;
   else {
      arr = malloc(sizeof(struct util_dynarray));
      if (!arr) {
         mesa_loge("ZINK: failed to allocate arr!");
         return;
      }

      util_dynarray_init(arr, NULL);
      _mesa_hash_table_insert(swapchain->presents, (void*)(uintptr_t)next, arr);
   }
   util_dynarray_append(arr, VkSemaphore, cpi->sem);
out:
   if (thread_idx != -1) {
      p_atomic_dec(&swapchain->async_presents);
      struct pipe_resource *pres = &cpi->res->base.b;
      pipe_resource_reference(&pres, NULL);
   }
   free(cpi);
}

void
zink_kopper_present_queue(struct zink_screen *screen, struct zink_resource *res)
{
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;
   assert(zink_kopper_acquired(res->obj->dt, res->obj->dt_idx));
   assert(res->obj->present);

   /* always try to prune if the current swapchain has seen presents */
   if (cdt->swapchain->last_present != UINT32_MAX)
      prune_old_swapchains(screen, cdt, false);

   struct kopper_present_info *cpi = malloc(sizeof(struct kopper_present_info));
   if (!cpi) {
      mesa_loge("ZINK: failed to allocate cpi!");
      return;
   }
      
   cpi->sem = res->obj->present;
   cpi->res = res;
   cpi->swapchain = cdt->swapchain;
   cpi->indefinite_acquire = res->obj->indefinite_acquire;
   cpi->image = res->obj->dt_idx;
   cpi->info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   cpi->info.pNext = NULL;
   cpi->info.waitSemaphoreCount = 1;
   cpi->info.pWaitSemaphores = &cpi->sem;
   cpi->info.swapchainCount = 1;
   cpi->info.pSwapchains = &cdt->swapchain->swapchain;
   cpi->info.pImageIndices = &cpi->image;
   cpi->info.pResults = NULL;
   res->obj->present = VK_NULL_HANDLE;
   /* Ex GLX_EXT_buffer_age:
    *
    *  Buffers' ages are initialized to 0 at buffer creation time.
    *  When a frame boundary is reached, the following occurs before
    *  any exchanging or copying of color buffers:
    *
    *  * The current back buffer's age is set to 1.
    *  * Any other color buffers' ages are incremented by 1 if
    *    their age was previously greater than 0.
    */
   if (!cdt->age_locked) {
      for (int i = 0; i < cdt->swapchain->num_images; i++) {
            if (i == res->obj->dt_idx)
               cdt->swapchain->images[i].age = 1;
            else if (cdt->swapchain->images[i].age > 0)
               cdt->swapchain->images[i].age += 1;
      }
   }
   if (util_queue_is_initialized(&screen->flush_queue)) {
      p_atomic_inc(&cpi->swapchain->async_presents);
      struct pipe_resource *pres = NULL;
      pipe_resource_reference(&pres, &res->base.b);
      util_queue_add_job(&screen->flush_queue, cpi, &cdt->swapchain->present_fence,
                         kopper_present, NULL, 0);
   } else {
      kopper_present(cpi, screen, -1);
   }
   res->obj->indefinite_acquire = false;
   cdt->swapchain->images[res->obj->dt_idx].acquired = false;
   res->obj->dt_idx = UINT32_MAX;
}

void
zink_kopper_update_last_written(struct zink_resource *res)
{
   res->obj->last_dt_idx = res->obj->dt_idx;
}

static void
kopper_ensure_readback(struct zink_screen *screen, struct zink_resource *res)
{
   struct kopper_displaytarget *cdt = res->obj->dt;
   struct kopper_swapchain *cswap = cdt->swapchain;

   for (unsigned i = 0; i < cswap->num_images; i++) {
      if (cswap->images[i].readback)
         return;
      struct pipe_resource templ = res->base.b;
      templ.bind = PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW;
      cswap->images[i].readback = screen->base.resource_create(&screen->base, &templ);
   }
}

bool
zink_kopper_acquire_readback(struct zink_context *ctx, struct zink_resource *res, struct zink_resource **readback)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;
   const struct kopper_swapchain *cswap = cdt->swapchain;
   uint32_t last_dt_idx = res->obj->last_dt_idx;
   VkResult ret = VK_SUCCESS;

   /* if this hasn't been presented or if it has data, use this as the readback target */
   if (res->obj->last_dt_idx == UINT32_MAX ||
       (zink_kopper_acquired(cdt, res->obj->dt_idx) && cdt->swapchain->images[res->obj->dt_idx].dt_has_data)) {
      *readback = res;
      return false;
   }
   if (cswap->images[last_dt_idx].readback) {
      struct zink_resource *rb = zink_resource(cswap->images[res->obj->last_dt_idx].readback);
      if (rb->valid) {
         *readback = rb;
         return false;
      }
   }
   if (++cdt->readback_counter >= ZINK_READBACK_THRESHOLD)
      kopper_ensure_readback(screen, res);
   while (res->obj->dt_idx != last_dt_idx) {
      cdt->age_locked = true;
      if (res->obj->dt_idx != UINT32_MAX && !zink_kopper_present_readback(ctx, res))
         break;
      cdt->age_locked = true;
      do {
         ret = kopper_acquire(screen, res, 0);
      } while (!is_swapchain_kill(ret) && (ret == VK_NOT_READY || ret == VK_TIMEOUT));
      if (is_swapchain_kill(ret)) {
         kill_swapchain(ctx, res);
         *readback = NULL;
         cdt->age_locked = false;
         return false;
      }
   }
   if (cswap != cdt->swapchain) {
      ctx->swapchain_size = cdt->swapchain->scci.imageExtent;
      res->base.b.width0 = ctx->swapchain_size.width;
      res->base.b.height0 = ctx->swapchain_size.height;
   }
   zink_batch_usage_set(&cdt->swapchain->batch_uses, ctx->batch.state);
   *readback = res;
   return true;
}

bool
zink_kopper_present_readback(struct zink_context *ctx, struct zink_resource *res)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkSubmitInfo si = {0};
   assert(zink_is_swapchain(res));
   if (res->obj->last_dt_idx == UINT32_MAX)
      return true;
   if (res->layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      zink_screen(ctx->base.screen)->image_barrier(ctx, res, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
      ctx->base.flush(&ctx->base, NULL, 0);
   }
   si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   si.signalSemaphoreCount = 1;
   VkPipelineStageFlags mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   si.pWaitDstStageMask = &mask;
   VkSemaphore acquire = zink_kopper_acquire_submit(screen, res);
   VkSemaphore present = res->obj->present ? res->obj->present : zink_kopper_present(screen, res);
   if (screen->threaded_submit)
      util_queue_finish(&screen->flush_queue);
   si.waitSemaphoreCount = !!acquire;
   si.pWaitSemaphores = &acquire;
   si.pSignalSemaphores = &present;
   simple_mtx_lock(&screen->queue_lock);
   VkResult error = VKSCR(QueueSubmit)(screen->queue, 1, &si, VK_NULL_HANDLE);
   simple_mtx_unlock(&screen->queue_lock);
   if (!zink_screen_handle_vkresult(screen, error))
      return false;

   zink_kopper_present_queue(screen, res);
   if (util_queue_is_initialized(&screen->flush_queue)) {
      struct kopper_displaytarget *cdt = res->obj->dt;
      util_queue_fence_wait(&cdt->swapchain->present_fence);
   }

   simple_mtx_lock(&screen->queue_lock);
   error = VKSCR(QueueWaitIdle)(screen->queue);
   simple_mtx_unlock(&screen->queue_lock);

   simple_mtx_lock(&screen->semaphores_lock);
   util_dynarray_append(&screen->semaphores, VkSemaphore, acquire);
   simple_mtx_unlock(&screen->semaphores_lock);

   struct kopper_displaytarget *cdt = res->obj->dt;
   cdt->age_locked = false;

   return zink_screen_handle_vkresult(screen, error);
}

void
zink_kopper_readback_update(struct zink_context *ctx, struct zink_resource *res)
{
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;
   struct kopper_swapchain *cswap = cdt->swapchain;
   assert(res->obj->dt_idx != UINT32_MAX);
   struct pipe_resource *readback = cswap->images[res->obj->dt_idx].readback;
   struct pipe_box box = {0, 0, 0, res->base.b.width0, res->base.b.height0, res->base.b.depth0};

   if (readback)
      ctx->base.resource_copy_region(&ctx->base, readback, 0, 0, 0, 0, &res->base.b, 0, &box);
}

bool
zink_kopper_update(struct pipe_screen *pscreen, struct pipe_resource *pres, int *w, int *h)
{
   struct zink_resource *res = zink_resource(pres);
   struct zink_screen *screen = zink_screen(pscreen);
   if (!res->obj->dt)
      return false;
   struct kopper_displaytarget *cdt = res->obj->dt;
   if (cdt->type != KOPPER_X11) {
      *w = res->base.b.width0;
      *h = res->base.b.height0;
      return true;
   }
   VkResult ret = update_caps(screen, cdt);
   if (ret != VK_SUCCESS) {
      mesa_loge("zink: failed to update swapchain capabilities: %s", vk_Result_to_str(ret));
      cdt->is_kill = true;
      return false;
   }
   *w = cdt->caps.currentExtent.width;
   *h = cdt->caps.currentExtent.height;
   return true;
}

bool
zink_kopper_is_cpu(const struct pipe_screen *pscreen)
{
   const struct zink_screen *screen = (const struct zink_screen*)pscreen;
   return screen->is_cpu;
}

void
zink_kopper_fixup_depth_buffer(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (!ctx->fb_state.zsbuf)
      return;

   assert(ctx->fb_state.zsbuf->texture->bind & PIPE_BIND_DISPLAY_TARGET);

   struct zink_resource *res = zink_resource(ctx->fb_state.zsbuf->texture);
   struct zink_surface *surf = zink_csurface(ctx->fb_state.zsbuf);
   struct zink_ctx_surface *csurf = (struct zink_ctx_surface*)ctx->fb_state.zsbuf;
   if (surf->info.width == ctx->fb_state.width &&
       surf->info.height == ctx->fb_state.height)
      return;

   struct pipe_resource templ = *ctx->fb_state.zsbuf->texture;
   templ.width0 = ctx->fb_state.width;
   templ.height0 = ctx->fb_state.height;
   struct pipe_resource *pz = screen->base.resource_create(&screen->base, &templ);
   struct zink_resource *z = zink_resource(pz);
   zink_resource_object_reference(screen, &res->obj, z->obj);
   res->base.b.width0 = ctx->fb_state.width;
   res->base.b.height0 = ctx->fb_state.height;
   pipe_resource_reference(&pz, NULL);

   ctx->fb_state.zsbuf->width = ctx->fb_state.width;
   ctx->fb_state.zsbuf->height = ctx->fb_state.height;
   struct pipe_surface *psurf = ctx->base.create_surface(&ctx->base, &res->base.b, ctx->fb_state.zsbuf);
   struct zink_ctx_surface *cz = (struct zink_ctx_surface*)psurf;

   /* oh god why */
   zink_surface_reference(screen, &csurf->surf, cz->surf);
   pipe_surface_release(&ctx->base, &psurf);
}

bool
zink_kopper_check(struct pipe_resource *pres)
{
   struct zink_resource *res = zink_resource(pres);
   assert(pres->bind & PIPE_BIND_DISPLAY_TARGET);
   if (!res->obj->dt)
      return false;
   struct kopper_displaytarget *cdt = res->obj->dt;
   return !cdt->is_kill;
}

void
zink_kopper_set_swap_interval(struct pipe_screen *pscreen, struct pipe_resource *pres, int interval)
{
   struct zink_resource *res = zink_resource(pres);
   struct zink_screen *screen = zink_screen(pscreen);
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;
   VkPresentModeKHR old_present_mode = cdt->present_mode;

   zink_kopper_set_present_mode_for_interval(cdt, interval);

   if (old_present_mode != cdt->present_mode)
      update_swapchain(screen, cdt, cdt->caps.currentExtent.width, cdt->caps.currentExtent.height);
}

int
zink_kopper_query_buffer_age(struct pipe_context *pctx, struct pipe_resource *pres)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_resource *res = zink_resource(pres);
   assert(res->obj->dt);
   struct kopper_displaytarget *cdt = res->obj->dt;

   ctx = zink_tc_context_unwrap(pctx, zink_screen(pctx->screen)->threaded);

   /* Returning 0 here isn't ideal (yes, the buffer is undefined, because you
    * lost it) but threading the error up is more hassle than it's worth.
    */
   if (!zink_kopper_acquired(res->obj->dt, res->obj->dt_idx))
      if (!zink_kopper_acquire(ctx, res, UINT64_MAX))
         return 0;

   return cdt->swapchain->images[res->obj->dt_idx].age;
}

static void
swapchain_prune_batch_usage(struct kopper_swapchain *cswap, const struct zink_batch_usage *u)
{
   if (cswap->batch_uses == u)
      cswap->batch_uses = NULL;
}

void
zink_kopper_prune_batch_usage(struct kopper_displaytarget *cdt, const struct zink_batch_usage *u)
{
   struct kopper_swapchain *cswap = cdt->swapchain;
   swapchain_prune_batch_usage(cswap, u);
   for (cswap = cdt->old_swapchain; cswap; cswap = cswap->next)
      swapchain_prune_batch_usage(cswap, u);
}
