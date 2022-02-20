#ifndef RENDER_PIXMAN_H
#define RENDER_PIXMAN_H

#include <wlr/render/pixman.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/drm_format_set.h>
#include "render/pixel_format.h"

struct wlr_pixman_pixel_format {
	uint32_t drm_format;
	pixman_format_code_t pixman_format;
};

struct wlr_pixman_buffer;

struct wlr_pixman_renderer {
	struct wlr_renderer wlr_renderer;

	struct wl_list buffers; // wlr_pixman_buffer.link
	struct wl_list textures; // wlr_pixman_texture.link

	struct wlr_pixman_buffer *current_buffer;
	int32_t width, height;

	struct wlr_drm_format_set drm_formats;
};

struct wlr_pixman_buffer {
	struct wlr_buffer *buffer;
	struct wlr_pixman_renderer *renderer;

	pixman_image_t *image;

	struct wl_listener buffer_destroy;
	struct wl_list link; // wlr_pixman_renderer.buffers
};

struct wlr_pixman_texture {
	struct wlr_texture wlr_texture;
	struct wlr_pixman_renderer *renderer;
	struct wl_list link; // wlr_pixman_renderer.textures

	pixman_image_t *image;
	pixman_format_code_t format;
	const struct wlr_pixel_format_info *format_info;

	void *data; // if created via texture_from_pixels
	struct wlr_buffer *buffer; // if created via texture_from_buffer
};

pixman_format_code_t get_pixman_format_from_drm(uint32_t fmt);
uint32_t get_drm_format_from_pixman(pixman_format_code_t fmt);
const uint32_t *get_pixman_drm_formats(size_t *len);

#endif
