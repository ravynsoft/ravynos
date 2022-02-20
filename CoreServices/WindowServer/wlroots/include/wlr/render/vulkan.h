/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_RENDER_VULKAN_H
#define WLR_RENDER_VULKAN_H

#include <wlr/render/wlr_renderer.h>

struct wlr_renderer *wlr_vk_renderer_create_with_drm_fd(int drm_fd);
bool wlr_texture_is_vk(struct wlr_texture *texture);

#endif

