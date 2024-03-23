/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright 2002 University of Southern California
 * Copyright 2005 Red Hat, Inc.
 * Copyright 2007 Emmanuel Pacaud
 * Copyright 2008 Benjamin Otte
 * Copyright 2008 Chris Wilson
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *      Owen Taylor <otaylor@redhat.com>
 *      Kristian Høgsberg <krh@redhat.com>
 *      Emmanuel Pacaud <emmanuel.pacaud@lapp.in2p3.fr>
 *      Chris Wilson <chris@chris-wilson.co.uk>
 *      Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define STEPS 16
#define START_OPERATOR	CAIRO_OPERATOR_CLEAR
#define STOP_OPERATOR	CAIRO_OPERATOR_HSL_LUMINOSITY

#define SIZE 3
#define COUNT 6
#define FULL_WIDTH  ((STEPS + 1) * COUNT - 1)
#define FULL_HEIGHT ((COUNT + STOP_OPERATOR - START_OPERATOR) / COUNT) * (STEPS + 1)

static void
create_patterns (cairo_t *bg, cairo_t *fg)
{
    int x;

    for (x = 0; x < STEPS; x++) {
	double i = (double) x / (STEPS - 1);
	cairo_set_source_rgba (bg, 0, 0, 0, i);
	cairo_rectangle (bg, x, 0, 1, STEPS);
	cairo_fill (bg);

	cairo_set_source_rgba (fg, 0, 0, 0, i);
	cairo_rectangle (fg, 0, x, STEPS, 1);
	cairo_fill (fg);
    }
}

/* expects a STEP*STEP pixel rectangle */
static void
do_composite (cairo_t *cr, cairo_operator_t op, cairo_surface_t *bg, cairo_surface_t *fg)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, bg, 0, 0);
    cairo_paint (cr);

    cairo_set_operator (cr, op);
    cairo_set_source_surface (cr, fg, 0, 0);
    cairo_paint (cr);
}

static void
subdraw (cairo_t *cr, int width, int height)
{
    size_t i = 0;
    cairo_operator_t op;
    cairo_t *bgcr, *fgcr;
    cairo_surface_t *bg, *fg;

    bg = cairo_surface_create_similar (cairo_get_target (cr),
	    CAIRO_CONTENT_ALPHA, SIZE * STEPS, SIZE * STEPS);
    fg = cairo_surface_create_similar (cairo_get_target (cr),
	    CAIRO_CONTENT_ALPHA, SIZE * STEPS, SIZE * STEPS);
    bgcr = cairo_create (bg);
    fgcr = cairo_create (fg);
    cairo_scale (bgcr, SIZE, SIZE);
    cairo_scale (fgcr, SIZE, SIZE);
    create_patterns (bgcr, fgcr);
    cairo_destroy (bgcr);
    cairo_destroy (fgcr);

    for (op = START_OPERATOR; op <= STOP_OPERATOR; op++, i++) {
	cairo_save (cr);
	cairo_translate (cr,
		SIZE * (STEPS + 1) * (i % COUNT),
		SIZE * (STEPS + 1) * (i / COUNT));
	cairo_rectangle (cr, 0, 0, SIZE * (STEPS + 1), SIZE * (STEPS+1));
	cairo_clip (cr);
	do_composite (cr, op, bg, fg);
	cairo_restore (cr);
    }

    cairo_surface_destroy (fg);
    cairo_surface_destroy (bg);
}


static cairo_surface_t *
create_source (cairo_surface_t *target, int width, int height)
{
    cairo_surface_t *similar;
    cairo_t *cr;

    similar = cairo_surface_create_similar (target,
					    CAIRO_CONTENT_ALPHA,
					    width, height);
    cr = cairo_create (similar);
    cairo_surface_destroy (similar);

    subdraw (cr, width, height);

    similar = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return similar;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *source;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    source = create_source (cairo_get_target (cr), width, height);
    cairo_set_source_surface (cr, source, 0, 0);
    cairo_surface_destroy (source);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (operator_alpha_alpha,
	    "Tests result of compositing pure-alpha surfaces"
	    "\nCompositing of pure-alpha sources is inconsistent across backends.",
	    "alpha, similar, operator", /* keywords */
	    NULL, /* requirements */
	    FULL_WIDTH * SIZE, FULL_HEIGHT * SIZE,
	    NULL, draw)
