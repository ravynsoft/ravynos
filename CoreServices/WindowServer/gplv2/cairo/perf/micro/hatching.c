/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright (c) 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "cairo-perf.h"
#include <assert.h>


#define STEP	5
#define WIDTH	100
#define HEIGHT	100

static void path (cairo_t *cr, unsigned int width, unsigned int height)
{
    unsigned int i;

    for (i = 0; i < width+1; i += STEP) {
	cairo_rectangle (cr, i-1, -1, 2, height+2);
	cairo_rectangle (cr, -1, i-1, width+2, 2);
    }
}

static void aa (cairo_t *cr)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
}

static void mono (cairo_t *cr)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
}

static void aligned (cairo_t *cr, int width, int height)
{
}

static void misaligned (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, 0.25, 0.25);
}

static void rotated (cairo_t *cr, int width, int height)
{
    cairo_translate (cr, width/2, height/2);
    cairo_rotate (cr, M_PI/4);
    cairo_translate (cr, -width/2, -height/2);
}

static void clip (cairo_t *cr)
{
    cairo_clip (cr);
    cairo_paint (cr);
}

static void clip_alpha (cairo_t *cr)
{
    cairo_clip (cr);
    cairo_paint_with_alpha (cr, .5);
}

static cairo_time_t
draw (cairo_t *cr,
      void (*prepare) (cairo_t *cr),
      void (*transform) (cairo_t *cr, int width, int height),
      void (*op) (cairo_t *cr),
      int width, int height, int loops)
{
    cairo_save (cr);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);

    prepare (cr);

    cairo_perf_timer_start ();
    while (loops--) {
	cairo_save (cr);
	transform (cr, width, height);
	path (cr, width, height);
	op (cr);
	cairo_restore (cr);
    }
    cairo_perf_timer_stop ();

    cairo_restore (cr);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
draw_aligned_aa (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, aa, aligned, cairo_fill,
		width, height, loops);
}

static cairo_time_t
draw_misaligned_aa (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, aa, misaligned, cairo_fill,
		width, height, loops);
}

static cairo_time_t
draw_rotated_aa (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, aa, rotated, cairo_fill,
		width, height, loops);
}

static cairo_time_t
draw_aligned_mono (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, mono, aligned, cairo_fill,
		width, height, loops);
}

static cairo_time_t
draw_misaligned_mono (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, mono, misaligned, cairo_fill,
		width, height, loops);
}

static cairo_time_t
draw_rotated_mono (cairo_t *cr, int width, int height, int loops)
{
    return draw(cr, mono, rotated, cairo_fill,
		width, height, loops);
}

#define F(name, op,transform,aa) \
static cairo_time_t \
draw_##name (cairo_t *cr, int width, int height, int loops) \
{ return draw(cr, (aa), (transform), (op), width, height, loops); }

F(clip_aligned, clip, aligned, aa)
F(clip_misaligned, clip, misaligned, aa)
F(clip_rotated, clip, rotated, aa)
F(clip_aligned_mono, clip, aligned, mono)
F(clip_misaligned_mono, clip, misaligned, mono)
F(clip_rotated_mono, clip, rotated, mono)

F(clip_alpha_aligned, clip_alpha, aligned, aa)
F(clip_alpha_misaligned, clip_alpha, misaligned, aa)
F(clip_alpha_rotated, clip_alpha, rotated, aa)
F(clip_alpha_aligned_mono, clip_alpha, aligned, mono)
F(clip_alpha_misaligned_mono, clip_alpha, misaligned, mono)
F(clip_alpha_rotated_mono, clip_alpha, rotated, mono)

cairo_bool_t
hatching_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "hatching", NULL);
}

void
hatching (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "hatching-aligned-aa", draw_aligned_aa, NULL);
    cairo_perf_run (perf, "hatching-misaligned-aa", draw_misaligned_aa, NULL);
    cairo_perf_run (perf, "hatching-rotated-aa", draw_rotated_aa, NULL);
    cairo_perf_run (perf, "hatching-aligned-mono", draw_aligned_mono, NULL);
    cairo_perf_run (perf, "hatching-misaligned-mono", draw_misaligned_mono, NULL);
    cairo_perf_run (perf, "hatching-rotated-mono", draw_rotated_mono, NULL);

    cairo_perf_run (perf, "hatching-clip-aligned-aa", draw_clip_aligned, NULL);
    cairo_perf_run (perf, "hatching-clip-misaligned-aa", draw_clip_misaligned, NULL);
    cairo_perf_run (perf, "hatching-clip-rotated-aa", draw_clip_rotated, NULL);
    cairo_perf_run (perf, "hatching-clip-aligned-mono", draw_clip_aligned_mono, NULL);
    cairo_perf_run (perf, "hatching-clip-misaligned-mono", draw_clip_misaligned_mono, NULL);
    cairo_perf_run (perf, "hatching-clip-rotated-mono", draw_clip_rotated_mono, NULL);

    cairo_perf_run (perf, "hatching-clip-alpha-aligned-aa", draw_clip_alpha_aligned, NULL);
    cairo_perf_run (perf, "hatching-clip-alpha-misaligned-aa", draw_clip_alpha_misaligned, NULL);
    cairo_perf_run (perf, "hatching-clip-alpha-rotated-aa", draw_clip_alpha_rotated, NULL);
    cairo_perf_run (perf, "hatching-clip-alpha-aligned-mono", draw_clip_alpha_aligned_mono, NULL);
    cairo_perf_run (perf, "hatching-clip-alpha-misaligned-mono", draw_clip_alpha_misaligned_mono, NULL);
    cairo_perf_run (perf, "hatching-clip-alpha-rotated-mono", draw_clip_alpha_rotated_mono, NULL);
}
