#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    double dsh[2] = {1,3};

    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    cairo_paint (cr);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

    cairo_move_to (cr, 3, 3);
    /* struct glitter_scan_converter spans_embedded array size is 64 */
    cairo_line_to (cr, 65+3, 3);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_FAST);
    cairo_set_tolerance (cr, 1);

    cairo_set_dash (cr, dsh, 2, 0);
    cairo_set_line_width (cr, 2);

    cairo_stroke (cr);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_75705,
	    "Bug 75705 (exercise tor22-scan-converter)",
	    "dash, stroke, antialias", /* keywords */
	    NULL, /* requirements */
	    72, 8,
	    NULL, draw)
