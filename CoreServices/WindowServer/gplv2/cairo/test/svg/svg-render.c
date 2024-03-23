/*
 * Copyright © 2016 Adrian Johnson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

/* Compilation:
 *   Build cairo with -DDEBUG_SVG_RENDER
 *   gcc -o svg-render svg-render.c `pkg-config --cflags --libs cairo librsvg`
 */

/* svg-render - render SVG files using both the cairo glyph renderer and librsvg.
 *
 * This allows testing the cairo SVG test cases before assembling them into an SVG font.
 * Usage:
 *   svg-render [-b] [-s] [-g <id>] [-e <em_size> ] input.svg
 *
 * Output is written to input.cairo.png and input.rsvg.png.
 *
 *  -b   print bounding box.
 *  -s   Use standard SVG viewport. See below.
 *  -g   render glyph with id <id>
 *  -e   set the EM size. The default is 1000.
 *
 * SVG Glyphs are assumed to be wholely within the view port. 
 */

#include <stdio.h>
#include <math.h>
#include <cairo.h>
#include <librsvg/rsvg.h>

typedef enum { CAIRO_SVG, LIBRSVG } svg_renderer_t;

/* output image size */
#define WIDTH 1000
#define HEIGHT 1000

static cairo_bool_t bbox = FALSE;
static cairo_bool_t standard_svg = FALSE;
static const char *glyph_id = NULL;
static int em_size = 1000;

cairo_bool_t
_cairo_debug_svg_render (cairo_t       *cr,
                         const char    *svg_document,
                         const char    *element,
                         double         units_per_em,
                         int            debug_level);

static void
cairo_render (const char *svg_document, cairo_t *cr)
{
    if (!_cairo_debug_svg_render (cr, svg_document, glyph_id, em_size, 2))
        printf("_cairo_debug_svg_render() failed\n");
}

static void
librsvg_render (const char *svg_document, cairo_t *cr)
{
    gboolean has_width;
    gboolean has_height;
    gboolean has_viewbox;
    RsvgLength svg_width;
    RsvgLength svg_height;
    RsvgRectangle svg_viewbox;
    RsvgRectangle viewport;
    double width, height;
    GError *error = NULL;

    RsvgHandle *handle = rsvg_handle_new_from_data ((guint8 *)svg_document,
                                                    strlen(svg_document),
                                                    &error);
    if (!handle) {
        printf ("Could not load: %s", error->message);
        return;
    }

    /* Default width if not specified is EM Square */
    width  = em_size;
    height = em_size;
    
    /* Get width/height if specified. */
    rsvg_handle_get_intrinsic_dimensions(handle,
                                         &has_width,
                                         &svg_width,
                                         &has_height,
                                         &svg_height,
                                         &has_viewbox,
                                         &svg_viewbox);
    if (has_width)
        width  = svg_width.length;

    if (has_height)
        height = svg_height.length;

    /* We set the viewport for the call rsvg_handle_render_layer() to
     * width/height. That way if either dimension is not specified in
     * the SVG it will be inherited from the viewport we provide.
     *
     * As this scales up the rendered dimensions by width/height we
     * need to undo this scaling to get a unit square scale that
     * matches the cairo SVG renderer scale. The OpenType SVG spec
     * does not say what to do if width != height. In this case we
     * will just use a uniform scale that ensures the viewport fits in
     * the unit square and also center it.
     */

    if (width > height) {
        cairo_scale (cr, 1.0/width, 1.0/width);
        cairo_translate (cr, 0, (width - height)/2.0);
    } else {
        cairo_scale (cr, 1.0/height, 1.0/height);
        cairo_translate (cr, (height - width)/2.0, 0);
    }

    viewport.x = 0;
    viewport.y = 0;
    viewport.width = width;
    viewport.height = height;
    if (!rsvg_handle_render_layer (handle, cr, glyph_id, &viewport, &error)) {
        printf ("librsvg render failed: %s", error->message);
        return;
    }
}

static void
render_svg (const char *svg_document, svg_renderer_t renderer, const char* out_file)
{
    double x, y, w, h;

    cairo_surface_t *recording = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    cairo_t *cr = cairo_create (recording);

    /* Scale up to image size when recording to reduce rounding errors
     * in cairo_recording_surface_ink_extents()
     */
    cairo_scale (cr, WIDTH, HEIGHT);

    if (renderer == CAIRO_SVG) {
        cairo_render (svg_document, cr);
    } else {
        librsvg_render (svg_document, cr);
    }
    cairo_destroy (cr);

    if (bbox) {
        cairo_recording_surface_ink_extents (recording, &x, &y, &w, &h);
        if (renderer == CAIRO_SVG)
            printf("cairo  ");
        else
            printf("librsvg");

        printf(" bbox left: %d  right: %d  top: %d  bottom: %d\n",
               (int)floor(x),
               (int)ceil(x + w),
               (int)floor(y),
               (int)ceil(y + h));
    }

    cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    cr = cairo_create (surface);

    /* If rendering a glyph need to translate base line to bottom of image */
    if (standard_svg)
        cairo_set_source_surface (cr, recording, 0, 0);
    else
        cairo_set_source_surface (cr, recording, 0, HEIGHT);

    cairo_paint (cr);
    
    cairo_surface_write_to_png (surface, out_file);
    cairo_surface_destroy (surface);
}

static char *
create_output_name (const char *svg_file, svg_renderer_t renderer)
{
    char buf[1000];
    int len;

    strcpy (buf, svg_file);
    len = strlen (buf);
    
    if (strlen (buf) > 5 && strcmp (buf + len - 4, ".svg") == 0)
        buf[len - 4] = 0;

    if (renderer == CAIRO_SVG)
        strcat (buf, ".cairo.png");
    else
        strcat (buf, ".rsvg.png");

    return strdup (buf);
}

static char *
read_file(const char *filename)
{
    FILE *fp;
    int len;
    char *data;

    fp = fopen (filename, "r");
    if (fp == NULL)
        return NULL;

    fseek (fp, 0, SEEK_END);
    len = ftell(fp);
    rewind (fp);
    data = malloc (len + 1);
    if (fread(data, len, 1, fp) != 1)
        return NULL;
    data[len] = 0;
    fclose(fp);
    return data;
}

static void
usage_and_exit()
{
    printf ("svg-render [-b] [-s] [-g <id>] [-e <em_size> ] input.svg\n");
    exit (1);
}

int
main(int argc, char **argv)
{
    const char *input_file = NULL;
    char *svg_document;
    char *output_file;

    argc--;
    argv++;
    while (argc > 0) {
        if (strcmp (argv[0], "-b") == 0) {
            bbox = TRUE;
            argc--;
            argv++;
        } else if (strcmp (argv[0], "-s") == 0) {
            standard_svg = TRUE;
            argc--;
            argv++;
        } else if (strcmp (argv[0], "-g") == 0) {
            if (argc > 1) {
                glyph_id = argv[1];
                argc -= 2;
                argv += 2;
            } else {
                usage_and_exit();
            }
        } else if (strcmp (argv[0], "-e") == 0) {
            if (argc > 1) {
                em_size = atoi (argv[1]);
                if (em_size <= 0) {
                    usage_and_exit();
                }
                argc -= 2;
                argv += 2;
            } else {
                usage_and_exit();
            }
        } else {
            input_file = argv[0];
            argc--;
            argv++;
        }
    }
    if (!input_file)
        usage_and_exit();

    svg_document = read_file (input_file);
    if (!svg_document) {
        printf("error reading file %s\n", input_file);
        exit (1);
    }

    output_file = create_output_name (input_file, CAIRO_SVG);
    render_svg (svg_document, CAIRO_SVG, output_file);
    free (output_file);

    output_file = create_output_name (input_file, LIBRSVG);
    render_svg (svg_document, LIBRSVG, output_file);
    free (output_file);

    free (svg_document);

    return 0;
}
