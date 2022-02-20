#include <drm_fourcc.h>
#include <wlr/util/log.h>

#include "render/pixman.h"

static const struct wlr_pixman_pixel_format formats[] = {
	{
		.drm_format = DRM_FORMAT_ARGB8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8a8,
#else
		.pixman_format = PIXMAN_a8r8g8b8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_XBGR8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8x8,
#else
		.pixman_format = PIXMAN_x8b8g8r8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_XRGB8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8x8,
#else
		.pixman_format = PIXMAN_x8r8g8b8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_ABGR8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8a8,
#else
		.pixman_format = PIXMAN_a8b8g8r8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_RGBA8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_a8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8a8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_RGBX8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_x8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8x8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_BGRA8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_a8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8a8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_BGRX8888,
#if WLR_BIG_ENDIAN
		.pixman_format = PIXMAN_x8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8x8,
#endif
	},
#if WLR_LITTLE_ENDIAN
	// Since DRM formats are always little-endian, they don't have an
	// equivalent on big-endian if their components are spanning across
	// multiple bytes.
	{
		.drm_format = DRM_FORMAT_RGB565,
		.pixman_format = PIXMAN_r5g6b5,
	},
	{
		.drm_format = DRM_FORMAT_BGR565,
		.pixman_format = PIXMAN_b5g6r5,
	},
	{
		.drm_format = DRM_FORMAT_ARGB2101010,
		.pixman_format = PIXMAN_a2r10g10b10,
	},
	{
		.drm_format = DRM_FORMAT_XRGB2101010,
		.pixman_format = PIXMAN_x2r10g10b10,
	},
	{
		.drm_format = DRM_FORMAT_ABGR2101010,
		.pixman_format = PIXMAN_a2b10g10r10,
	},
	{
		.drm_format = DRM_FORMAT_XBGR2101010,
		.pixman_format = PIXMAN_x2b10g10r10,
	},
#endif
};

pixman_format_code_t get_pixman_format_from_drm(uint32_t fmt) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (formats[i].drm_format == fmt) {
			return formats[i].pixman_format;
		}
	}

	wlr_log(WLR_ERROR, "DRM format 0x%"PRIX32" has no pixman equivalent", fmt);
	return 0;
}

uint32_t get_drm_format_from_pixman(pixman_format_code_t fmt) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (formats[i].pixman_format == fmt) {
			return formats[i].drm_format;
		}
	}

	wlr_log(WLR_ERROR, "pixman format 0x%"PRIX32" has no drm equivalent", fmt);
	return DRM_FORMAT_INVALID;
}

const uint32_t *get_pixman_drm_formats(size_t *len) {
	static uint32_t drm_formats[sizeof(formats) / sizeof(formats[0])];
	*len = sizeof(formats) / sizeof(formats[0]);
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
		drm_formats[i] = formats[i].drm_format;
	}
	return drm_formats;
}
