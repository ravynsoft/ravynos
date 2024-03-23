#define WIDTH 1350
#define HEIGHT 900

#include "cairo-perf.h"

static cairo_time_t
do_twin (cairo_t *cr,
	 int width,
	 int height,
	 int loops)
{
    int i, j, h;
    unsigned char s[2] = {0, 0};

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_select_font_face (cr,
			    "@cairo:",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_perf_timer_start ();

    while (loops--) {
	h = 2;
	for (i = 8; i < 48; i >= 24 ? i+=3 : i++) {
	    cairo_set_font_size (cr, i);
	    for (j = 33; j < 128; j++) {
		if (j == 33 || (j == 80 && i > 24)) {
		    h += i + 2;
		    cairo_move_to (cr, 10, h);
		}
		s[0] = j;
		cairo_text_path (cr, (const char *) s);
	    }
	}
	cairo_fill (cr);
    }

    cairo_perf_timer_stop ();
    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
twin_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "twin", NULL);
}

void
twin (cairo_perf_t *perf,
      cairo_t      *cr,
      int           width,
      int           height)
{
    cairo_perf_run (perf, "twin", do_twin, NULL);
}
