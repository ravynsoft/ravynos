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
#include "cairo-boilerplate-scaled-font.h"
#include "cairo-malloc-private.h"

#include <pixman.h>

#include <cairo-ctype-inline.h>
#include <cairo-types-private.h>
#include <cairo-scaled-font-private.h>

#if CAIRO_HAS_SCRIPT_SURFACE
#include <cairo-script.h>
#endif

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#if HAVE_UNISTD_H && HAVE_FCNTL_H && HAVE_SIGNAL_H && HAVE_SYS_STAT_H && HAVE_SYS_SOCKET_H && HAVE_SYS_UN_H
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#define HAS_DAEMON 1
#define SOCKET_PATH "./.any2ppm"
#endif

cairo_content_t
cairo_boilerplate_content (cairo_content_t content)
{
    if (content == CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED)
	content = CAIRO_CONTENT_COLOR_ALPHA;

    return content;
}

const char *
cairo_boilerplate_content_name (cairo_content_t content)
{
    /* For the purpose of the content name, we don't distinguish the
     * flattened content value.
     */
    switch (cairo_boilerplate_content (content)) {
    case CAIRO_CONTENT_COLOR:
	return "rgb24";
    case CAIRO_CONTENT_COLOR_ALPHA:
	return "argb32";
    case CAIRO_CONTENT_ALPHA:
    default:
	assert (0); /* not reached */
	return "---";
    }
}

static const char *
_cairo_boilerplate_content_visible_name (cairo_content_t content)
{
    switch (cairo_boilerplate_content (content)) {
    case CAIRO_CONTENT_COLOR:
	return "rgb";
    case CAIRO_CONTENT_COLOR_ALPHA:
	return "rgba";
    case CAIRO_CONTENT_ALPHA:
	return "a";
    default:
	assert (0); /* not reached */
	return "---";
    }
}

cairo_format_t
cairo_boilerplate_format_from_content (cairo_content_t content)
{
    cairo_format_t format;

    switch (content) {
    case CAIRO_CONTENT_COLOR:
        format = CAIRO_FORMAT_RGB24;
        break;
    case CAIRO_CONTENT_COLOR_ALPHA:
        format = CAIRO_FORMAT_ARGB32;
        break;
    case CAIRO_CONTENT_ALPHA:
        format = CAIRO_FORMAT_A8;
        break;
    default:
        assert (0); /* not reached */
        format = CAIRO_FORMAT_INVALID;
        break;
    }

    return format;
}

static cairo_surface_t *
_cairo_boilerplate_image_create_surface (const char		   *name,
					 cairo_content_t	    content,
					 double 		    width,
					 double 		    height,
					 double 		    max_width,
					 double 		    max_height,
					 cairo_boilerplate_mode_t   mode,
					 void			  **closure)
{
    cairo_format_t format;

    *closure = NULL;

    if (content == CAIRO_CONTENT_COLOR_ALPHA) {
	format = CAIRO_FORMAT_ARGB32;
    } else if (content == CAIRO_CONTENT_COLOR) {
	format = CAIRO_FORMAT_RGB24;
    } else {
	assert (0); /* not reached */
	return NULL;
    }

    return cairo_image_surface_create (format, ceil (width), ceil (height));
}

static const cairo_user_data_key_t key;

static cairo_surface_t *
_cairo_boilerplate_image_create_similar (cairo_surface_t *other,
					 cairo_content_t content,
					 int width, int height)
{
    cairo_format_t format;
    cairo_surface_t *surface;
    int stride;
    void *ptr;

    switch (content) {
    case CAIRO_CONTENT_ALPHA:
        format = CAIRO_FORMAT_A8;
        break;
    case CAIRO_CONTENT_COLOR:
        format = CAIRO_FORMAT_RGB24;
        break;
    case CAIRO_CONTENT_COLOR_ALPHA:
    default:
        format = CAIRO_FORMAT_ARGB32;
        break;
    }

    stride = cairo_format_stride_for_width(format, width);
    ptr = _cairo_malloc (stride * height);

    surface = cairo_image_surface_create_for_data (ptr, format,
						   width, height, stride);
    cairo_surface_set_user_data (surface, &key, ptr, free);

    return surface;
}

static cairo_surface_t *
_cairo_boilerplate_image16_create_surface (const char		     *name,
					   cairo_content_t	      content,
					   double		      width,
					   double		      height,
					   double		      max_width,
					   double		      max_height,
					   cairo_boilerplate_mode_t   mode,
					   void			    **closure)
{
    *closure = NULL;

    /* XXX force CAIRO_CONTENT_COLOR */
    return cairo_image_surface_create (CAIRO_FORMAT_RGB16_565, ceil (width), ceil (height));
}

static cairo_surface_t *
_cairo_boilerplate_image16_create_similar (cairo_surface_t *other,
					   cairo_content_t content,
					   int width, int height)
{
    cairo_format_t format;
    cairo_surface_t *surface;
    int stride;
    void *ptr;

    switch (content) {
    case CAIRO_CONTENT_ALPHA:
        format = CAIRO_FORMAT_A8;
        break;
    case CAIRO_CONTENT_COLOR:
        format = CAIRO_FORMAT_RGB16_565;
        break;
    case CAIRO_CONTENT_COLOR_ALPHA:
    default:
        format = CAIRO_FORMAT_ARGB32;
        break;
    }

    stride = cairo_format_stride_for_width(format, width);
    ptr = _cairo_malloc (stride * height);

    surface = cairo_image_surface_create_for_data (ptr, format,
						   width, height, stride);
    cairo_surface_set_user_data (surface, &key, ptr, free);

    return surface;
}

static char *
_cairo_boilerplate_image_describe (void *closure)
{
    char *s;

    xasprintf (&s, "pixman %s", pixman_version_string ());

    return s;
}

#if CAIRO_HAS_RECORDING_SURFACE
static cairo_surface_t *
_cairo_boilerplate_recording_create_surface (const char 	       *name,
					     cairo_content_t		content,
					     double			width,
					     double			height,
					     double			max_width,
					     double			max_height,
					     cairo_boilerplate_mode_t	mode,
					     void		      **closure)
{
    cairo_rectangle_t extents;

    extents.x = 0;
    extents.y = 0;
    extents.width = width;
    extents.height = height;
    return *closure = cairo_surface_reference (cairo_recording_surface_create (content, &extents));
}

static void
_cairo_boilerplate_recording_surface_cleanup (void *closure)
{
    cairo_surface_finish (closure);
    cairo_surface_destroy (closure);
}
#endif

const cairo_user_data_key_t cairo_boilerplate_output_basename_key;

cairo_surface_t *
_cairo_boilerplate_get_image_surface (cairo_surface_t *src,
				      int	       page,
				      int	       width,
				      int	       height)
{
    cairo_surface_t *surface, *image;
    cairo_t *cr;
    cairo_status_t status;
    cairo_format_t format;

    if (cairo_surface_status (src))
	return cairo_surface_reference (src);

    if (page != 0)
	return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    /* extract sub-surface */
    switch (cairo_surface_get_content (src)) {
    case CAIRO_CONTENT_ALPHA:
	format = CAIRO_FORMAT_A8;
	break;
    case CAIRO_CONTENT_COLOR:
	format = CAIRO_FORMAT_RGB24;
	break;
    default:
    case CAIRO_CONTENT_COLOR_ALPHA:
	format = CAIRO_FORMAT_ARGB32;
	break;
    }
    surface = cairo_image_surface_create (format, width, height);
    assert (cairo_surface_get_content (surface) == cairo_surface_get_content (src));
    image = cairo_surface_reference (surface);

    /* open a logging channel (only interesting for recording surfaces) */
#if CAIRO_HAS_SCRIPT_SURFACE && CAIRO_HAS_RECORDING_SURFACE
    if (cairo_surface_get_type (src) == CAIRO_SURFACE_TYPE_RECORDING) {
	const char *test_name;

	test_name = cairo_surface_get_user_data (src,
						 &cairo_boilerplate_output_basename_key);
	if (test_name != NULL) {
	    cairo_device_t *ctx;
	    char *filename;

	    cairo_surface_destroy (surface);

	    xasprintf (&filename, "%s.out.trace", test_name);
	    ctx = cairo_script_create (filename);
	    surface = cairo_script_surface_create_for_target (ctx, image);
	    cairo_device_destroy (ctx);
	    free (filename);
	}
    }
#endif

    cr = cairo_create (surface);
    cairo_surface_destroy (surface);
    cairo_set_source_surface (cr, src, 0, 0);
    cairo_paint (cr);

    status = cairo_status (cr);
    if (status) {
	cairo_surface_destroy (image);
	image = cairo_surface_reference (cairo_get_target (cr));
    }
    cairo_destroy (cr);

    return image;
}

cairo_surface_t *
cairo_boilerplate_get_image_surface_from_png (const char   *filename,
					      int	    width,
					      int	    height,
					      cairo_bool_t  flatten)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface))
	return surface;

    if (flatten) {
	cairo_t *cr;
	cairo_surface_t *flattened;

	flattened = cairo_image_surface_create (cairo_image_surface_get_format (surface),
						width,
						height);
	cr = cairo_create (flattened);
	cairo_surface_destroy (flattened);

	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_paint (cr);

	cairo_set_source_surface (cr, surface,
				  width - cairo_image_surface_get_width (surface),
				  height - cairo_image_surface_get_height (surface));
	cairo_paint (cr);

	cairo_surface_destroy (surface);
	surface = cairo_surface_reference (cairo_get_target (cr));
	cairo_destroy (cr);
    } else if (cairo_image_surface_get_width (surface) != width ||
	       cairo_image_surface_get_height (surface) != height)
    {
	cairo_t *cr;
	cairo_surface_t *sub;

	sub = cairo_image_surface_create (cairo_image_surface_get_format (surface),
					  width,
					  height);
	cr = cairo_create (sub);
	cairo_surface_destroy (sub);

	cairo_set_source_surface (cr, surface,
				  width - cairo_image_surface_get_width (surface),
				  height - cairo_image_surface_get_height (surface));
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);

	cairo_surface_destroy (surface);
	surface = cairo_surface_reference (cairo_get_target (cr));
	cairo_destroy (cr);
    }

    return surface;
}

static const cairo_boilerplate_target_t builtin_targets[] = {
    /* I'm uncompromising about leaving the image backend as 0
     * for tolerance. There shouldn't ever be anything that is out of
     * our control here. */
    {
	"image", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE, CAIRO_CONTENT_COLOR_ALPHA, 0,
	NULL,
	_cairo_boilerplate_image_create_surface,
	_cairo_boilerplate_image_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL,
        _cairo_boilerplate_image_describe,
	TRUE, FALSE, FALSE
    },
    {
	"image", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE, CAIRO_CONTENT_COLOR, 0,
	NULL,
	_cairo_boilerplate_image_create_surface,
	_cairo_boilerplate_image_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL,
        _cairo_boilerplate_image_describe,
	FALSE, FALSE, FALSE
    },
    {
	"image16", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_IMAGE, CAIRO_CONTENT_COLOR, 0,
	NULL,
	_cairo_boilerplate_image16_create_surface,
	_cairo_boilerplate_image16_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	NULL, NULL,
        _cairo_boilerplate_image_describe,
	TRUE, FALSE, FALSE
    },
#if CAIRO_HAS_RECORDING_SURFACE
    {
	"recording", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_RECORDING, CAIRO_CONTENT_COLOR_ALPHA, 0,
	"cairo_recording_surface_create",
	_cairo_boilerplate_recording_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_recording_surface_cleanup,
	NULL, NULL,
	FALSE, FALSE, TRUE
    },
    {
	"recording", "image", NULL, NULL,
	CAIRO_SURFACE_TYPE_RECORDING, CAIRO_CONTENT_COLOR, 0,
	"cairo_recording_surface_create",
	_cairo_boilerplate_recording_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_recording_surface_cleanup,
	NULL, NULL,
	FALSE, FALSE, TRUE
    },
#endif
};
CAIRO_BOILERPLATE (builtin, builtin_targets)

static struct cairo_boilerplate_target_list {
    struct cairo_boilerplate_target_list *next;
    const cairo_boilerplate_target_t *target;
} *cairo_boilerplate_targets;

static cairo_bool_t
probe_target (const cairo_boilerplate_target_t *target)
{
    if (target->probe == NULL)
	return TRUE;

#if HAVE_DLSYM
    return dlsym (NULL, target->probe) != NULL;
#else
    return TRUE;
#endif
}

void
_cairo_boilerplate_register_backend (const cairo_boilerplate_target_t *targets,
				     unsigned int		       count)
{
    targets += count;
    while (count--) {
	struct cairo_boilerplate_target_list *list;

	--targets;
	if (! probe_target (targets))
	    continue;

	list = xmalloc (sizeof (*list));
	list->next = cairo_boilerplate_targets;
	list->target = targets;
	cairo_boilerplate_targets = list;
    }
}

static cairo_bool_t
_cairo_boilerplate_target_format_matches_name (const cairo_boilerplate_target_t *target,
					const char *tcontent_name,
					const char *tcontent_end)
{
	char const *content_name;
	const char *content_end = tcontent_end;
	size_t content_len;

	content_name = _cairo_boilerplate_content_visible_name (target->content);
	if (tcontent_end)
		content_len = content_end - tcontent_name;
	else
		content_len = strlen(tcontent_name);
	if (strlen(content_name) != content_len)
		return FALSE;
	if (0 == strncmp (content_name, tcontent_name, content_len))
		return TRUE;

	return FALSE;
}

static cairo_bool_t
_cairo_boilerplate_target_matches_name (const cairo_boilerplate_target_t *target,
					const char			 *tname,
					const char			 *end)
{
    char const *content_name;
    const char *content_start = strpbrk (tname, ".");
    const char *content_end = end;
    size_t name_len;
    size_t content_len;

    if (content_start >= end)
	content_start = NULL;
    if (content_start != NULL)
	end = content_start++;

    name_len = end - tname;

    /* Check name. */
    if (! (name_len == 1 && 0 == strncmp (tname, "?", 1))) { /* wildcard? */
	if (0 != strncmp (target->name, tname, name_len)) /* exact match? */
	    return FALSE;
	if (_cairo_isalnum (target->name[name_len]))
	    return FALSE;
    }

    /* Check optional content. */
    if (content_start == NULL)	/* none given? */
	return TRUE;

    /* Exact content match? */
    content_name = _cairo_boilerplate_content_visible_name (target->content);
    content_len = content_end - content_start;
    if (strlen(content_name) != content_len)
	return FALSE;
    if (0 == strncmp (content_name, content_start, content_len))
	return TRUE;

    return FALSE;
}

const cairo_boilerplate_target_t **
cairo_boilerplate_get_targets (int	    *pnum_targets,
			       cairo_bool_t *plimited_targets)
{
    size_t i, num_targets;
    cairo_bool_t limited_targets = FALSE;
    const char *tname;
    const cairo_boilerplate_target_t **targets_to_test;
    struct cairo_boilerplate_target_list *list;

    if (cairo_boilerplate_targets == NULL)
	_cairo_boilerplate_register_all ();

    if ((tname = getenv ("CAIRO_TEST_TARGET")) != NULL && *tname) {
	/* check the list of targets specified by the user */
	limited_targets = TRUE;

	num_targets = 0;
	targets_to_test = NULL;

	while (*tname) {
	    int found = 0;
	    const char *end = strpbrk (tname, " \t\r\n;:,");
	    if (!end)
		end = tname + strlen (tname);

	    if (end == tname) {
		tname = end + 1;
		continue;
	    }

	    for (list = cairo_boilerplate_targets;
		 list != NULL;
		 list = list->next)
	    {
		    const cairo_boilerplate_target_t *target = list->target;
		    const char *tcontent_name;
		    const char *tcontent_end;
		    if (_cairo_boilerplate_target_matches_name (target, tname, end)) {
			    if ((tcontent_name = getenv ("CAIRO_TEST_TARGET_FORMAT")) != NULL && *tcontent_name) {
				    while(tcontent_name) {
					    tcontent_end = strpbrk (tcontent_name, " \t\r\n;:,");
					    if (tcontent_end == tcontent_name) {
						    tcontent_name = tcontent_end + 1;
						    continue;
					    }
					    if(_cairo_boilerplate_target_format_matches_name (target,
								    tcontent_name, tcontent_end)) {
						    /* realloc isn't exactly the best thing here, but meh. */
						    targets_to_test = xrealloc (targets_to_test,
								    sizeof(cairo_boilerplate_target_t *) * (num_targets+1));
						    targets_to_test[num_targets++] = target;
						    found = 1;
					    }

					    if (tcontent_end)
						    tcontent_end++;
					    tcontent_name = tcontent_end;
				    }
			    } else {
				    /* realloc isn't exactly the best thing here, but meh. */
				    targets_to_test = xrealloc (targets_to_test,
						    sizeof(cairo_boilerplate_target_t *) * (num_targets+1));
				    targets_to_test[num_targets++] = target;
				    found = 1;
			    }
		    }
	    }

	    if (!found) {
		const char *last_name = NULL;

		fprintf (stderr, "Cannot find target '%.*s'.\n",
			 (int)(end - tname), tname);
		fprintf (stderr, "Known targets:");
		for (list = cairo_boilerplate_targets;
		     list != NULL;
		     list = list->next)
		{
		    const cairo_boilerplate_target_t *target = list->target;
		    if (last_name != NULL) {
			if (strcmp (target->name, last_name) == 0) {
			    /* filter out repeats that differ in content */
			    continue;
			}
			fprintf (stderr, ",");
		    }
		    fprintf (stderr, " %s", target->name);
		    last_name = target->name;
		}
		fprintf (stderr, "\n");
		exit(-1);
	    }

	    if (*end)
	      end++;
	    tname = end;
	}
    } else {
	    int found = 0;
	    int not_found_targets = 0;
	    num_targets = 0;
	    targets_to_test = xmalloc (sizeof(cairo_boilerplate_target_t*) * num_targets);
	    for (list = cairo_boilerplate_targets; list != NULL; list = list->next)
	    {
		    const cairo_boilerplate_target_t *target = list->target;
		    const char *tcontent_name;
		    const char *tcontent_end;
		    if ((tcontent_name = getenv ("CAIRO_TEST_TARGET_FORMAT")) != NULL && *tcontent_name) {
			    while(tcontent_name) {
				    tcontent_end = strpbrk (tcontent_name, " \t\r\n;:,");
				    if (tcontent_end == tcontent_name) {
					    tcontent_name = tcontent_end + 1;
					    continue;
				    }
				    if (_cairo_boilerplate_target_format_matches_name (target,
							    tcontent_name, tcontent_end)) {
					    /* realloc isn't exactly the best thing here, but meh. */
					    targets_to_test = xrealloc (targets_to_test,
							    sizeof(cairo_boilerplate_target_t *) * (num_targets+1));
					    targets_to_test[num_targets++] = target;
					    found =1;
				    }
				    else
				    {
					    not_found_targets++;
				    }

				    if (tcontent_end)
					    tcontent_end++;

				    tcontent_name = tcontent_end;
			    }
		    }
		    else
		    {
			    num_targets++;
		    }
	    }
	    if (!found)
	    {
		    /* check all compiled in targets */
		    num_targets = num_targets + not_found_targets;
		    targets_to_test = xrealloc (targets_to_test,
				    sizeof(cairo_boilerplate_target_t*) * num_targets);
		    num_targets = 0;
		    for (list = cairo_boilerplate_targets;
				    list != NULL;
				    list = list->next)
		    {
			    const cairo_boilerplate_target_t *target = list->target;
			    targets_to_test[num_targets++] = target;
		    }
	    }

    }

    /* exclude targets as specified by the user */
    if ((tname = getenv ("CAIRO_TEST_TARGET_EXCLUDE")) != NULL && *tname) {
	limited_targets = TRUE;

	while (*tname) {
	    int j;
	    const char *end = strpbrk (tname, " \t\r\n;:,");
	    if (!end)
		end = tname + strlen (tname);

	    if (end == tname) {
		tname = end + 1;
		continue;
	    }

	    for (i = j = 0; i < num_targets; i++) {
		const cairo_boilerplate_target_t *target = targets_to_test[i];
		if (! _cairo_boilerplate_target_matches_name (target,
							      tname, end))
		{
		    targets_to_test[j++] = targets_to_test[i];
		}
	    }
	    num_targets = j;

	    if (*end)
	      end++;
	    tname = end;
	}
    }

    if (pnum_targets)
	*pnum_targets = num_targets;

    if (plimited_targets)
	*plimited_targets = limited_targets;

    return targets_to_test;
}

const cairo_boilerplate_target_t *
cairo_boilerplate_get_image_target (cairo_content_t content)
{
    if (cairo_boilerplate_targets == NULL)
	_cairo_boilerplate_register_all ();

    switch (content) {
    case CAIRO_CONTENT_COLOR:
        return &builtin_targets[1];
    case CAIRO_CONTENT_COLOR_ALPHA:
        return &builtin_targets[0];
    case CAIRO_CONTENT_ALPHA:
    default:
        return NULL;
    }
}

const cairo_boilerplate_target_t *
cairo_boilerplate_get_target_by_name (const char      *name,
				      cairo_content_t  content)
{
    struct cairo_boilerplate_target_list *list;

    if (cairo_boilerplate_targets == NULL)
	_cairo_boilerplate_register_all ();

    /* first return an exact match */
    for (list = cairo_boilerplate_targets; list != NULL; list = list->next) {
	const cairo_boilerplate_target_t *target = list->target;
	if (strcmp (target->name, name) == 0 &&
	    target->content == content)
	{
	    return target;
	}
    }

    /* otherwise just return a match that may differ in content */
    for (list = cairo_boilerplate_targets; list != NULL; list = list->next) {
	const cairo_boilerplate_target_t *target = list->target;
	if (strcmp (target->name, name) == 0)
	    return target;
    }

    return NULL;
}

void
cairo_boilerplate_free_targets (const cairo_boilerplate_target_t **targets)
{
    free (targets);
}

cairo_surface_t *
cairo_boilerplate_surface_create_in_error (cairo_status_t status)
{
    cairo_surface_t *surface = NULL;
    int loop = 5;

    do {
	cairo_surface_t *intermediate;
	cairo_t *cr;
	cairo_path_t path;

	intermediate = cairo_image_surface_create (CAIRO_FORMAT_A8, 0, 0);
	cr = cairo_create (intermediate);
	cairo_surface_destroy (intermediate);

	path.status = status;
	cairo_append_path (cr, &path);

	cairo_surface_destroy (surface);
	surface = cairo_surface_reference (cairo_get_target (cr));
	cairo_destroy (cr);
    } while (cairo_surface_status (surface) != status && --loop);

    return surface;
}

void
cairo_boilerplate_scaled_font_set_max_glyphs_cached (cairo_scaled_font_t *scaled_font,
						     int		  max_glyphs)
{
    /* XXX CAIRO_DEBUG */
}

#if HAS_DAEMON
static int
any2ppm_daemon_exists (void)
{
    struct stat st;
    int fd;
    char buf[80];
    int pid;
    int ret;

    if (stat (SOCKET_PATH, &st) < 0)
	return 0;

    fd = open (SOCKET_PATH ".pid", O_RDONLY);
    if (fd < 0)
	return 0;

    pid = 0;
    ret = read (fd, buf, sizeof (buf) - 1);
    if (ret > 0) {
	buf[ret] = '\0';
	pid = atoi (buf);
    }
    close (fd);

    return pid > 0 && kill (pid, 0) != -1;
}
#endif

FILE *
cairo_boilerplate_open_any2ppm (const char   *filename,
				int	      page,
				unsigned int  flags,
				int        (**close_cb) (FILE *))
{
    char command[4096];
    const char *any2ppm;
#if HAS_DAEMON
    int sk;
    struct sockaddr_un addr;
    int len;
#endif

    any2ppm = getenv ("ANY2PPM");
    if (any2ppm == NULL)
	any2ppm = "./any2ppm";

#if HAS_DAEMON
    if (flags & CAIRO_BOILERPLATE_OPEN_NO_DAEMON)
	goto POPEN;

    if (! any2ppm_daemon_exists ()) {
	if (system (any2ppm) != 0)
	    goto POPEN;
    }

    sk = socket (PF_UNIX, SOCK_STREAM, 0);
    if (sk == -1)
	goto POPEN;

    memset (&addr, 0, sizeof (addr));
    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, SOCKET_PATH);

    if (connect (sk, (struct sockaddr *) &addr, sizeof (addr)) == -1) {
	close (sk);
	goto POPEN;
    }

    len = sprintf (command, "%s %d\n", filename, page);
    if (write (sk, command, len) != len) {
	close (sk);
	goto POPEN;
    }

    *close_cb = fclose;
    return fdopen (sk, "rb");

POPEN:
#endif

    *close_cb = pclose;
    sprintf (command, "%s %s %d", any2ppm, filename, page);
    return popen (command, "r");
}

static cairo_bool_t
freadn (unsigned char *buf,
	int	       len,
	FILE	      *file)
{
    int ret;

    while (len) {
	ret = fread (buf, 1, len, file);
	if (ret != len) {
	    if (ferror (file) || feof (file))
		return FALSE;
	}
	len -= ret;
	buf += len;
    }

    return TRUE;
}

cairo_surface_t *
cairo_boilerplate_image_surface_create_from_ppm_stream (FILE *file)
{
    char format;
    int width, height;
    ptrdiff_t stride;
    int x, y;
    unsigned char *data;
    cairo_surface_t *image = NULL;

    if (fscanf (file, "P%c %d %d 255\n", &format, &width, &height) != 3)
	goto FAIL;

    switch (format) {
    case '7': /* XXX */
	image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	break;
    case '6':
	image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
	break;
    case '5':
	image = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
	break;
    default:
	goto FAIL;
    }
    if (cairo_surface_status (image))
	return image;

    data = cairo_image_surface_get_data (image);
    stride = cairo_image_surface_get_stride (image);
    for (y = 0; y < height; y++) {
	unsigned char *buf = data + y*stride;
	switch (format) {
	case '7':
	    if (! freadn (buf, 4 * width, file))
		goto FAIL;
	    break;
	case '6':
	    if (! freadn (buf, 3*width, file))
		goto FAIL;
	    buf += 3*width;
	    for (x = width; x--; ) {
		buf -= 3;
		((uint32_t *) (data + y*stride))[x] =
		    (buf[0] << 16) | (buf[1] << 8) | (buf[2] << 0);
	    }
	    break;
	case '5':
	    if (! freadn (buf, width, file))
		goto FAIL;
	    break;
	}
    }
    cairo_surface_mark_dirty (image);

    return image;

FAIL:
    cairo_surface_destroy (image);
    return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_READ_ERROR);
}

cairo_surface_t *
cairo_boilerplate_convert_to_image (const char *filename,
				    int 	page)
{
    FILE *file;
    unsigned int flags = 0;
    cairo_surface_t *image;
    int (*close_cb) (FILE *);
    int ret;

    if (getenv ("CAIRO_BOILERPLATE_OPEN_NO_DAEMON") != NULL) {
	flags |= CAIRO_BOILERPLATE_OPEN_NO_DAEMON;
    }

  RETRY:
    file = cairo_boilerplate_open_any2ppm (filename, page, flags, &close_cb);
    if (file == NULL) {
	switch (errno) {
	case ENOMEM:
	    return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_NO_MEMORY);
	default:
	    return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_READ_ERROR);
	}
    }

    image = cairo_boilerplate_image_surface_create_from_ppm_stream (file);
    ret = close_cb (file);
    /* check for fatal errors from the interpreter */
    if (ret) { /* any2pmm should never die... */
	cairo_surface_destroy (image);
	if (getenv ("CAIRO_BOILERPLATE_DO_NOT_CRASH_ON_ANY2PPM_ERROR") != NULL) {
	    return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_WRITE_ERROR);
	} else {
	    return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_INVALID_STATUS);
	}
    }

    if (ret == 0 && cairo_surface_status (image) == CAIRO_STATUS_READ_ERROR) {
	if (flags == 0) {
	    /* Try again in a standalone process. */
	    cairo_surface_destroy (image);
	    flags = CAIRO_BOILERPLATE_OPEN_NO_DAEMON;
	    goto RETRY;
	}
    }

    return image;
}

int
cairo_boilerplate_version (void)
{
    return CAIRO_VERSION;
}

const char*
cairo_boilerplate_version_string (void)
{
    return CAIRO_VERSION_STRING;
}

void
cairo_boilerplate_fini (void)
{
    while (cairo_boilerplate_targets != NULL) {
	struct cairo_boilerplate_target_list *next;

	next = cairo_boilerplate_targets->next;

	free (cairo_boilerplate_targets);
	cairo_boilerplate_targets = next;
    }
}
