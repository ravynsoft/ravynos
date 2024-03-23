/* cairo-tutorial-gtk.h - a tutorial framework for cairo
 *
 * Copyright © 2005, Carl Worth
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <cairo.h>
#include <math.h>

/* The application program may override these before including
 * cairo-tutorial.h in order to get a window of a different size. */
#ifndef WIDTH
#define WIDTH 400
#endif

#ifndef HEIGHT
#define HEIGHT 400
#endif

#ifdef CAIRO_TUTORIAL_GTK
#include "cairo-tutorial-gtk.h"
#elif CAIRO_TUTORIAL_XLIB
#include "cairo-tutorial-xlib.h"
#elif CAIRO_TUTORIAL_PDF
#include "cairo-tutorial-pdf.h"
#elif CAIRO_TUTORIAL_PNG
#include "cairo-tutorial-png.h"
#endif
