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

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					  WIDTH, HEIGHT);

    cr = cairo_create (surface);

    draw (cr, WIDTH, HEIGHT);

    filename = strdup (argv[0]);
    assert (filename != NULL);

    dash = strrchr (filename, '-');

    if (strcmp (dash, "-png") == 0) {
	*dash = '.';
    } else {
	char *new_filename;
	new_filename = malloc (strlen (filename) + 5);
	sprintf (new_filename, "%s.png", filename);
	free (filename);
	filename = new_filename;
    }

    cairo_surface_write_to_png (surface, filename);

    free (filename);

    cairo_surface_destroy (surface);
    cairo_destroy (cr);

    return 0;
}
