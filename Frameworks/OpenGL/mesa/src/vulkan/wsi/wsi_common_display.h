/*
 * Copyright © 2017 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef WSI_COMMON_DISPLAY_H
#define WSI_COMMON_DISPLAY_H

#include "wsi_common.h"
#include <xf86drm.h>
#include <xf86drmMode.h>

struct vk_sync;

/* VK_EXT_display_control */
VkResult
wsi_register_device_event(VkDevice                      device,
                          struct wsi_device             *wsi_device,
                          const VkDeviceEventInfoEXT    *device_event_info,
                          const VkAllocationCallbacks   *allocator,
                          struct vk_sync                **sync,
                          int sync_fd);

VkResult
wsi_register_display_event(VkDevice                     device,
                           struct wsi_device            *wsi_device,
                           VkDisplayKHR                 display,
                           const VkDisplayEventInfoEXT  *display_event_info,
                           const VkAllocationCallbacks  *allocator,
                           struct vk_sync               **sync,
                           int sync_fd);

#endif
