/*
 * Copyright 2012-2014, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *      Artur Wyszynski, harakash@gmail.com
 *      Alexander von Gluck IV, kallisti5@unixzen.com
 */

#include "hgl_context.h"

#include <stdio.h>

#include "util/format/u_formats.h"
#include "util/u_atomic.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "state_tracker/st_context.h"


#ifdef DEBUG
#   define TRACE(x...) printf("hgl:frontend: " x)
#   define CALLED() TRACE("CALLED: %s\n", __PRETTY_FUNCTION__)
#else
#   define TRACE(x...)
#   define CALLED()
#endif
#define ERROR(x...) printf("hgl:frontend: " x)


// Perform a safe void to hgl_context cast
static inline struct hgl_context*
hgl_st_context(struct st_context *st)
{
	struct hgl_context* context;
	assert(st);
	context = (struct hgl_context*)st->frontend_context;
	assert(context);
	return context;
}


// Perform a safe void to hgl_buffer cast
static struct hgl_buffer*
hgl_st_framebuffer(struct pipe_frontend_drawable *drawable)
{
	struct hgl_buffer* buffer;
	assert(drawable);
	buffer = (struct hgl_buffer*)drawable;
	assert(buffer);
	return buffer;
}


static bool
hgl_st_framebuffer_flush_front(struct st_context *st,
	struct pipe_frontend_drawable* drawable, enum st_attachment_type statt)
{
	CALLED();

	struct hgl_buffer* buffer = hgl_st_framebuffer(drawable);
	struct pipe_resource* ptex = buffer->textures[statt];

	if (statt != ST_ATTACHMENT_FRONT_LEFT)
		return false;

	if (!ptex)
		return true;

	// TODO: pipe_context here??? Might be needed for hw renderers
	buffer->screen->flush_frontbuffer(buffer->screen, NULL, ptex, 0, 0,
		buffer->winsysContext, NULL);

	return true;
}


static bool
hgl_st_framebuffer_validate_textures(struct pipe_frontend_drawable *drawable,
	unsigned width, unsigned height, unsigned mask)
{
	struct hgl_buffer* buffer;
	enum st_attachment_type i;
	struct pipe_resource templat;

	CALLED();

	buffer = hgl_st_framebuffer(drawable);

	if (buffer->width != width || buffer->height != height) {
		TRACE("validate_textures: size changed: %d, %d -> %d, %d\n",
			buffer->width, buffer->height, width, height);
		for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
			pipe_resource_reference(&buffer->textures[i], NULL);
	}

	memset(&templat, 0, sizeof(templat));
	templat.target = buffer->target;
	templat.width0 = width;
	templat.height0 = height;
	templat.depth0 = 1;
	templat.array_size = 1;
	templat.last_level = 0;

	for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
		enum pipe_format format;
		unsigned bind;

		if (((1 << i) & buffer->visual.buffer_mask) && buffer->textures[i] == NULL) {
			switch (i) {
				case ST_ATTACHMENT_FRONT_LEFT:
				case ST_ATTACHMENT_BACK_LEFT:
				case ST_ATTACHMENT_FRONT_RIGHT:
				case ST_ATTACHMENT_BACK_RIGHT:
					format = buffer->visual.color_format;
					bind = PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_RENDER_TARGET;
					break;
				case ST_ATTACHMENT_DEPTH_STENCIL:
					format = buffer->visual.depth_stencil_format;
					bind = PIPE_BIND_DEPTH_STENCIL;
					break;
				default:
					format = PIPE_FORMAT_NONE;
					bind = 0;
					break;
			}

			if (format != PIPE_FORMAT_NONE) {
				templat.format = format;
				templat.bind = bind;
				TRACE("resource_create(%d, %d, %d)\n", i, format, bind);
				buffer->textures[i] = buffer->screen->resource_create(buffer->screen,
					&templat);
				if (!buffer->textures[i])
					return false;
			}
		}
	}

	buffer->width = width;
	buffer->height = height;
	buffer->mask = mask;

	return true;
}


/**
 * Called by the st manager to validate the framebuffer (allocate
 * its resources).
 */
static bool
hgl_st_framebuffer_validate(struct st_context *st,
	struct pipe_frontend_drawable *drawable, const enum st_attachment_type *statts,
	unsigned count, struct pipe_resource **out, struct pipe_resource **resolve)
{
	struct hgl_buffer* buffer;
	unsigned stAttachmentMask, newMask;
	unsigned i;
	bool resized;

	CALLED();

	buffer = hgl_st_framebuffer(drawable);

	// Build mask of current attachments
	stAttachmentMask = 0;
	for (i = 0; i < count; i++)
		stAttachmentMask |= 1 << statts[i];

	newMask = stAttachmentMask & ~buffer->mask;

	resized = (buffer->width != buffer->newWidth)
		|| (buffer->height != buffer->newHeight);

	if (resized || newMask) {
		bool ret;
		TRACE("%s: resize event. old:  %d x %d; new: %d x %d\n", __func__,
			buffer->width, buffer->height, buffer->newWidth, buffer->newHeight);

		ret = hgl_st_framebuffer_validate_textures(drawable,
			buffer->newWidth, buffer->newHeight, stAttachmentMask);

		if (!ret)
			return ret;
	}

	for (i = 0; i < count; i++)
		pipe_resource_reference(&out[i], buffer->textures[statts[i]]);

	return true;
}


static int
hgl_st_manager_get_param(struct pipe_frontend_screen *fscreen, enum st_manager_param param)
{
	CALLED();

	switch (param) {
		case ST_MANAGER_BROKEN_INVALIDATE:
			return 1;
	}

	return 0;
}


static uint32_t hgl_fb_ID = 0;

/**
 * Create new framebuffer
 */
struct hgl_buffer *
hgl_create_st_framebuffer(struct hgl_display *display, struct st_visual* visual, void *winsysContext)
{
	struct hgl_buffer *buffer;
	CALLED();

	// Our requires before creating a framebuffer
	assert(display);
	assert(visual);

	buffer = CALLOC_STRUCT(hgl_buffer);
	assert(buffer);

	// Prepare our buffer
	buffer->visual = *visual;
	buffer->screen = display->fscreen->screen;
	buffer->winsysContext = winsysContext;

	if (buffer->screen->get_param(buffer->screen, PIPE_CAP_NPOT_TEXTURES))
		buffer->target = PIPE_TEXTURE_2D;
	else
		buffer->target = PIPE_TEXTURE_RECT;

	// Prepare our frontend interface
	buffer->base.flush_front = hgl_st_framebuffer_flush_front;
	buffer->base.validate = hgl_st_framebuffer_validate;
	buffer->base.visual = &buffer->visual;

	p_atomic_set(&buffer->base.stamp, 1);
	buffer->base.ID = p_atomic_inc_return(&hgl_fb_ID);
	buffer->base.fscreen = display->fscreen;

	return buffer;
}


void
hgl_destroy_st_framebuffer(struct hgl_buffer *buffer)
{
	CALLED();

	int i;
	for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
		pipe_resource_reference(&buffer->textures[i], NULL);

	FREE(buffer);
}


struct hgl_context*
hgl_create_context(struct hgl_display *display, struct st_visual* visual, struct st_context* shared)
{
	struct hgl_context* context = CALLOC_STRUCT(hgl_context);
	assert(context);
	context->display = display;

	struct st_context_attribs attribs;
	memset(&attribs, 0, sizeof(attribs));
	attribs.options.force_glsl_extensions_warn = false;
	attribs.profile = API_OPENGL_COMPAT;
	attribs.visual = *visual;
	attribs.major = 1;
	attribs.minor = 0;

	enum st_context_error result;
	context->st = st_api_create_context(display->fscreen, &attribs, &result, shared);
	if (context->st == NULL) {
		FREE(context);
		return NULL;
	}

	assert(!context->st->frontend_context);
	context->st->frontend_context = (void*)context;

	struct st_context *stContext = (struct st_context*)context->st;

	// Init Gallium3D Post Processing
	// TODO: no pp filters are enabled yet through postProcessEnable
	context->postProcess = pp_init(stContext->pipe, context->postProcessEnable, stContext->cso_context, stContext, (void*)st_context_invalidate_state);

	return context;
}


void
hgl_destroy_context(struct hgl_context* context)
{
	if (context->st) {
		st_context_flush(context->st, 0, NULL, NULL, NULL);
		st_destroy_context(context->st);
	}

	if (context->postProcess)
		pp_free(context->postProcess);

	FREE(context);
}


void
hgl_get_st_visual(struct st_visual* visual, ulong options)
{
	CALLED();

	assert(visual);

	// Determine color format
	if ((options & HGL_INDEX) != 0) {
		// Index color
		visual->color_format = PIPE_FORMAT_B5G6R5_UNORM;
		// TODO: Indexed color depth buffer?
		visual->depth_stencil_format = PIPE_FORMAT_NONE;
	} else {
		// RGB color
		visual->color_format = (options & HGL_ALPHA)
			? PIPE_FORMAT_BGRA8888_UNORM : PIPE_FORMAT_BGRX8888_UNORM;
		// TODO: Determine additional stencil formats
		visual->depth_stencil_format = (options & HGL_DEPTH)
			? PIPE_FORMAT_Z24_UNORM_S8_UINT : PIPE_FORMAT_NONE;
    }

	visual->accum_format = (options & HGL_ACCUM)
		? PIPE_FORMAT_R16G16B16A16_SNORM : PIPE_FORMAT_NONE;

	visual->buffer_mask |= ST_ATTACHMENT_FRONT_LEFT_MASK;

	if ((options & HGL_DOUBLE) != 0) {
		TRACE("double buffer enabled\n");
		visual->buffer_mask |= ST_ATTACHMENT_BACK_LEFT_MASK;
	}

#if 0
	if ((options & HGL_STEREO) != 0) {
		visual->buffer_mask |= ST_ATTACHMENT_FRONT_RIGHT_MASK;
		if ((options & HGL_DOUBLE) != 0)
			visual->buffer_mask |= ST_ATTACHMENT_BACK_RIGHT_MASK;
  }
#endif

	if ((options & HGL_DEPTH) || (options & HGL_STENCIL))
		visual->buffer_mask |= ST_ATTACHMENT_DEPTH_STENCIL_MASK;

	TRACE("%s: Visual color format: %s\n", __func__,
		util_format_name(visual->color_format));
}


struct hgl_display*
hgl_create_display(struct pipe_screen* screen)
{
	struct hgl_display* display;

	display = CALLOC_STRUCT(hgl_display);
	assert(display);
	display->fscreen = CALLOC_STRUCT(pipe_frontend_screen);
	assert(display->fscreen);
	display->fscreen->screen = screen;
	display->fscreen->get_param = hgl_st_manager_get_param;
	// display->fscreen->st_screen is used by llvmpipe

	return display;
}


void
hgl_destroy_display(struct hgl_display *display)
{
	st_screen_destroy(display->fscreen);
	FREE(display->fscreen);
	FREE(display);
}
