/*
 * Copyright © 2015 Intel Corporation
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

#pragma once

#include "lvp_private.h"

struct lvp_swapchain;

struct lvp_wsi_interface {
   VkResult (*get_support)(VkIcdSurfaceBase *surface,
                           struct lvp_physical_device *device,
                           uint32_t queueFamilyIndex,
                           VkBool32* pSupported);
   VkResult (*get_capabilities)(VkIcdSurfaceBase *surface,
                                struct lvp_physical_device *device,
                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
   VkResult (*get_formats)(VkIcdSurfaceBase *surface,
                           struct lvp_physical_device *device,
                           uint32_t* pSurfaceFormatCount,
                           VkSurfaceFormatKHR* pSurfaceFormats);
   VkResult (*get_present_modes)(VkIcdSurfaceBase *surface,
                                 struct lvp_physical_device *device,
                                 uint32_t* pPresentModeCount,
                                 VkPresentModeKHR* pPresentModes);
   VkResult (*create_swapchain)(VkIcdSurfaceBase *surface,
                                struct lvp_device *device,
                                const VkSwapchainCreateInfoKHR* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator,
                                struct lvp_swapchain **swapchain);
};

struct lvp_swapchain {
   struct lvp_device *device;

   VkResult (*destroy)(struct lvp_swapchain *swapchain,
                       const VkAllocationCallbacks *pAllocator);
   VkResult (*get_images)(struct lvp_swapchain *swapchain,
                          uint32_t *pCount, VkImage *pSwapchainImages);
   VkResult (*acquire_next_image)(struct lvp_swapchain *swap_chain,
                                  uint64_t timeout, VkSemaphore semaphore,
                                  uint32_t *image_index);
   VkResult (*queue_present)(struct lvp_swapchain *swap_chain,
                             struct lvp_queue *queue,
                             uint32_t image_index);
};

VkResult lvp_x11_init_wsi(struct lvp_instance *instance);
void lvp_x11_finish_wsi(struct lvp_instance *instance);
VkResult lvp_wl_init_wsi(struct lvp_instance *instance);
void lvp_wl_finish_wsi(struct lvp_instance *instance);
