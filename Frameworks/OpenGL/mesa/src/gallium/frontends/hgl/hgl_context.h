/*
 * Copyright 2009-2023, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Alexander von Gluck IV, kallisti5@unixzen.com
 */
#ifndef HGL_CONTEXT_H
#define HGL_CONTEXT_H

#include "util/u_thread.h"
#include "util/format/u_formats.h"
#include "util/compiler.h"
#include "pipe/p_screen.h"
#include "postprocess/filters.h"

#include "frontend/api.h"

// visual options
#define HGL_RGB			0
#define HGL_INDEX		1
#define HGL_SINGLE		0
#define HGL_DOUBLE		2
#define HGL_DIRECT		0
#define HGL_INDIRECT		4
#define HGL_ACCUM		8
#define HGL_ALPHA		16
#define HGL_DEPTH		32
#define HGL_OVERLAY		64
#define HGL_UNDERLAY		128
#define HGL_STENCIL		512
#define HGL_SHARE_CONTEXT	1024


#ifdef __cplusplus
extern "C" {
#endif


#define CONTEXT_MAX 32

typedef int64 context_id;


struct hgl_buffer
{
	struct pipe_frontend_drawable base;
	struct st_visual visual;

	unsigned width;
	unsigned height;
	unsigned newWidth;
	unsigned newHeight;
	unsigned mask;

	struct pipe_screen* screen;
	void* winsysContext;

	enum pipe_texture_target target;
	struct pipe_resource* textures[ST_ATTACHMENT_COUNT];
};


struct hgl_display
{
	mtx_t mutex;

	struct pipe_frontend_screen *fscreen;
};


struct hgl_context
{
	struct hgl_display* display;
	struct st_context* st;

	// Post processing
	struct pp_queue_t* postProcess;
	unsigned int postProcessEnable[PP_FILTERS];
};

// hgl framebuffer
struct hgl_buffer* hgl_create_st_framebuffer(struct hgl_display *display, struct st_visual* visual, void *winsysContext);
void hgl_destroy_st_framebuffer(struct hgl_buffer *buffer);

struct hgl_context* hgl_create_context(struct hgl_display *display, struct st_visual* visual, struct st_context* shared);
void hgl_destroy_context(struct hgl_context* context);

// hgl visual
void hgl_get_st_visual(struct st_visual* visual, ulong options);

// hgl display
struct hgl_display* hgl_create_display(struct pipe_screen* screen);
void hgl_destroy_display(struct hgl_display *display);


#ifdef __cplusplus
}
#endif

#endif /* HGL_CONTEXT_H */
