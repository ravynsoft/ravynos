#ifndef RENDER_PIXEL_FORMAT_H
#define RENDER_PIXEL_FORMAT_H

#include <wayland-server-protocol.h>

struct wlr_pixel_format_info {
	uint32_t drm_format;

	/* Equivalent of the format if it has an alpha channel,
	 * DRM_FORMAT_INVALID (0) if NA
	 */
	uint32_t opaque_substitute;

	/* Bits per pixels */
	uint32_t bpp;

	/* True if the format has an alpha channel */
	bool has_alpha;
};

const struct wlr_pixel_format_info *drm_get_pixel_format_info(uint32_t fmt);

uint32_t convert_wl_shm_format_to_drm(enum wl_shm_format fmt);
enum wl_shm_format convert_drm_format_to_wl_shm(uint32_t fmt);

#endif
