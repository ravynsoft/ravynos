/*
 * Copyright © 2005 Red Hat, Inc.
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
 * Author: Carl Worth <cworth@redhat.com>
 */

/* Bug history
 *
 * 2005-04-12 Carl Worth <cworth@cworth.org>
 *
 *   I noticed that if we call cairo_select_font_face, but then do a
 *   cairo_destroy before ever drawing any text, then we get:
 *
 *   *** glibc detected *** double free or corruption (fasttop): 0x083274d0 ***
 *   Aborted
 *
 * 2005-04-14 Owen Taylor <otaylor@redhat.com>
 *
 *  Fixed... just a stray free().
 */

#include "cairo-test.h"
#include <math.h>

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (select_font_no_show_text,
	    "Test calling cairo_select_font_face but never drawing text.",
	    "font", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
