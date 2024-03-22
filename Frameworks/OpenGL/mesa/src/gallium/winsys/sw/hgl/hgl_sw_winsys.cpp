/**************************************************************************
 *
 * Copyright 2009 Artur Wyszynski <harakash@gmail.com>
 * Copyright 2013-2014 Alexander von Gluck IV <kallisti5@unixzen.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

#include <stdio.h>

#include "util/compiler.h"
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "frontend/api.h"
#include "frontend/sw_winsys.h"

#include "hgl_sw_winsys.h"

#include <Bitmap.h>
#include <OS.h>

#ifdef DEBUG
#   define TRACE(x...) printf("hgl:winsys: " x)
#   define CALLED() TRACE("CALLED: %s\n", __PRETTY_FUNCTION__)
#else
#   define TRACE(x...)
#   define CALLED()
#endif
#define ERROR(x...) printf("hgl:winsys: " x)


struct haiku_displaytarget
{
	enum pipe_format format;
	color_space colorSpace;

	unsigned width;
	unsigned height;
	unsigned stride;

	unsigned size;

	void* data;
	BBitmap* bitmap;
};


// Cast
static inline struct haiku_displaytarget*
hgl_sw_displaytarget(struct sw_displaytarget* target)
{
	return (struct haiku_displaytarget *)target;
}


static void
hgl_winsys_destroy(struct sw_winsys* winsys)
{
	FREE(winsys);
}


static bool
hgl_winsys_is_displaytarget_format_supported(struct sw_winsys* winsys,
	unsigned textureUsage, enum pipe_format format)
{
	// TODO STUB
	return true;
}

static color_space
hgl_winsys_convert_cs(enum pipe_format format)
{
	// TODO: B_RGB24, B_RGB16, B_RGB15?
	switch(format) {
		case PIPE_FORMAT_B5G6R5_UNORM:
			return B_CMAP8;
		case PIPE_FORMAT_A8B8G8R8_UNORM:
		case PIPE_FORMAT_X8B8G8R8_UNORM:
		default:
			return B_RGB32;
	}
}

static struct sw_displaytarget*
hgl_winsys_displaytarget_create(struct sw_winsys* winsys,
	unsigned textureUsage, enum pipe_format format, unsigned width,
	unsigned height, unsigned alignment, const void *front_private,
	unsigned* stride)
{
	struct haiku_displaytarget* haikuDisplayTarget
		= CALLOC_STRUCT(haiku_displaytarget);
	assert(haikuDisplayTarget);

	TRACE("%s: %d x %d\n", __func__, width, height);

	haikuDisplayTarget->colorSpace = hgl_winsys_convert_cs(format);
	haikuDisplayTarget->format = format;
	haikuDisplayTarget->width = width;
	haikuDisplayTarget->height = height;

	size_t formatStride = util_format_get_stride(format, width);
	unsigned blockSize = util_format_get_nblocksy(format, height);

	haikuDisplayTarget->stride = align(formatStride, alignment);
	haikuDisplayTarget->size = haikuDisplayTarget->stride * blockSize;

	if (textureUsage & PIPE_BIND_DISPLAY_TARGET) {
		haikuDisplayTarget->data = NULL;
		haikuDisplayTarget->bitmap = new BBitmap(
			BRect(0, 0, width - 1, height - 1),
			0,
			haikuDisplayTarget->colorSpace,
			haikuDisplayTarget->stride);
	} else {
		haikuDisplayTarget->data
			= align_malloc(haikuDisplayTarget->size, alignment);

		haikuDisplayTarget->bitmap = NULL;
	}

	*stride = haikuDisplayTarget->stride;

	// Cast to ghost sw_displaytarget type
	return (struct sw_displaytarget*)haikuDisplayTarget;
}


static void
hgl_winsys_displaytarget_destroy(struct sw_winsys* winsys,
	struct sw_displaytarget* displayTarget)
{
	struct haiku_displaytarget* haikuDisplayTarget
		= hgl_sw_displaytarget(displayTarget);

	if (!haikuDisplayTarget)
		return;

	if (haikuDisplayTarget->data != NULL)
		align_free(haikuDisplayTarget->data);

	if (haikuDisplayTarget->bitmap != NULL)
		delete haikuDisplayTarget->bitmap;

	FREE(haikuDisplayTarget);
}


static struct sw_displaytarget*
hgl_winsys_displaytarget_from_handle(struct sw_winsys* winsys,
	const struct pipe_resource* templat, struct winsys_handle* whandle,
	unsigned* stride)
{
	return NULL;
}


static bool
hgl_winsys_displaytarget_get_handle(struct sw_winsys* winsys,
	struct sw_displaytarget* displayTarget, struct winsys_handle* whandle)
{
	return false;
}


static void*
hgl_winsys_displaytarget_map(struct sw_winsys* winsys,
	struct sw_displaytarget* displayTarget, unsigned flags)
{
	struct haiku_displaytarget* haikuDisplayTarget
		= hgl_sw_displaytarget(displayTarget);

	if (haikuDisplayTarget->bitmap != NULL)
		return haikuDisplayTarget->bitmap->Bits();

	return haikuDisplayTarget->data;
}


static void
hgl_winsys_displaytarget_unmap(struct sw_winsys* winsys,
	struct sw_displaytarget* displayTarget)
{
	return;
}


static void
hgl_winsys_displaytarget_display(struct sw_winsys* winsys,
	struct sw_displaytarget* displayTarget, void* contextPrivate,
	struct pipe_box *box)
{
	assert(contextPrivate);

	struct haiku_displaytarget* haikuDisplayTarget
		= hgl_sw_displaytarget(displayTarget);

	BitmapHook *context = (BitmapHook*)contextPrivate;
	context->SetBitmap(haikuDisplayTarget->bitmap);
}


struct sw_winsys*
hgl_create_sw_winsys()
{
	struct sw_winsys* winsys = CALLOC_STRUCT(sw_winsys);

	if (!winsys)
		return NULL;

	// Attach winsys hooks for Haiku
	winsys->destroy = hgl_winsys_destroy;
	winsys->is_displaytarget_format_supported
		= hgl_winsys_is_displaytarget_format_supported;
	winsys->displaytarget_create = hgl_winsys_displaytarget_create;
	winsys->displaytarget_from_handle = hgl_winsys_displaytarget_from_handle;
	winsys->displaytarget_get_handle = hgl_winsys_displaytarget_get_handle;
	winsys->displaytarget_map = hgl_winsys_displaytarget_map;
	winsys->displaytarget_unmap = hgl_winsys_displaytarget_unmap;
	winsys->displaytarget_display = hgl_winsys_displaytarget_display;
	winsys->displaytarget_destroy = hgl_winsys_displaytarget_destroy;

	return winsys;
}
