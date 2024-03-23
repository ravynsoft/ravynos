/*
 * Copyright © 2006 Red Hat, Inc.
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

#include "cairo-perf.h"

static void
init_and_set_source_surface (cairo_t		*cr,
			     cairo_surface_t	*source,
			     int		 width,
			     int		 height)
{
    cairo_t *cr2;

    /* Fill it with something known */
    cr2 = cairo_create (source);
    cairo_set_operator (cr2, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr2, 0, 0, 1); /* blue */
    cairo_paint (cr2);

    cairo_set_source_rgba (cr2, 1, 0, 0, 0.5); /* 50% red */
    cairo_new_path (cr2);
    cairo_rectangle (cr2, 0, 0, width/2.0, height/2.0);
    cairo_rectangle (cr2, width/2.0, height/2.0, width/2.0, height/2.0);
    cairo_fill (cr2);

    cairo_set_source_surface (cr, cairo_get_target (cr2), 0, 0);
    cairo_destroy (cr2);
}

static void
set_source_solid_rgb (cairo_t	*cr,
		      int	 width,
		      int	 height)
{
    cairo_set_source_rgb (cr, 0.2, 0.6, 0.9);
}

static void
set_source_solid_rgba (cairo_t	*cr,
		       int	 width,
		       int	 height)
{
    cairo_set_source_rgba (cr, 0.2, 0.6, 0.9, 0.7);
}

static void
set_source_image_surface_rgb (cairo_t	*cr,
			      int	 width,
			      int	 height)
{
    cairo_surface_t *source;

    source = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					 width, height);
    init_and_set_source_surface (cr, source, width, height);

    cairo_surface_destroy (source);
}

static void
set_source_image_surface_rgba (cairo_t	*cr,
			       int	 width,
			       int	 height)
{
    cairo_surface_t *source;

    source = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					 width, height);
    init_and_set_source_surface (cr, source, width, height);

    cairo_surface_destroy (source);
}

static void
set_source_image_surface_rgba_mag (cairo_t	*cr,
				   int		width,
				   int		height)
{
    cairo_surface_t *source;

    source = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					 width/2, height/2);
    cairo_scale(cr, 2.1, 2.1);
    init_and_set_source_surface (cr, source, width/2, height/2);
    cairo_scale(cr, 1/2.1, 1/2.1);

    cairo_surface_destroy (source);
}

static void
set_source_image_surface_rgba_min (cairo_t	*cr,
				   int		width,
				   int		height)
{
    cairo_surface_t *source;

    source = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					 width*2, height*2);
    cairo_scale(cr, 1/1.9, 1/1.9);
    init_and_set_source_surface (cr, source, width*2, height*2);
    cairo_scale(cr, 1.9, 1.9);

    cairo_surface_destroy (source);
}

static void
set_source_similar_surface_rgb (cairo_t	*cr,
				int	 width,
				int	 height)
{
    cairo_surface_t *source;

    source = cairo_surface_create_similar (cairo_get_group_target (cr),
					   CAIRO_CONTENT_COLOR,
					   width, height);
    init_and_set_source_surface (cr, source, width, height);

    cairo_surface_destroy (source);
}

static void
set_source_similar_surface_rgba (cairo_t	*cr,
				 int		 width,
				 int		 height)
{
    cairo_surface_t *source;

    source = cairo_surface_create_similar (cairo_get_group_target (cr),
					   CAIRO_CONTENT_COLOR_ALPHA,
					   width, height);
    init_and_set_source_surface (cr, source, width, height);

    cairo_surface_destroy (source);
}

static void
set_source_similar_surface_rgba_mag (cairo_t	*cr,
				     int	width,
				     int	height)
{
    cairo_surface_t *source;

    source = cairo_surface_create_similar (cairo_get_group_target (cr),
					   CAIRO_CONTENT_COLOR_ALPHA,
					   width/2, height/2);
    cairo_scale(cr, 2.1, 2.1);
    init_and_set_source_surface (cr, source, width/2, height/2);
    cairo_scale(cr, 1/2.1, 1/2.1);

    cairo_surface_destroy (source);
}

static void
set_source_similar_surface_rgba_min (cairo_t	*cr,
				     int	width,
				     int	height)
{
    cairo_surface_t *source;

    source = cairo_surface_create_similar (cairo_get_group_target (cr),
					   CAIRO_CONTENT_COLOR_ALPHA,
					   width*2, height*2);
    cairo_scale(cr, 1/1.9, 1/1.9);
    init_and_set_source_surface (cr, source, width*2, height*2);
    cairo_scale(cr, 1.9, 1.9);

    cairo_surface_destroy (source);
}

static void
set_source_linear_rgb (cairo_t *cr,
		       int	width,
		       int	height)
{
    cairo_pattern_t *linear;

    linear = cairo_pattern_create_linear (0.0, 0.0, width, height);
    cairo_pattern_add_color_stop_rgb (linear, 0.0, 1, 0, 0); /* red */
    cairo_pattern_add_color_stop_rgb (linear, 1.0, 0, 0, 1); /* blue */

    cairo_set_source (cr, linear);

    cairo_pattern_destroy (linear);
}

static void
set_source_linear_rgba (cairo_t *cr,
			int	width,
			int	height)
{
    cairo_pattern_t *linear;

    linear = cairo_pattern_create_linear (0.0, 0.0, width, height);
    cairo_pattern_add_color_stop_rgba (linear, 0.0, 1, 0, 0, 0.5); /* 50% red */
    cairo_pattern_add_color_stop_rgba (linear, 1.0, 0, 0, 1, 0.0); /*  0% blue */

    cairo_set_source (cr, linear);

    cairo_pattern_destroy (linear);
}

static void
set_source_linear3_rgb (cairo_t *cr,
		       int	width,
		       int	height)
{
    cairo_pattern_t *linear;

    linear = cairo_pattern_create_linear (0.0, 0.0, width, height);
    cairo_pattern_add_color_stop_rgb (linear, 0.0, 1, 0, 0); /* red */
    cairo_pattern_add_color_stop_rgb (linear, 0.5, 0, 1, 0); /* green */
    cairo_pattern_add_color_stop_rgb (linear, 1.0, 0, 0, 1); /* blue */

    cairo_set_source (cr, linear);

    cairo_pattern_destroy (linear);
}

static void
set_source_linear3_rgba (cairo_t *cr,
			int	width,
			int	height)
{
    cairo_pattern_t *linear;

    linear = cairo_pattern_create_linear (0.0, 0.0, width, height);
    cairo_pattern_add_color_stop_rgba (linear, 0.0, 1, 0, 0, 0.5); /* 50% red */
    cairo_pattern_add_color_stop_rgba (linear, 0.5, 0, 1, 0, 0.0); /*  0% green */
    cairo_pattern_add_color_stop_rgba (linear, 1.0, 0, 0, 1, 0.5); /*  50% blue */

    cairo_set_source (cr, linear);

    cairo_pattern_destroy (linear);
}

static void
set_source_radial_rgb (cairo_t *cr,
		       int	width,
		       int	height)
{
    cairo_pattern_t *radial;

    radial = cairo_pattern_create_radial (width/2.0, height/2.0, 0.0,
					  width/2.0, height/2.0, width/2.0);
    cairo_pattern_add_color_stop_rgb (radial, 0.0, 1, 0, 0); /* red */
    cairo_pattern_add_color_stop_rgb (radial, 1.0, 0, 0, 1); /* blue */

    cairo_set_source (cr, radial);

    cairo_pattern_destroy (radial);
}

static void
set_source_radial_rgba (cairo_t *cr,
			int	width,
			int	height)
{
    cairo_pattern_t *radial;

    radial = cairo_pattern_create_radial (width/2.0, height/2.0, 0.0,
					  width/2.0, height/2.0, width/2.0);
    cairo_pattern_add_color_stop_rgba (radial, 0.0, 1, 0, 0, 0.5); /* 50% red */
    cairo_pattern_add_color_stop_rgba (radial, 1.0, 0, 0, 1, 0.0); /*  0% blue */

    cairo_set_source (cr, radial);

    cairo_pattern_destroy (radial);
}

typedef void (*set_source_func_t) (cairo_t *cr, int width, int height);

void
cairo_perf_cover_sources_and_operators (cairo_perf_t		*perf,
					const char		*name,
					cairo_perf_func_t	 perf_func,
					cairo_count_func_t	 count_func)
{
    unsigned int i, j;
    char *expanded_name;

    struct { set_source_func_t set_source; const char *name; } sources[] = {
	{ set_source_solid_rgb, "solid-rgb" },
	{ set_source_solid_rgba, "solid-rgba" },
	{ set_source_image_surface_rgb, "image-rgb" },
	{ set_source_image_surface_rgba, "image-rgba" },
	{ set_source_image_surface_rgba_mag, "image-rgba-mag" },
	{ set_source_image_surface_rgba_min, "image-rgba-min" },
	{ set_source_similar_surface_rgb, "similar-rgb" },
	{ set_source_similar_surface_rgba, "similar-rgba" },
	{ set_source_similar_surface_rgba_mag, "similar-rgba-mag" },
	{ set_source_similar_surface_rgba_min, "similar-rgba-min" },
	{ set_source_linear_rgb, "linear-rgb" },
	{ set_source_linear_rgba, "linear-rgba" },
	{ set_source_linear3_rgb, "linear3-rgb" },
	{ set_source_linear3_rgba, "linear3-rgba" },
	{ set_source_radial_rgb, "radial-rgb" },
	{ set_source_radial_rgba, "radial-rgba" }
    };

    struct { cairo_operator_t op; const char *name; } operators[] = {
	{ CAIRO_OPERATOR_OVER, "over" },
	{ CAIRO_OPERATOR_SOURCE, "source" }
    };

    for (i = 0; i < ARRAY_LENGTH (sources); i++) {
	(sources[i].set_source) (perf->cr, perf->size, perf->size);

	for (j = 0; j < ARRAY_LENGTH (operators); j++) {
	    cairo_set_operator (perf->cr, operators[j].op);

	    xasprintf (&expanded_name, "%s_%s_%s",
		       name, sources[i].name, operators[j].name);
	    cairo_perf_run (perf, expanded_name, perf_func, count_func);
	    free (expanded_name);
	}
    }
}
