/*
  Copyright (C) 2006 Yangli Hector Yee
  Copyright (C) 2006 Red Hat, Inc.

  This program is free software; you can redistribute it and/or modify it under the terms of the
  GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program;
  if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
*/

#ifndef _PDIFF_H
#define _PDIFF_H

#include <cairo.h>

typedef int bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Image comparison metric using Yee's method (and a cairo interface)
 * References: A Perceptual Metric for Production Testing, Hector Yee, Journal of Graphics Tools 2004
 */
int
pdiff_compare (cairo_surface_t *surface_a,
	       cairo_surface_t *surface_b,
	       double gamma,
	       double luminance,
	       double field_of_view);

#endif
