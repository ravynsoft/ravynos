#include "cairo-tutorial.h"

static void
draw (cairo_t *cr, int width, int height)
{
    int radius;

    if (width < height)
	radius = width/2 - 4;
    else
	radius = height/2 - 4;

    cairo_move_to (cr, width/2 + radius, height/2);
    cairo_arc (cr, width/2, height/2, radius,
	       0.0, 2 * M_PI);

    cairo_set_source_rgb (cr, 0.6, 0.8, 1.0);
    cairo_fill_preserve (cr);

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_stroke (cr);
}
