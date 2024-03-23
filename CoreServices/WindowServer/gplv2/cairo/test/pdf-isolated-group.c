/*
 * Copyright © 2012 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

#define SIZE 60
#define WIDTH  SIZE
#define HEIGHT SIZE


/* PDF transparency groups can be isolated or non-isolated. This test
 * checks that the PDF output is using isolated groups. If the group
 * is non-isolated the bottom half of the inner rectangle will be
 * red. Note poppler-cairo currently ignores the isolated flag and
 * treats the group as isolated.
 *
 * Refer to http://www.pdfvt.com/PDFVT_TransparencyGuide.html for an
 * explanation isolated vs non-isolated.
 */

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1, 0.5, 0);
    cairo_rectangle (cr, 0, SIZE/2, SIZE, SIZE/2);
    cairo_fill (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_MULTIPLY);

    cairo_push_group (cr);

    cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
    cairo_rectangle (cr, SIZE/4, SIZE/4, SIZE/2, SIZE/2);
    cairo_fill (cr);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (pdf_isolated_group,
	    "Check that transparency groups in PDF output are isolated",
	    "group, operator", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
