/*
 * Copyright © 2011 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-perf.h"

#define ITER 1000
#define HOLDOVERS 256
#define LIVE_ENTRIES 257
#define ACTIVE_FONTS (LIVE_ENTRIES - HOLDOVERS - 1)

/*
 * The original implementation of hash tables was very inefficient, as
 * pointed out in https://bugs.freedesktop.org/show_bug.cgi?id=17399
 *
 * This benchmark tries to fill up the scaled_font_map hash table to
 * show the O(n) behavior.
 */

static cairo_time_t
do_hash_table (cairo_t *cr, int width, int height, int loops)
{
    /*
     * Microsoft C Compiler complains that:
     * error C2466: cannot allocate an array of constant size 0
     * so we add an unused element to make it happy
     */
    cairo_scaled_font_t *active_fonts[ACTIVE_FONTS + 1];
    cairo_matrix_t m;
    int i;

    cairo_matrix_init_identity (&m);

    /* Touch HOLDOVERS scaled fonts to fill up the holdover list. */
    for (i = 0; i < HOLDOVERS; i++) {
	m.yy = m.xx * (i + 1);
	cairo_set_font_matrix (cr, &m);
	cairo_get_scaled_font (cr);
    }

    /*
     * Reference some scaled fonts so that they will be kept in the
     * scaled fonts map. We want LIVE_ENTRIES elements in the font
     * map, but cairo keeps HOLDOVERS recently used fonts in it and we
     * will be activating a new font in the cr context, so we just
     * keep references to ACTIVE_FONTS fonts.
     *
     * Note: setting LIVE_ENTRIES == HOLDOVERS+1 means that we keep no
     * font in active_fonts and the slowness is caused by the holdover
     * fonts only.
     */
    for (i = 0; i < ACTIVE_FONTS; i++) {
	cairo_scaled_font_t *scaled_font;

	m.yy = m.xx * (i + 1);
	cairo_set_font_matrix (cr, &m);

	scaled_font = cairo_get_scaled_font (cr);
	active_fonts[i] = cairo_scaled_font_reference (scaled_font);
    }

    cairo_perf_timer_start ();

    while (loops--) {
	m.xx += 1.0;

	/* Generate ITER new scaled fonts per loop */
	for (i = 0; i < ITER; i++) {
	    m.yy = m.xx * (i + 1);
	    cairo_set_font_matrix (cr, &m);
	    cairo_get_scaled_font (cr);
	}
    }

    cairo_perf_timer_stop ();

    for (i = 0; i < ACTIVE_FONTS; i++)
	cairo_scaled_font_destroy (active_fonts[i]);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
hash_table_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "hash-table", NULL);
}

void
hash_table (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_cover_sources_and_operators (perf, "hash-table",
					    do_hash_table, NULL);
}
