#define WIDTH 750
#define HEIGHT 360

#include "cairo-tutorial.h"

static void
draw (cairo_t *cr, int width, int height)
{
    cairo_save (cr);

    cairo_translate (cr, 60, 60);
    cairo_scale (cr, 3, 3);

    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width (cr, 20);

    /* L */
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 0, 80);
    cairo_line_to (cr, 50, 80);

    /* C */
    cairo_move_to (cr, 110 + 40 * cos (M_PI / 3), 40 + 40 * sin(M_PI / 3));
    cairo_arc (cr, 110, 40, 40, M_PI / 3, -M_PI / 3);

    /* A */
    cairo_move_to (cr, 160, 80);
    cairo_curve_to (cr, 160, -30, 210, -30, 210, 80);

    cairo_stroke (cr);
}
