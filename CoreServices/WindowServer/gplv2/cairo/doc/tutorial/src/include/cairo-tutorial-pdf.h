/* cairo-tutorial-png.h - a tutorial framework for cairo to write a PNG image
 *
 * Copyright © 2005, Carl Worth
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <cairo.h>
#include <cairo-pdf.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef WIDTH
#define WIDTH 400
#endif

#ifndef HEIGHT
#define HEIGHT 400
#endif

static void
draw (cairo_t *cr, int width, int height);

int
main (int argc, char **argv)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    char *filename, *dash;

    filename = strdup (argv[0]);
    assert (filename != NULL);

    dash = strrchr (filename, '-');

    if (strcmp (dash, "-pdf") == 0) {
	*dash = '.';
    } else {
	char *new_filename;
	new_filename = malloc (strlen (filename) + 5);
	sprintf (new_filename, "%s.pdf", filename);
	free (filename);
	filename = new_filename;
    }

    surface = cairo_pdf_surface_create (filename, WIDTH, HEIGHT);

    cr = cairo_create (surface);

    draw (cr, WIDTH, HEIGHT);

    cairo_show_page (cr);

    cairo_surface_destroy (surface);
    cairo_destroy (cr);

    free (filename);

    return 0;
}
