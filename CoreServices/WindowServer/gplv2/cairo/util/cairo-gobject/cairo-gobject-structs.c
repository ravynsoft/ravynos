/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2010 Red Hat Inc.
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
 *	Benjamin Otte <otte@redhat.com>
 */

#include "config.h"

#include "cairo-gobject.h"

#define CAIRO_DEFINE_BOXED(Name,underscore_name,copy_func,free_func) \
GType \
underscore_name ## _get_type (void) \
{ \
   static gsize type_ret = 0; \
   if (g_once_init_enter (&type_ret)) { \
      GType type = g_boxed_type_register_static (g_intern_static_string (Name), \
                                                 (GBoxedCopyFunc)copy_func, \
                                                 (GBoxedFreeFunc)free_func); \
      g_once_init_leave (&type_ret, type); \
   } \
   return type_ret; \
}

CAIRO_DEFINE_BOXED ("CairoContext", cairo_gobject_context, 
                    cairo_reference, cairo_destroy);
CAIRO_DEFINE_BOXED ("CairoDevice", cairo_gobject_device, 
                    cairo_device_reference, cairo_device_destroy);
CAIRO_DEFINE_BOXED ("CairoPattern", cairo_gobject_pattern, 
                    cairo_pattern_reference, cairo_pattern_destroy);
CAIRO_DEFINE_BOXED ("CairoSurface", cairo_gobject_surface, 
                    cairo_surface_reference, cairo_surface_destroy);
CAIRO_DEFINE_BOXED ("CairoScaledFont", cairo_gobject_scaled_font, 
                    cairo_scaled_font_reference, cairo_scaled_font_destroy);
CAIRO_DEFINE_BOXED ("CairoFontFace", cairo_gobject_font_face, 
                    cairo_font_face_reference, cairo_font_face_destroy);
CAIRO_DEFINE_BOXED ("CairoFontOptions", cairo_gobject_font_options, 
                    cairo_font_options_copy, cairo_font_options_destroy);
CAIRO_DEFINE_BOXED ("CairoRegion", cairo_gobject_region, 
                    cairo_region_reference, cairo_region_destroy);

#if GLIB_CHECK_VERSION(2, 68, 0)
#define COPY_FUNC(name) \
static gpointer \
cairo_gobject_cairo_ ## name ## _copy (gpointer src) { \
  return g_memdup2 (src, sizeof (cairo_ ## name ## _t)); \
}
#else
#define COPY_FUNC(name) \
static gpointer \
cairo_gobject_cairo_ ## name ## _copy (gpointer src) { \
  return g_memdup (src, sizeof (cairo_ ## name ## _t)); \
}
#endif

COPY_FUNC (matrix)
CAIRO_DEFINE_BOXED ("CairoMatrix", cairo_gobject_matrix, 
                    cairo_gobject_cairo_matrix_copy, g_free);
COPY_FUNC (rectangle)
CAIRO_DEFINE_BOXED ("CairoRectangle", cairo_gobject_rectangle, 
                    cairo_gobject_cairo_rectangle_copy, g_free);
COPY_FUNC (rectangle_int)
CAIRO_DEFINE_BOXED ("CairoRectangleInt", cairo_gobject_rectangle_int, 
                    cairo_gobject_cairo_rectangle_int_copy, g_free);
COPY_FUNC (glyph)
CAIRO_DEFINE_BOXED ("CairoGlyph", cairo_gobject_glyph,
                    cairo_gobject_cairo_glyph_copy, g_free);
COPY_FUNC (text_cluster)
CAIRO_DEFINE_BOXED ("CairoTextCluster", cairo_gobject_text_cluster,
                    cairo_gobject_cairo_text_cluster_copy, g_free);
