/*
 * Copyright © 2023 Adrian Johnson
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

#include "cairo-test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* __unix__ */
#endif

#include <cairo.h>
#include <cairo-pdf.h>

/* Test PDF logical structure
 */

#define BASENAME "pdf-structure"

#define PAGE_WIDTH 595
#define PAGE_HEIGHT 842

#define PDF_VERSION CAIRO_PDF_VERSION_1_4

struct pdf_structure_test {
    const char *name;
    void (*func)(cairo_t *cr);
};

static void
text(cairo_t *cr, const char *text)
{
    double x, y;

    cairo_show_text (cr, text);
    cairo_get_current_point (cr, &x, &y);
    cairo_move_to (cr, 20, y + 15);
}

static void
test_simple (cairo_t *cr)
{
    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    text (cr, "Heading");
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, "P");

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para3");

    cairo_tag_begin (cr, "Note", "");
    text (cr, "Note");
    cairo_tag_end (cr, "Note");

    text (cr, "Para4");
    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_simple_ref (cairo_t *cr)
{
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='H' id='heading'");
    text (cr, "Heading");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para1'");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para2'");
    text (cr, "Para3");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='Note' id='note'");
    text (cr, "Note");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para3'");
    text (cr, "Para4");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='heading'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_tag_begin (cr, "P", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para1'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "P");

    cairo_tag_begin (cr, "P", "");

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para2'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);

    cairo_tag_begin (cr, "Note", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='note'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "Note");

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para3'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);

    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_group (cairo_t *cr)
{
    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    text (cr, "Heading");
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_push_group (cr);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, "P");

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_group_ref (cairo_t *cr)
{
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='H' id='heading'");
    text (cr, "Heading");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_push_group (cr);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para'");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='heading'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_tag_begin (cr, "P", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");

}

static void
test_repeated_group (cairo_t *cr)
{
    cairo_pattern_t *pat;

    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    text (cr, "Heading");
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_push_group (cr);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, "P");

    pat = cairo_pop_group (cr);

    cairo_set_source (cr, pat);
    cairo_paint (cr);

    cairo_translate (cr, 0, 100);
    cairo_set_source (cr, pat);
    cairo_rectangle (cr, 0, 0, 100, 100);
    cairo_fill (cr);

    cairo_translate (cr, 0, 100);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_mask (cr, pat);

    cairo_translate (cr, 0, 100);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_move_to (cr, 20, 0);
    cairo_line_to (cr, 100, 0);
    cairo_stroke (cr);

    cairo_translate (cr, 0, 100);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, 20, 0);
    cairo_show_text (cr, "Text");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_multipage_simple (cairo_t *cr)
{
    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='para1-dest'");
    text (cr, "Heading1");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='para2-dest'");
    text (cr, "Heading2");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_show_page (cr);

    cairo_tag_begin (cr, "P", "");

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='para1-dest' internal");
    text (cr, "Para1");
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    cairo_show_page (cr);

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='para2-dest' internal");
    text (cr, "Para2");
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_multipage_simple_ref (cairo_t *cr)
{
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='H' id='heading1'");
    text (cr, "Heading1");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='H' id='heading2'");
    text (cr, "Heading2");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);

    cairo_show_page (cr);

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='para1-dest' internal");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para1'");
    text (cr, "Para1");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    cairo_show_page (cr);

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='para2-dest' internal");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT, "tag_name='P' id='para2'");
    text (cr, "Para2");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT);
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='para1-dest' link_page=1");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='heading1'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='para2-dest' link_page=1");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='heading2'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_tag_begin (cr, "P", "");
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para1'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_begin (cr, CAIRO_TAG_CONTENT_REF, "ref='para2'");
    cairo_tag_end (cr, CAIRO_TAG_CONTENT_REF);
    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static void
test_multipage_group (cairo_t *cr)
{
    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    text (cr, "Heading");
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_push_group (cr);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, "P");

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_show_page (cr);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para3");
    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

/* Same as test_multipage_group but but repeat the group on the second page. */
static void
test_multipage_group2 (cairo_t *cr)
{
    cairo_tag_begin (cr, "Document", NULL);

    cairo_tag_begin (cr, "H", "");
    text (cr, "Heading");
    cairo_tag_end (cr, "H");

    cairo_tag_begin (cr, "Sect", NULL);

    cairo_push_group (cr);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para1");
    text (cr, "Para2");
    cairo_tag_end (cr, "P");

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    cairo_show_page (cr);

    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_tag_begin (cr, "P", "");
    text (cr, "Para3");
    cairo_tag_end (cr, "P");

    cairo_tag_end (cr, "Sect");

    cairo_tag_end (cr, "Document");
}

static const struct pdf_structure_test pdf_structure_tests[] = {
    { "simple", test_simple },
    { "simple-ref", test_simple_ref },
    { "group", test_group },
    { "group-ref", test_group_ref },
    { "repeated-group", test_repeated_group },
    { "multipage-simple", test_multipage_simple },
    { "multipage-simple-ref", test_multipage_simple_ref },
    { "multipage-group", test_multipage_group },
    { "multipage-group2", test_multipage_group2 },
};

static cairo_test_status_t
create_pdf (cairo_test_context_t *ctx, const struct pdf_structure_test *test, const char *output)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status, status2;

    surface = cairo_pdf_surface_create (output, PAGE_WIDTH, PAGE_HEIGHT);

    cairo_pdf_surface_restrict_to_version (surface, PDF_VERSION);

    cr = cairo_create (surface);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Serif",
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 10);
    cairo_move_to (cr, 20, 20);

    test->func(cr);

    status = cairo_status (cr);
    cairo_destroy (cr);
    cairo_surface_finish (surface);
    status2 = cairo_surface_status (surface);
    if (status == CAIRO_STATUS_SUCCESS)
	status = status2;

    cairo_surface_destroy (surface);
    if (status) {
	cairo_test_log (ctx, "Failed to create pdf surface for file %s: %s\n",
			output, cairo_status_to_string (status));
	return CAIRO_TEST_FAILURE;
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
check_pdf (cairo_test_context_t *ctx, const struct pdf_structure_test *test, const char *output)
{
    char *command;
    int ret;
    cairo_test_status_t result = CAIRO_TEST_FAILURE;

    /* check-pdf-structure.sh <pdf-file> <pdfinfo-output> <pdfinfo-ref> <diff-output> */
    xasprintf (&command,
               "%s/check-pdf-structure.sh  %s  %s/%s-%s.out.txt  %s/%s-%s.ref.txt %s/%s-%s.diff.txt ",
               ctx->srcdir,
               output,
               ctx->output, BASENAME, test->name,
               ctx->refdir, BASENAME, test->name,
               ctx->output, BASENAME, test->name);

    ret = system (command);
    cairo_test_log (ctx, "%s  exit code %d\n", command,
                    WIFEXITED (ret) ? WEXITSTATUS (ret) : -1);

    if (WIFEXITED (ret)) {
        if (WEXITSTATUS (ret) == 0)
            result = CAIRO_TEST_SUCCESS;
        else if (WEXITSTATUS (ret) == 4)
            result = CAIRO_TEST_UNTESTED; /* pdfinfo not found, wrong version, missing ref */
    }

    free (command);
    return result;
}

static void
merge_test_status (cairo_test_status_t *current, cairo_test_status_t new)
{
    if (new == CAIRO_TEST_FAILURE || *current == CAIRO_TEST_FAILURE)
        *current = CAIRO_TEST_FAILURE;
    else if (new == CAIRO_TEST_UNTESTED)
        *current = CAIRO_TEST_UNTESTED;
    else
        *current = new;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    int i;
    char *filename;
    cairo_test_status_t result, all_results;
    cairo_bool_t can_check = FALSE;

/* Need a POSIX shell to run the check. */
#ifdef __unix__
    can_check = TRUE;
#endif

    all_results = CAIRO_TEST_SUCCESS;
    if (! cairo_test_is_target_enabled (ctx, "pdf"))
	return CAIRO_TEST_UNTESTED;

    for (i = 0; i < ARRAY_LENGTH(pdf_structure_tests); i++) {
        xasprintf (&filename, "%s/%s-%s.out.pdf",
                   ctx->output,
                   BASENAME,
                   pdf_structure_tests[i].name);

        result = create_pdf (ctx, &pdf_structure_tests[i], filename);
        merge_test_status (&all_results, result);

        if (can_check && result == CAIRO_TEST_SUCCESS) {
            result = check_pdf (ctx, &pdf_structure_tests[i], filename);
            merge_test_status (&all_results, result);
        } else {
            merge_test_status (&all_results, CAIRO_TEST_UNTESTED);
        }
    }

    free (filename);
    return all_results;
}

CAIRO_TEST (pdf_structure,
	    "Check PDF Structure",
	    "pdf", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
