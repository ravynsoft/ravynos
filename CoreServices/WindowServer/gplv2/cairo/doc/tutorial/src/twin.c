/**
 * Put this file in cairo/doc/tutorial/src and type "make"
 */

#define WIDTH 1350
#define HEIGHT 900

#include "cairo-tutorial.h"


static void
draw (cairo_t *cr, int width, int height)
{
    int i, j, h;
    unsigned char s[2] = {0, 0};

    /* clear background */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_select_font_face (cr,
			    "@cairo:",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    h = 2;
    for (i = 8; i < 48; i >= 24 ? i+=3 : i++) {
	cairo_set_font_size (cr, i);
	for (j = 33; j < 128; j++) {
	    if (j == 33 || (j == 80 && i > 24)) {
		h += i + 2;
		cairo_move_to (cr, 10, h);
	    }
	    s[0] = j;
	    cairo_show_text (cr, (const char *) s);
	}
    }
}
