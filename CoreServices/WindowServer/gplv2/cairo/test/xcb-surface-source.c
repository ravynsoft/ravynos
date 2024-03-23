/*
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"
#if CAIRO_HAS_XCB_SURFACE
#include <cairo-xcb.h>
#endif

#include "surface-source.c"

#if CAIRO_HAS_XCB_SURFACE
static cairo_user_data_key_t closure_key;

struct closure {
    cairo_device_t *device;
    xcb_connection_t *connection;
    xcb_pixmap_t pixmap;
};

static void
cleanup (void *data)
{
    struct closure *arg = data;

    cairo_device_finish (arg->device);
    cairo_device_destroy (arg->device);

    xcb_free_pixmap (arg->connection, arg->pixmap);
    xcb_disconnect (arg->connection);

    free (arg);
}

static xcb_render_pictforminfo_t *
find_depth (xcb_connection_t *connection, int depth, void **formats_out)
{
    xcb_render_query_pict_formats_reply_t	*formats;
    xcb_render_query_pict_formats_cookie_t cookie;
    xcb_render_pictforminfo_iterator_t i;

    cookie = xcb_render_query_pict_formats (connection);
    xcb_flush (connection);

    formats = xcb_render_query_pict_formats_reply (connection, cookie, 0);
    if (formats == NULL)
	return NULL;

    for (i = xcb_render_query_pict_formats_formats_iterator (formats);
	 i.rem;
	 xcb_render_pictforminfo_next (&i))
    {
	if (XCB_RENDER_PICT_TYPE_DIRECT != i.data->type)
	    continue;

	if (depth != i.data->depth)
	    continue;

	*formats_out = formats;
	return i.data;
    }

    free (formats);
    return NULL;
}
#endif

static cairo_surface_t *
create_source_surface (int size)
{
#if CAIRO_HAS_XCB_SURFACE
    xcb_connection_t *connection;
    xcb_render_pictforminfo_t *render_format;
    struct closure *data;
    cairo_surface_t *surface;
    xcb_screen_t *root;
    xcb_void_cookie_t cookie;
    void *formats;

    connection = xcb_connect (NULL, NULL);
    if (connection == NULL)
	return NULL;

    data = xmalloc (sizeof (struct closure));
    data->connection = connection;

    render_format = find_depth (connection, 32, &formats);
    if (render_format == NULL) {
	xcb_disconnect (connection);
	free (data);
	return NULL;
    }

    root = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;

    data->pixmap = xcb_generate_id (connection);
    cookie = xcb_create_pixmap_checked (connection, 32,
					data->pixmap, root->root, size, size);
    /* slow, but sure */
    if (xcb_request_check (connection, cookie) != NULL) {
	free (formats);
	xcb_disconnect (connection);
	free (data);
	return NULL;
    }

    surface = cairo_xcb_surface_create_with_xrender_format (connection,
							    root,
							    data->pixmap,
							    render_format,
							    size, size);
    free (formats);

    data->device = cairo_device_reference (cairo_surface_get_device (surface));
    cairo_surface_set_user_data (surface, &closure_key, data, cleanup);

    return surface;
#else
    return NULL;
#endif
}

CAIRO_TEST (xcb_surface_source,
	    "Test using a XCB surface as the source",
	    "source", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    preamble, draw)
