/*
 * Copyright © 2022 John Ralls <jralls@ceridwen.us>
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
 */

#include "cairo-test.h"
#include <cairo-quartz.h>

#define WIDTH  100
#define HEIGHT 50

#define FONT "Apple Color Emoji"

static const char smiley_face_utf8[] = { 0xf0, 0x9f, 0x99, 0x82, 0x00 }; /* U+1F642 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_select_font_face(cr, FONT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, HEIGHT/2);
    cairo_move_to (cr, width/5, 2*height/3);

    cairo_show_text(cr, smiley_face_utf8);
    cairo_show_text(cr, smiley_face_utf8);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (quartz_color_font,
	    "Test color font",
	    "quartz, font", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
