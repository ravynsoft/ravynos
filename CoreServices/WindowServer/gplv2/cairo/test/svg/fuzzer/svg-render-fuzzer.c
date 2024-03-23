/*
 * Copyright © 2022 Uli Schlachter
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Uli Schlachter <psychon@znc.in>
 */

#include <cairo.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

cairo_bool_t
_cairo_debug_svg_render (cairo_t       *cr,
                         const char    *svg_document,
                         const char    *element,
                         double         units_per_em,
                         int            debug_level);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    cairo_surface_t *s = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t *cr = cairo_create(s);

    /* Get us a zero terminated string */
    const char *svg_document = strndup ((const char *) data, size);

    _cairo_debug_svg_render (cr,
                             svg_document,
                             NULL,
                             1000,
                             0);
    free (svg_document);
    cairo_destroy (cr);
    cairo_surface_destroy (s);
    return 0;
}
