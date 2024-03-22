/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * 
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */

#ifndef ZINK_KOPPER_H
#define ZINK_KOPPER_H

#include "kopper_interface.h"
#include "util/u_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zink_batch_usage;

/* number of times a swapchain can be read without forcing readback mode */
#define ZINK_READBACK_THRESHOLD 3

struct kopper_swapchain_image {
   bool init;
   bool acquired;
   bool dt_has_data;
   int age;
   VkImage image;
   struct pipe_resource *readback;
   VkSemaphore acquire;
   VkImageLayout layout;
};

struct kopper_swapchain {
   struct kopper_swapchain *next;
   VkSwapchainKHR swapchain;

   unsigned last_present;
   unsigned num_images;
   uint32_t last_present_prune;
   struct hash_table *presents;
   VkSwapchainCreateInfoKHR scci;
   unsigned num_acquires;
   unsigned max_acquires;
   unsigned async_presents;
   struct util_queue_fence present_fence;
   struct zink_batch_usage *batch_uses;
   struct kopper_swapchain_image *images;
};

enum kopper_type {
   KOPPER_X11,
   KOPPER_WAYLAND,
   KOPPER_WIN32
};

struct kopper_displaytarget
{
   unsigned refcount;
   VkFormat formats[2];
   unsigned width;
   unsigned height;
   unsigned stride;
   void *loader_private;

   VkSurfaceKHR surface;
   uint32_t present_modes; //VkPresentModeKHR bitmask
   struct kopper_swapchain *swapchain;
   struct kopper_swapchain *old_swapchain;

   struct kopper_loader_info info;

   VkSurfaceCapabilitiesKHR caps;
   VkImageFormatListCreateInfo format_list;
   enum kopper_type type;
   bool is_kill;
   VkPresentModeKHR present_mode;
   unsigned readback_counter;

   bool age_locked; //disables buffer age during readback
};

struct zink_context;
struct zink_screen;
struct zink_resource;

static inline bool
zink_kopper_has_srgb(const struct kopper_displaytarget *cdt)
{
   return cdt->formats[1] != VK_FORMAT_UNDEFINED;
}

static inline bool
zink_kopper_last_present_eq(const struct kopper_displaytarget *cdt, uint32_t idx)
{
   return cdt->swapchain->last_present == idx;
}

static inline bool
zink_kopper_acquired(const struct kopper_displaytarget *cdt, uint32_t idx)
{
   return idx != UINT32_MAX && cdt->swapchain->images[idx].acquired;
}

void
zink_kopper_update_last_written(struct zink_resource *res);

struct kopper_displaytarget *
zink_kopper_displaytarget_create(struct zink_screen *screen, unsigned tex_usage,
                                 enum pipe_format format, unsigned width,
                                 unsigned height, unsigned alignment,
                                 const void *loader_private, unsigned *stride);
void
zink_kopper_displaytarget_destroy(struct zink_screen *screen, struct kopper_displaytarget *cdt);


bool
zink_kopper_acquire(struct zink_context *ctx, struct zink_resource *res, uint64_t timeout);
VkSemaphore
zink_kopper_acquire_submit(struct zink_screen *screen, struct zink_resource *res);
VkSemaphore
zink_kopper_present(struct zink_screen *screen, struct zink_resource *res); 
void
zink_kopper_present_queue(struct zink_screen *screen, struct zink_resource *res);
bool
zink_kopper_acquire_readback(struct zink_context *ctx, struct zink_resource *res, struct zink_resource **readback);
bool
zink_kopper_present_readback(struct zink_context *ctx, struct zink_resource *res);
void
zink_kopper_readback_update(struct zink_context *ctx, struct zink_resource *res);
void
zink_kopper_deinit_displaytarget(struct zink_screen *screen, struct kopper_displaytarget *cdt);
bool
zink_kopper_update(struct pipe_screen *pscreen, struct pipe_resource *pres, int *w, int *h);
bool
zink_kopper_is_cpu(const struct pipe_screen *pscreen);
void
zink_kopper_fixup_depth_buffer(struct zink_context *ctx);
bool
zink_kopper_check(struct pipe_resource *pres);
void
zink_kopper_set_swap_interval(struct pipe_screen *pscreen, struct pipe_resource *pres, int interval);
int
zink_kopper_query_buffer_age(struct pipe_context *pctx, struct pipe_resource *pres);
void
zink_kopper_prune_batch_usage(struct kopper_displaytarget *cdt, const struct zink_batch_usage *u);

#ifdef __cplusplus
}
#endif

#endif
