/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2004,2006 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-boilerplate-private.h"

#include <cairo-win32.h>

#include <errno.h>
#include <limits.h>

static const cairo_user_data_key_t win32_closure_key;

typedef struct _win32_target_closure {
    HWND wnd;
    HDC dc;
    ATOM bpl_atom;
    cairo_surface_t *surface;
} win32_target_closure_t;

static void
_cairo_boilerplate_win32_cleanup_window_surface (void *closure)
{
    win32_target_closure_t *win32tc = closure;

    if (win32tc != NULL)
    {
	if (win32tc->wnd != NULL &&
	    ReleaseDC (win32tc->wnd, win32tc->dc) != 1)
	    fprintf (stderr,
		     "Failed to release DC of a test window when cleaning up.\n");
	if (win32tc->wnd != NULL &&
	    DestroyWindow (win32tc->wnd) == 0)
	    fprintf (stderr,
		     "Failed to destroy a test window when cleaning up, GLE is %lu.\n",
		     GetLastError ());
	if (win32tc->bpl_atom != 0 &&
	    UnregisterClassA ((LPCSTR) MAKELPARAM (win32tc->bpl_atom, 0), GetModuleHandle (NULL)) == 0 &&
	    GetLastError () != ERROR_CLASS_DOES_NOT_EXIST)
	    fprintf (stderr,
		     "Failed to unregister boilerplate window class, GLE is %lu.\n",
		     GetLastError ());

	free (win32tc);
    }
}

static win32_target_closure_t *
_cairo_boilerplate_win32_create_window (int width,
					int height)
{
    WNDCLASSEXA wincl;
    win32_target_closure_t *win32tc;
    LPCSTR window_class_name;

    ZeroMemory (&wincl, sizeof (WNDCLASSEXA));
    wincl.cbSize = sizeof (WNDCLASSEXA);
    wincl.hInstance = GetModuleHandle (0);
    wincl.lpszClassName = "cairo_boilerplate_win32_dummy";
    wincl.lpfnWndProc = DefWindowProcA;
    wincl.style = CS_OWNDC;

    win32tc = calloc (1, sizeof (win32_target_closure_t));

    if (win32tc == NULL)
    {
	int error = errno;
	fprintf (stderr, "Ran out of memory: %d.\n", error);
	return NULL;
    }

    ZeroMemory (win32tc, sizeof (win32_target_closure_t));

    win32tc->bpl_atom = RegisterClassExA (&wincl);

    if (win32tc->bpl_atom == 0 && GetLastError () != ERROR_CLASS_ALREADY_EXISTS)
    {
	fprintf (stderr,
		 "Failed to register a boilerplate window class, GLE is %lu.\n",
		 GetLastError ());
	_cairo_boilerplate_win32_cleanup_window_surface (win32tc);
	return NULL;
    }

    if (win32tc->bpl_atom == 0)
	window_class_name = wincl.lpszClassName;
    else
	window_class_name = (LPCSTR) MAKELPARAM (win32tc->bpl_atom, 0);

    win32tc->wnd = CreateWindowExA (WS_EX_TOOLWINDOW,
				    window_class_name,
				    0,
				    WS_POPUP,
				    0,
				    0,
				    width,
				    height,
				    0,
				    0,
				    0,
				    0);

    if (win32tc->wnd == NULL)
    {
	fprintf (stderr,
		 "Failed to create a test window, GLE is %lu.\n",
		 GetLastError ());
	_cairo_boilerplate_win32_cleanup_window_surface (win32tc);
	return NULL;
    }

    win32tc->dc = GetDC (win32tc->wnd);

    if (win32tc->dc == NULL)
    {
	fprintf (stderr, "Failed to get test window DC.\n");
	_cairo_boilerplate_win32_cleanup_window_surface (win32tc);
	return NULL;
    }

    SetWindowPos (win32tc->wnd,
		  HWND_BOTTOM,
		  INT_MIN,
		  INT_MIN,
		  width,
		  height,
		  SWP_NOACTIVATE | SWP_SHOWWINDOW);

    return win32tc;
}

static cairo_surface_t *
_cairo_boilerplate_win32_create_window_surface (const char		  *name,
						cairo_content_t		   content,
						double			   width,
						double			   height,
						double			   max_width,
						double			   max_height,
						cairo_boilerplate_mode_t   mode,
						void			 **closure)
{
    win32_target_closure_t *win32tc;
    cairo_surface_t *surface;
    cairo_format_t format;
    cairo_status_t status;

    win32tc = _cairo_boilerplate_win32_create_window (width, height);

    if (win32tc == NULL)
	return NULL;

    format = cairo_boilerplate_format_from_content (content);

    surface = cairo_win32_surface_create_with_format (win32tc->dc, format);

    win32tc->surface = surface;

    status = cairo_surface_status (surface);

    if (status != CAIRO_STATUS_SUCCESS)
    {
	fprintf (stderr,
		 "Failed to create the test surface: %s [%d].\n",
		 cairo_status_to_string (status), status);
	_cairo_boilerplate_win32_cleanup_window_surface (win32tc);
	return NULL;
    }

    status = cairo_surface_set_user_data (surface, &win32_closure_key, win32tc, NULL);

    if (status != CAIRO_STATUS_SUCCESS)
    {
	fprintf (stderr,
		 "Failed to set surface userdata: %s [%d].\n",
		 cairo_status_to_string (status), status);

	cairo_surface_destroy (surface);
	_cairo_boilerplate_win32_cleanup_window_surface (win32tc);

	return NULL;
    }

    *closure = win32tc;

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_win32_create_dib_surface (const char		       *name,
					     cairo_content_t		content,
					     double 			width,
					     double 			height,
					     double 			max_width,
					     double 			max_height,
					     cairo_boilerplate_mode_t   mode,
					     void		      **closure)
{
    cairo_format_t format;

    format = cairo_boilerplate_format_from_content (content);

    *closure = NULL;

    return cairo_win32_surface_create_with_dib (format, width, height);
}

static const cairo_boilerplate_target_t targets[] = {
    {
	"win32", "win32", NULL, NULL,
	CAIRO_SURFACE_TYPE_WIN32, CAIRO_CONTENT_COLOR, 0,
	"cairo_win32_surface_create_with_dib",
	_cairo_boilerplate_win32_create_dib_surface,
	cairo_surface_create_similar,
	NULL,
	NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL,
	NULL,
	NULL,
	TRUE, FALSE, FALSE
    },
    /* Testing the win32 surface isn't interesting, since for
     * ARGB images it just chains to the image backend
     */
    {
	"win32", "win32", NULL, NULL,
	CAIRO_SURFACE_TYPE_WIN32, CAIRO_CONTENT_COLOR_ALPHA, 0,
	"cairo_win32_surface_create_with_dib",
	_cairo_boilerplate_win32_create_dib_surface,
	cairo_surface_create_similar,
	NULL,
	NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL,
	NULL,
	NULL,
	FALSE, FALSE, FALSE
    },
    {
	"win32-window-color", "win32", NULL, NULL,
	CAIRO_SURFACE_TYPE_WIN32, CAIRO_CONTENT_COLOR, 1,
	"cairo_win32_surface_create",
	_cairo_boilerplate_win32_create_window_surface,
	cairo_surface_create_similar,
	NULL,
	NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_win32_cleanup_window_surface,
	NULL,
	NULL,
	FALSE, FALSE, FALSE
    },
    {
	"win32-window-coloralpha", "win32", NULL, NULL,
	CAIRO_SURFACE_TYPE_WIN32, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_win32_surface_create_with_format",
	_cairo_boilerplate_win32_create_window_surface,
	cairo_surface_create_similar,
	NULL,
	NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_win32_cleanup_window_surface,
	NULL,
	NULL,
	FALSE, FALSE, FALSE
    },
};
CAIRO_BOILERPLATE (win32, targets)
