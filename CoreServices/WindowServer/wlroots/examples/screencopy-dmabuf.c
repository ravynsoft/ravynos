/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2020 Andri Yngvason
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

#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <png.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <gbm.h>
#include <xf86drm.h>
#include <drm_fourcc.h>
#include <wayland-client-protocol.h>
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include "linux-dmabuf-unstable-v1-client-protocol.h"

struct format {
	uint32_t format;
	bool is_bgr;
};

static int drm_fd = -1;
static struct gbm_device *gbm_device = NULL;

static struct zwp_linux_dmabuf_v1 *dmabuf = NULL;
static struct zwlr_screencopy_manager_v1 *screencopy_manager = NULL;
static struct wl_output *output = NULL;

static struct {
	struct gbm_bo *bo;
	struct wl_buffer *wl_buffer;
	int width, height;
	uint32_t format;
	bool y_invert;
} buffer;

bool buffer_copy_done = false;
static bool have_linux_dmabuf = false;

// wl_shm_format describes little-endian formats, libpng uses big-endian
// formats (so Wayland's ABGR is libpng's RGBA).
static const struct format formats[] = {
	{DRM_FORMAT_XRGB8888, true},
	{DRM_FORMAT_ARGB8888, true},
	{DRM_FORMAT_XBGR8888, false},
	{DRM_FORMAT_ABGR8888, false},
};

static bool find_render_node(char *node, size_t maxlen) {
	bool r = false;
	drmDevice *devices[64];

	int n = drmGetDevices2(0, devices, sizeof(devices) / sizeof(devices[0]));
	for (int i = 0; i < n; ++i) {
		drmDevice *dev = devices[i];
		if (!(dev->available_nodes & (1 << DRM_NODE_RENDER))) {
			continue;
		}

		strncpy(node, dev->nodes[DRM_NODE_RENDER], maxlen - 1);
		node[maxlen - 1] = '\0';
		r = true;
		break;
	}

	drmFreeDevices(devices, n);
	return r;
}

static void dmabuf_created(void *data,
		struct zwp_linux_buffer_params_v1 *params,
		struct wl_buffer *wl_buffer) {
	buffer.wl_buffer = wl_buffer;

	zwlr_screencopy_frame_v1_copy(data, buffer.wl_buffer);
}

static void dmabuf_failed(void *data,
		struct zwp_linux_buffer_params_v1 *params) {
	fprintf(stderr, "Failed to create dmabuf\n");
	exit(EXIT_FAILURE);
}

static const struct zwp_linux_buffer_params_v1_listener params_listener = {
	.created = dmabuf_created,
	.failed = dmabuf_failed,
};

static void frame_handle_buffer(void *data,
		struct zwlr_screencopy_frame_v1 *frame, uint32_t wl_format,
		uint32_t width, uint32_t height, uint32_t stride) {
	// Not implemented
}

static void frame_handle_linux_dmabuf(void *data,
		struct zwlr_screencopy_frame_v1 *frame, uint32_t fourcc,
		uint32_t width, uint32_t height) {
	buffer.width = width;
	buffer.height = height;
	buffer.format = fourcc;
	have_linux_dmabuf = true;
}

static void frame_handle_buffer_done(void *data,
		struct zwlr_screencopy_frame_v1 *frame) {
	assert(!buffer.bo);

	if (!have_linux_dmabuf) {
		fprintf(stderr, "linux-dmabuf is not supported\n");
		exit(EXIT_FAILURE);
	}

	buffer.bo = gbm_bo_create(gbm_device, buffer.width, buffer.height,
			buffer.format,
			GBM_BO_USE_LINEAR | GBM_BO_USE_RENDERING);
	if (buffer.bo == NULL) {
		fprintf(stderr, "failed to create GBM buffer object\n");
		exit(EXIT_FAILURE);
	}

	struct zwp_linux_buffer_params_v1 *params;
	params = zwp_linux_dmabuf_v1_create_params(dmabuf);
	assert(params);

	int fd = gbm_bo_get_fd(buffer.bo);
	uint32_t off = gbm_bo_get_offset(buffer.bo, 0);
	uint32_t bo_stride = gbm_bo_get_stride(buffer.bo);
	uint64_t mod = gbm_bo_get_modifier(buffer.bo);
	zwp_linux_buffer_params_v1_add(params, fd, 0, off, bo_stride, mod >> 32,
			mod & 0xffffffff);

	zwp_linux_buffer_params_v1_add_listener(params, &params_listener, frame);

	zwp_linux_buffer_params_v1_create(params, buffer.width, buffer.height,
			buffer.format, /* flags */ 0);
}

static void frame_handle_flags(void *data,
		struct zwlr_screencopy_frame_v1 *frame, uint32_t flags) {
	buffer.y_invert = flags & ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT;
}

static void frame_handle_ready(void *data,
		struct zwlr_screencopy_frame_v1 *frame, uint32_t tv_sec_hi,
		uint32_t tv_sec_lo, uint32_t tv_nsec) {
	buffer_copy_done = true;
}

static void frame_handle_failed(void *data,
		struct zwlr_screencopy_frame_v1 *frame) {
	fprintf(stderr, "failed to copy frame\n");
	exit(EXIT_FAILURE);
}

static const struct zwlr_screencopy_frame_v1_listener frame_listener = {
	.buffer = frame_handle_buffer,
	.linux_dmabuf = frame_handle_linux_dmabuf,
	.buffer_done = frame_handle_buffer_done,
	.flags = frame_handle_flags,
	.ready = frame_handle_ready,
	.failed = frame_handle_failed,
};

static void dmabuf_format(void *data,
		struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format) {
	// deprecated
}

static void dmabuf_modifier(void *data,
		struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format,
		uint32_t modifier_hi, uint32_t modifier_lo) {
	// TODO?
}

static const struct zwp_linux_dmabuf_v1_listener dmabuf_listener = {
	.format = dmabuf_format,
	.modifier = dmabuf_modifier,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_output_interface.name) == 0 && output == NULL) {
		output = wl_registry_bind(registry, name, &wl_output_interface, 1);
	} else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
		dmabuf = wl_registry_bind(registry, name,
				&zwp_linux_dmabuf_v1_interface, 3);
		zwp_linux_dmabuf_v1_add_listener(dmabuf, &dmabuf_listener, data);
	} else if (strcmp(interface,
			zwlr_screencopy_manager_v1_interface.name) == 0) {
		screencopy_manager = wl_registry_bind(registry, name,
			&zwlr_screencopy_manager_v1_interface, 3);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// Who cares?
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static void write_image(char *filename, uint32_t format, int width,
		int height, int stride, bool y_invert, png_bytep data) {
	const struct format *fmt = NULL;
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
		if (formats[i].format == format) {
			fmt = &formats[i];
			break;
		}
	}

	if (fmt == NULL) {
		fprintf(stderr, "unsupported format %"PRIu32"\n", format);
		exit(EXIT_FAILURE);
	}

	FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		fprintf(stderr, "failed to open output file\n");
		exit(EXIT_FAILURE);
	}

	png_structp png =
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);

	png_init_io(png, f);

	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	if (fmt->is_bgr) {
		png_set_bgr(png);
	}

	png_write_info(png, info);

	for (size_t i = 0; i < (size_t)height; ++i) {
		png_bytep row;
		if (y_invert) {
			row = data + (height - i - 1) * stride;
		} else {
			row = data + i * stride;
		}
		png_write_row(png, row);
	}

	png_write_end(png, NULL);

	png_destroy_write_struct(&png, &info);

	fclose(f);
}

int main(int argc, char *argv[]) {
	char render_node[256];
	if (!find_render_node(render_node, sizeof(render_node))) {
		fprintf(stderr, "Failed to find a DRM render node\n");
		return EXIT_FAILURE;
	}

	printf("Using render node: %s\n", render_node);

	drm_fd = open(render_node, O_RDWR);
	if (drm_fd < 0) {
		perror("Failed to open drm render node");
		return EXIT_FAILURE;
	}

	gbm_device = gbm_create_device(drm_fd);
	if (!gbm_device) {
		fprintf(stderr, "Failed to create gbm device\n");
		return EXIT_FAILURE;
	}

	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		perror("failed to create display");
		return EXIT_FAILURE;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (dmabuf == NULL) {
		fprintf(stderr, "compositor is missing linux-dmabuf-unstable-v1\n");
		return EXIT_FAILURE;
	}
	if (screencopy_manager == NULL) {
		fprintf(stderr, "compositor doesn't support wlr-screencopy-unstable-v1\n");
		return EXIT_FAILURE;
	}
	if (output == NULL) {
		fprintf(stderr, "no output available\n");
		return EXIT_FAILURE;
	}

	struct zwlr_screencopy_frame_v1 *frame =
		zwlr_screencopy_manager_v1_capture_output(screencopy_manager, 0, output);
	zwlr_screencopy_frame_v1_add_listener(frame, &frame_listener, NULL);

	while (!buffer_copy_done && wl_display_dispatch(display) != -1) {
		// This space is intentionally left blank
	}

	uint32_t stride = 0;
	void *map_data = NULL;
	void *data = gbm_bo_map(buffer.bo, 0, 0, buffer.width, buffer.height,
			GBM_BO_TRANSFER_READ, &stride, &map_data);
	if (!data) {
		perror("Failed to map gbm bo");
		return EXIT_FAILURE;
	}

	write_image("wayland-screenshot.png", buffer.format, buffer.width,
		buffer.height, stride, buffer.y_invert, data);

	gbm_bo_unmap(buffer.bo, map_data);
	gbm_bo_destroy(buffer.bo);
	wl_buffer_destroy(buffer.wl_buffer);

	gbm_device_destroy(gbm_device);
	close(drm_fd);

	return EXIT_SUCCESS;
}
