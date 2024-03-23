#include "cairo-test.h"

static const char *png_filename = "romedalen.png";

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *surface;

    surface = cairo_test_create_surface_from_png (ctx, png_filename);
    cairo_set_source_surface (cr, surface, 32, 32);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);

    cairo_paint (cr);

    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (extend_repeat,
	    "Test CAIRO_EXTEND_REPEAT for surface patterns",
	    "extend", /* keywords */
	    NULL, /* requirements */
	    256 + 32*2, 192 + 32*2,
	    NULL, draw)
