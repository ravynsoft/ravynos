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

#include "cairo-test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

#include <cairo.h>

#if CAIRO_HAS_PDF_SURFACE

#include <cairo-pdf.h>

/* This test checks PDF with
 * - tagged text
 * - hyperlinks
 * - document outline
 * - metadata
 * - thumbnails
 * - page labels
 */

#define BASENAME "pdf-tagged-text.out"

#define PAGE_WIDTH 595
#define PAGE_HEIGHT 842

#define HEADING1_SIZE 16
#define HEADING2_SIZE 14
#define HEADING3_SIZE 12
#define TEXT_SIZE 12
#define HEADING_HEIGHT 50
#define MARGIN 50

struct section {
    int level;
    const char *heading;
    int num_paragraphs;
};

static const struct section contents[] = {
    { 0, "Chapter 1",     1 },
    { 1, "Section 1.1",   4 },
    { 2, "Section 1.1.1", 3 },
    { 1, "Section 1.2",   2 },
    { 2, "Section 1.2.1", 4 },
    { 2, "Section 1.2.2", 4 },
    { 1, "Section 1.3",   2 },
    { 0, "Chapter 2",     1 },
    { 1, "Section 2.1",   4 },
    { 2, "Section 2.1.1", 3 },
    { 1, "Section 2.2",   2 },
    { 2, "Section 2.2.1", 4 },
    { 2, "Section 2.2.2", 4 },
    { 1, "Section 2.3",   2 },
    { 0, "Chapter 3",     1 },
    { 1, "Section 3.1",   4 },
    { 2, "Section 3.1.1", 3 },
    { 1, "Section 3.2",   2 },
    { 2, "Section 3.2.1", 4 },
    { 2, "Section 3.2.2", 4 },
    { 1, "Section 3.3",   2 },
    { 0, NULL }
};

static const char *ipsum_lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing"
    " elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
    " Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi"
    " ut aliquip ex ea commodo consequat. Duis aute irure dolor in"
    " reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    " pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa"
    " qui officia deserunt mollit anim id est laborum.";

static const char *roman_numerals[] = {
    "i", "ii", "iii", "iv", "v"
};

#define MAX_PARAGRAPH_LINES 20

static int paragraph_num_lines;
static char *paragraph_text[MAX_PARAGRAPH_LINES];
static double paragraph_height;
static double line_height;
static double y_pos;
static int outline_parents[10];
static int page_num;

static void
layout_paragraph (cairo_t *cr)
{
    char *text, *begin, *end, *prev_end;
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;

    cairo_select_font_face (cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, TEXT_SIZE);
    cairo_font_extents (cr, &font_extents);
    line_height = font_extents.height;
    paragraph_height = 0;
    paragraph_num_lines = 0;
    text = strdup (ipsum_lorem);
    begin = text;
    end = text;
    prev_end = end;
    while (*begin) {
	end = strchr(end, ' ');
	if (!end) {
	    paragraph_text[paragraph_num_lines++] = strdup (begin);
	    break;
	}
	*end = 0;
	cairo_text_extents (cr, begin, &text_extents);
	*end = ' ';
	if (text_extents.width + 2*MARGIN > PAGE_WIDTH) {
	    int len = prev_end - begin;
	    char *s = xmalloc (len);
	    memcpy (s, begin, len);
	    s[len-1] = 0;
	    paragraph_text[paragraph_num_lines++] = s;
	    begin = prev_end + 1;
	}
	prev_end = end;
	end++;
    }
    paragraph_height = line_height * (paragraph_num_lines + 1);
    free (text);
}

static void
draw_paragraph (cairo_t *cr)
{
    int i;

    cairo_select_font_face (cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, TEXT_SIZE);
    cairo_tag_begin (cr, "P", NULL);
    for (i = 0; i < paragraph_num_lines; i++) {
	cairo_move_to (cr, MARGIN, y_pos);
	cairo_show_text (cr, paragraph_text[i]);
	y_pos += line_height;
    }
    cairo_tag_end (cr, "P");
    y_pos += line_height;
}

static void
draw_page_num (cairo_surface_t *surface, cairo_t *cr, const char *prefix, int num)
{
    char buf[100];

    buf[0] = 0;
    if (prefix)
	strcat (buf, prefix);

    if (num)
	sprintf (buf + strlen(buf), "%d", num);

    cairo_save (cr);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_move_to (cr, PAGE_WIDTH/2, PAGE_HEIGHT - MARGIN);
    cairo_show_text (cr, buf);
    cairo_restore (cr);
    cairo_pdf_surface_set_page_label (surface, buf);
}

static void
draw_contents (cairo_surface_t *surface, cairo_t *cr, const struct section *section)
{
    char *attrib;

    xasprintf (&attrib, "dest='%s'", section->heading);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    switch (section->level) {
	case 0:
	    cairo_set_font_size(cr, HEADING1_SIZE);
	    break;
	case 1:
	    cairo_set_font_size(cr, HEADING2_SIZE);
	    break;
	case 2:
	    cairo_set_font_size(cr, HEADING3_SIZE);
	    break;
    }

    if (y_pos + HEADING_HEIGHT + MARGIN > PAGE_HEIGHT) {
	cairo_show_page (cr);
	draw_page_num (surface, cr, roman_numerals[page_num++], 0);
	y_pos = MARGIN;
    }
    cairo_move_to (cr, MARGIN, y_pos);
    cairo_save (cr);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_tag_begin (cr, "TOCI", NULL);
    cairo_tag_begin (cr, "Reference", NULL);
    cairo_tag_begin (cr, CAIRO_TAG_LINK, attrib);
    cairo_show_text (cr, section->heading);
    cairo_tag_end (cr, CAIRO_TAG_LINK);
    cairo_tag_end (cr, "Reference");
    cairo_tag_end (cr, "TOCI");
    cairo_restore (cr);
    y_pos += HEADING_HEIGHT;
    free (attrib);
}

static void
draw_section (cairo_surface_t *surface, cairo_t *cr, const struct section *section)
{
    int flags, i;
    char *name_attrib;
    char *dest_attrib;

    cairo_tag_begin (cr, "Sect", NULL);
    xasprintf(&name_attrib, "name='%s'", section->heading);
    xasprintf(&dest_attrib, "dest='%s'", section->heading);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    if (section->level == 0) {
	cairo_show_page (cr);
	draw_page_num (surface, cr, NULL, page_num++);
	cairo_set_font_size(cr, HEADING1_SIZE);
	cairo_move_to (cr, MARGIN, MARGIN);
	cairo_tag_begin (cr, "H1", NULL);
	cairo_tag_begin (cr, CAIRO_TAG_DEST, name_attrib);
	cairo_show_text (cr, section->heading);
	cairo_tag_end (cr, CAIRO_TAG_DEST);
	cairo_tag_end (cr, "H1");
	y_pos = MARGIN + HEADING_HEIGHT;
	flags = CAIRO_PDF_OUTLINE_FLAG_BOLD | CAIRO_PDF_OUTLINE_FLAG_OPEN;
	outline_parents[0] = cairo_pdf_surface_add_outline (surface,
							    CAIRO_PDF_OUTLINE_ROOT,
							    section->heading,
							    dest_attrib,
							    flags);
    } else {
	if (section->level == 1) {
	    cairo_set_font_size(cr, HEADING2_SIZE);
	    flags = 0;
	} else {
	    cairo_set_font_size(cr, HEADING3_SIZE);
	    flags = CAIRO_PDF_OUTLINE_FLAG_ITALIC;
	}

	if (y_pos + HEADING_HEIGHT + paragraph_height + MARGIN > PAGE_HEIGHT) {
	    cairo_show_page (cr);
	    draw_page_num (surface, cr, NULL, page_num++);
	    y_pos = MARGIN;
	}
	cairo_move_to (cr, MARGIN, y_pos);
	if (section->level == 1)
	    cairo_tag_begin (cr, "H2", NULL);
	else
	    cairo_tag_begin (cr, "H3", NULL);
	cairo_tag_begin (cr, CAIRO_TAG_DEST, name_attrib);
	cairo_show_text (cr, section->heading);
	cairo_tag_end (cr, CAIRO_TAG_DEST);
	if (section->level == 1)
	    cairo_tag_end (cr, "H2");
	else
	    cairo_tag_end (cr, "H3");
	y_pos += HEADING_HEIGHT;
	outline_parents[section->level] = cairo_pdf_surface_add_outline (surface,
									 outline_parents[section->level - 1],
									 section->heading,
									 dest_attrib,
									 flags);
    }

    for (i = 0; i < section->num_paragraphs; i++) {
	if (y_pos + paragraph_height + MARGIN > PAGE_HEIGHT) {
	    cairo_show_page (cr);
	    draw_page_num (surface, cr, NULL, page_num++);
		y_pos = MARGIN;
	}
	draw_paragraph (cr);
    }
    cairo_tag_end (cr, "Sect");
    free (name_attrib);
    free (dest_attrib);
}

static void
draw_cover (cairo_surface_t *surface, cairo_t *cr)
{
    cairo_text_extents_t text_extents;
    char *attrib;
    cairo_rectangle_t url_box;
    const char *cairo_url = "https://www.cairographics.org/";
    const double url_box_margin = 20.0;

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='cover'  internal");
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_move_to (cr, PAGE_WIDTH/3, PAGE_HEIGHT/3);
    cairo_tag_begin (cr, "Span", NULL);
    cairo_show_text (cr, "PDF Features Test");
    cairo_tag_end (cr, "Span");

    /* Test URL link using "rect" attribute. The entire rectangle surrounding the URL should be a clickable link.  */
    cairo_move_to (cr, PAGE_WIDTH/3, 2*PAGE_HEIGHT/3);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, TEXT_SIZE);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_show_text (cr, cairo_url);
    cairo_text_extents (cr, cairo_url, &text_extents);
    url_box.x = PAGE_WIDTH/3 - url_box_margin;
    url_box.y = 2*PAGE_HEIGHT/3 - url_box_margin;
    url_box.width = text_extents.width + 2*url_box_margin;
    url_box.height = -text_extents.height + 2*url_box_margin;
    cairo_rectangle(cr, url_box.x, url_box.y, url_box.width, url_box.height);
    cairo_stroke(cr);
    xasprintf(&attrib, "rect=[%f %f %f %f] uri=\'%s\'",
             url_box.x, url_box.y, url_box.width, url_box.height, cairo_url);
    cairo_tag_begin (cr, CAIRO_TAG_LINK, attrib);
    cairo_tag_end (cr, CAIRO_TAG_LINK);
    free (attrib);

    /* Create link to not yet emmited page number */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "page=5");
    cairo_move_to (cr, PAGE_WIDTH/3, 4*PAGE_HEIGHT/5);
    cairo_show_text (cr, "link to page 5");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    /* Create link to not yet emmited destination */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='Section 3.3'");
    cairo_move_to (cr, PAGE_WIDTH/3, 4.2*PAGE_HEIGHT/5);
    cairo_show_text (cr, "link to page section 3.3");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    /* Create link to external file */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "file='foo.pdf' page=1");
    cairo_move_to (cr, PAGE_WIDTH/3, 4.4*PAGE_HEIGHT/5);
    cairo_show_text (cr, "link file 'foo.pdf'");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    draw_page_num (surface, cr, "cover", 0);
}

static void
create_document (cairo_surface_t *surface, cairo_t *cr)
{
    layout_paragraph (cr);

    cairo_pdf_surface_set_thumbnail_size (surface, PAGE_WIDTH/10, PAGE_HEIGHT/10);

    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_TITLE, "PDF Features Test");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_AUTHOR, "cairo test suite");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_SUBJECT, "cairo test");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_KEYWORDS,
				    "tags, links, outline, page labels, metadata, thumbnails");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_CREATOR, "pdf-features");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_CREATE_DATE, "2016-01-01T12:34:56+10:30");
    cairo_pdf_surface_set_metadata (surface, CAIRO_PDF_METADATA_MOD_DATE, "2016-06-21T05:43:21Z");

    cairo_pdf_surface_set_custom_metadata (surface, "DocumentNumber", "12345");
    /* Include some non ASCII characters */
    cairo_pdf_surface_set_custom_metadata (surface, "Document Name", "\xc2\xab""cairo test\xc2\xbb");
    /* Test unsetting custom metadata. "DocumentNumber" should not be emitted. */
    cairo_pdf_surface_set_custom_metadata (surface, "DocumentNumber", "");

    cairo_tag_begin (cr, "Document", NULL);

    draw_cover (surface, cr);
    cairo_pdf_surface_add_outline (surface,
				   CAIRO_PDF_OUTLINE_ROOT,
				   "Cover", "page=1",
                                   CAIRO_PDF_OUTLINE_FLAG_BOLD);

    /* Create a simple link annotation. */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "uri='http://example.org' rect=[10 10 20 20]");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    /* Try to create a link annotation while the clip is empty;
     * it will still be emitted.
     */
    cairo_save (cr);
    cairo_new_path (cr);
    cairo_rectangle (cr, 100, 100, 50, 0);
    cairo_clip (cr);
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "uri='http://example.com' rect=[100 100 20 20]");
    cairo_tag_end (cr, CAIRO_TAG_LINK);
    cairo_restore (cr);

    /* An annotation whose rect has a negative coordinate. */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "uri='http://127.0.0.1/' rect=[10.0 -10.0 100.0 100.0]");
    cairo_tag_end (cr, CAIRO_TAG_LINK);


    /* Distilled from Mozilla bug https://bugzilla.mozilla.org/show_bug.cgi?id=1725743:
     * attempting to emit a Destination tag within a pushed group will lead to an
     * assertion in _cairo_pdf_interchange_end_structure_tag when processing a
     * following LINK tag that is outside the pushed group.
     */

    /* PushLayer */
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR_ALPHA);

    /* Destination */
    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='a' x=42 y=42");
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    /* PopLayer */
    cairo_pop_group_to_source (cr);
    cairo_paint_with_alpha (cr, 1);
    cairo_set_source_rgb (cr, 0, 0, 0);

    /* Link */
    cairo_tag_begin (cr, CAIRO_TAG_LINK, "rect=[100 200 300 400] uri='http://127.0.0.1/'");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    /* End of extra Mozilla testcase. */


    cairo_show_page (cr);

    page_num = 0;
    draw_page_num (surface, cr, roman_numerals[page_num++], 0);
    y_pos = MARGIN;

    cairo_pdf_surface_add_outline (surface,
				   CAIRO_PDF_OUTLINE_ROOT,
				   "Contents", "dest='TOC'",
                                   CAIRO_PDF_OUTLINE_FLAG_BOLD);

    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='TOC' internal");
    cairo_tag_begin (cr, "TOC", NULL);
    const struct section *sect = contents;
    while (sect->heading) {
	draw_contents (surface, cr, sect);
	sect++;
    }
    cairo_tag_end (cr, "TOC");
    cairo_tag_end (cr, CAIRO_TAG_DEST);

    page_num = 1;
    sect = contents;
    while (sect->heading) {
	draw_section (surface, cr, sect);
	sect++;
    }

    cairo_show_page (cr);

    cairo_set_source_rgb (cr, 0, 0, 1);

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "dest='cover'");
    cairo_move_to (cr, PAGE_WIDTH/3, 2*PAGE_HEIGHT/5);
    cairo_show_text (cr, "link to cover");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_begin (cr, CAIRO_TAG_LINK, "page=3");
    cairo_move_to (cr, PAGE_WIDTH/3, 3*PAGE_HEIGHT/5);
    cairo_show_text (cr, "link to page 3");
    cairo_tag_end (cr, CAIRO_TAG_LINK);

    cairo_tag_end (cr, "Document");
}

#ifdef HAVE_MMAP
static cairo_test_status_t
check_contains_string(cairo_test_context_t *ctx, const void *hay, size_t size, const char *needle)
{
    if (memmem(hay, size, needle, strlen(needle)))
        return CAIRO_TEST_SUCCESS;

    cairo_test_log (ctx, "Failed to find expected string in generated PDF: %s\n", needle);
    return CAIRO_TEST_FAILURE;
}
#endif

static cairo_test_status_t
check_created_pdf(cairo_test_context_t *ctx, const char* filename)
{
    cairo_test_status_t result = CAIRO_TEST_SUCCESS;
    int fd;
    struct stat st;
#ifdef HAVE_MMAP
    void *contents;
#endif

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0) {
        cairo_test_log (ctx, "Failed to open generated PDF file %s: %s\n", filename, strerror(errno));
        return CAIRO_TEST_FAILURE;
    }

    if (fstat(fd, &st) == -1)
    {
        cairo_test_log (ctx, "Failed to stat generated PDF file %s: %s\n", filename, strerror(errno));
        close(fd);
        return CAIRO_TEST_FAILURE;
    }

    if (st.st_size == 0)
    {
        cairo_test_log (ctx, "Generated PDF file %s is empty\n", filename);
        close(fd);
        return CAIRO_TEST_FAILURE;
    }

#ifdef HAVE_MMAP
    contents = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (contents == NULL)
    {
        cairo_test_log (ctx, "Failed to mmap generated PDF file %s: %s\n", filename, strerror(errno));
        close(fd);
        return CAIRO_TEST_FAILURE;
    }

    /* check metadata */
    result |= check_contains_string(ctx, contents, st.st_size, "/Title (PDF Features Test)");
    result |= check_contains_string(ctx, contents, st.st_size, "/Author (cairo test suite)");
    result |= check_contains_string(ctx, contents, st.st_size, "/Creator (pdf-features)");
    result |= check_contains_string(ctx, contents, st.st_size, "/CreationDate (20160101123456+10'30')");
    result |= check_contains_string(ctx, contents, st.st_size, "/ModDate (20160621054321Z)");

    /* check that both the example.org and example.com links were generated */
    result |= check_contains_string(ctx, contents, st.st_size, "http://example.org");
    result |= check_contains_string(ctx, contents, st.st_size, "http://example.com");

    // TODO: add more checks

    munmap(contents, st.st_size);
#endif

    close(fd);

    return result;
}

static cairo_test_status_t
create_pdf (cairo_test_context_t *ctx, cairo_bool_t check_output)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status, status2;
    cairo_test_status_t result;
    cairo_pdf_version_t version;
    char *filename;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    /* check_created_pdf() only works with version 1.4. In version 1.5
     * the text that is searched for is compressed. */
    version = check_output ? CAIRO_PDF_VERSION_1_4 : CAIRO_PDF_VERSION_1_5;

    xasprintf (&filename, "%s/%s-%s.pdf",
               path,
               BASENAME,
               check_output ? "1.4" : "1.5");
    surface = cairo_pdf_surface_create (filename, PAGE_WIDTH, PAGE_HEIGHT);

    cairo_pdf_surface_restrict_to_version (surface, version);

    cr = cairo_create (surface);
    create_document (surface, cr);

    status = cairo_status (cr);
    cairo_destroy (cr);
    cairo_surface_finish (surface);
    status2 = cairo_surface_status (surface);
    if (status == CAIRO_STATUS_SUCCESS)
	status = status2;

    cairo_surface_destroy (surface);
    if (status) {
	cairo_test_log (ctx, "Failed to create pdf surface for file %s: %s\n",
			filename, cairo_status_to_string (status));
	return CAIRO_TEST_FAILURE;
    }

    result = CAIRO_TEST_SUCCESS;
    if (check_output)
        result = check_created_pdf(ctx, filename);

    free (filename);

    return result;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_test_status_t result;

    if (! cairo_test_is_target_enabled (ctx, "pdf"))
	return CAIRO_TEST_UNTESTED;

    /* Create version 1.5 PDF. This can only be manually checked */
    create_pdf (ctx, FALSE);

    /* Create version 1.4 PDF and checkout output */
    result = create_pdf (ctx, TRUE);


    return result;
}

CAIRO_TEST (pdf_tagged_text,
	    "Check tagged text, hyperlinks and PDF document features",
	    "pdf", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)

#endif /* CAIRO_HAS_PDF_SURFACE */
