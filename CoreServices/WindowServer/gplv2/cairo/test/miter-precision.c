/*
 * Copyright © 2007 Keith Packard
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
 * The Initial Developer of the Original Code is Keith Packard
 *
 * Contributor(s):
 *      Keith Packard <keithp@keithp.com>
 */
#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double  xscale, yscale;
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_miter_limit (cr, 100000);
    for (xscale = 1; xscale <= 1000; xscale += 999)
	for (yscale = 1; yscale <= 1000; yscale += 999)
	{
	    double  max_scale = xscale > yscale ? xscale : yscale;
	    cairo_save (cr);
	    if (xscale > 1)
		cairo_translate (cr, 50, 0);
	    if (yscale > 1)
		cairo_translate (cr, 0, 50);
	    cairo_scale (cr, xscale, yscale);
	    cairo_set_line_width (cr, 10.0 / max_scale);
	    cairo_move_to (cr, 10.0 / xscale, 10.0 / yscale);
	    cairo_line_to (cr, 40.0 / xscale, 10.0 / yscale);
	    cairo_line_to (cr, 10.0 / xscale, 30.0 / yscale);
	    cairo_stroke (cr);
	    cairo_restore (cr);
	}

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (miter_precision,
	    "test how cairo deals with small miters"
	    "\ncurrent code draws inappropriate bevels at times",
	    "stoke, stress", /* keywords */
	    NULL, /* requirements */
	    120, 100,
	    NULL, draw)
