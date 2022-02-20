#include <drm_fourcc.h>
#include "render/pixel_format.h"

static const struct wlr_pixel_format_info pixel_format_info[] = {
	{
		.drm_format = DRM_FORMAT_XRGB8888,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_ARGB8888,
		.opaque_substitute = DRM_FORMAT_XRGB8888,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_XBGR8888,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_ABGR8888,
		.opaque_substitute = DRM_FORMAT_XBGR8888,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_RGBX8888,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGBA8888,
		.opaque_substitute = DRM_FORMAT_RGBX8888,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_BGRX8888,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_BGRA8888,
		.opaque_substitute = DRM_FORMAT_BGRX8888,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_BGR888,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 24,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGBX4444,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGBA4444,
		.opaque_substitute = DRM_FORMAT_RGBX4444,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGBX5551,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGBA5551,
		.opaque_substitute = DRM_FORMAT_RGBX5551,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_RGB565,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_BGR565,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 16,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_XRGB2101010,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_ARGB2101010,
		.opaque_substitute = DRM_FORMAT_XRGB2101010,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_XBGR2101010,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 32,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_ABGR2101010,
		.opaque_substitute = DRM_FORMAT_XBGR2101010,
		.bpp = 32,
		.has_alpha = true,
	},
	{
		.drm_format = DRM_FORMAT_XBGR16161616F,
		.opaque_substitute = DRM_FORMAT_INVALID,
		.bpp = 64,
		.has_alpha = false,
	},
	{
		.drm_format = DRM_FORMAT_ABGR16161616F,
		.opaque_substitute = DRM_FORMAT_XBGR16161616F,
		.bpp = 64,
		.has_alpha = true,
	},
};

static const size_t pixel_format_info_size =
	sizeof(pixel_format_info) / sizeof(pixel_format_info[0]);

const struct wlr_pixel_format_info *drm_get_pixel_format_info(uint32_t fmt) {
	for (size_t i = 0; i < pixel_format_info_size; ++i) {
		if (pixel_format_info[i].drm_format == fmt) {
			return &pixel_format_info[i];
		}
	}

	return NULL;
}

uint32_t convert_wl_shm_format_to_drm(enum wl_shm_format fmt) {
	switch (fmt) {
	case WL_SHM_FORMAT_XRGB8888:
		return DRM_FORMAT_XRGB8888;
	case WL_SHM_FORMAT_ARGB8888:
		return DRM_FORMAT_ARGB8888;
	default:
		return (uint32_t)fmt;
	}
}

enum wl_shm_format convert_drm_format_to_wl_shm(uint32_t fmt) {
	switch (fmt) {
	case DRM_FORMAT_XRGB8888:
		return WL_SHM_FORMAT_XRGB8888;
	case DRM_FORMAT_ARGB8888:
		return WL_SHM_FORMAT_ARGB8888;
	default:
		return (enum wl_shm_format)fmt;
	}
}
