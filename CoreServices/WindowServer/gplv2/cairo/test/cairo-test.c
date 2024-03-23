/*
 * Copyright © 2004 Red Hat, Inc.
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>
#if HAVE_FCFINI
#include <fontconfig/fontconfig.h>
#endif
#if CAIRO_HAS_REAL_PTHREAD
#include <pthread.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_VALGRIND
#include <valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif

#if HAVE_MEMFAULT
#include <memfault.h>
#define MF(x) x
#else
#define MF(x)
#endif

#include "cairo-test-private.h"

#include "buffer-diff.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#include <direct.h>
#define F_OK 0
#define HAVE_MKDIR 1
#define mkdir _mkdir
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

#if ! HAVE_ALARM || ! defined(SIGALRM)
#define alarm(X);
#endif

static const cairo_user_data_key_t _cairo_test_context_key;

static void
_xunlink (const cairo_test_context_t *ctx, const char *pathname);

static const char *fail_face = "", *xfail_face="", *normal_face = "";
static cairo_bool_t print_fail_on_stdout;
static int cairo_test_timeout = 60;

#define NUM_DEVICE_OFFSETS 2
#define NUM_DEVICE_SCALE 2

cairo_bool_t
cairo_test_mkdir (const char *path)
{
#if ! HAVE_MKDIR
    return FALSE;
#elif HAVE_MKDIR == 1
    if (mkdir (path) == 0)
	return TRUE;
#elif HAVE_MKDIR == 2
    if (mkdir (path, 0770) == 0)
	return TRUE;
#else
#error Bad value for HAVE_MKDIR
#endif

    return errno == EEXIST;
}

static char *
_cairo_test_fixup_name (const char *original)
{
    char *name, *s;

    s = name = xstrdup (original);
    while ((s = strchr (s, '_')) != NULL)
	*s++ = '-';

    return name;
}

char *
cairo_test_get_name (const cairo_test_t *test)
{
    return _cairo_test_fixup_name (test->name);
}

static void
_cairo_test_init (cairo_test_context_t *ctx,
		  const cairo_test_context_t *parent,
		  const cairo_test_t *test,
		  const char *test_name,
		  const char *output)
{
    char *log_name;

    MF (MEMFAULT_DISABLE_FAULTS ());

#if HAVE_FEENABLEEXCEPT
    feenableexcept (FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

    ctx->test = test;
    ctx->test_name = _cairo_test_fixup_name (test_name);
    ctx->output = output;

    cairo_test_mkdir (ctx->output);

    ctx->malloc_failure = 0;
#if HAVE_MEMFAULT
    if (getenv ("CAIRO_TEST_MALLOC_FAILURE"))
	ctx->malloc_failure = atoi (getenv ("CAIRO_TEST_MALLOC_FAILURE"));
    if (ctx->malloc_failure && ! RUNNING_ON_MEMFAULT ())
	ctx->malloc_failure = 0;
#endif

    ctx->timeout = cairo_test_timeout;
    if (getenv ("CAIRO_TEST_TIMEOUT"))
	ctx->timeout = atoi (getenv ("CAIRO_TEST_TIMEOUT"));

    xasprintf (&log_name, "%s/%s%s", ctx->output, ctx->test_name, CAIRO_TEST_LOG_SUFFIX);
    _xunlink (NULL, log_name);

    ctx->log_file = fopen (log_name, "a");
    if (ctx->log_file == NULL) {
	fprintf (stderr, "Error opening log file: %s\n", log_name);
	ctx->log_file = stderr;
    }
    free (log_name);

    ctx->ref_name = NULL;
    ctx->ref_image = NULL;
    ctx->ref_image_flattened = NULL;

    if (parent != NULL) {
	ctx->targets_to_test = parent->targets_to_test;
	ctx->num_targets = parent->num_targets;
	ctx->limited_targets = parent->limited_targets;
	ctx->own_targets = FALSE;

	ctx->srcdir = parent->srcdir;
	ctx->refdir = xstrdup (parent->refdir);
    } else {
	int tmp_num_targets;
	cairo_bool_t tmp_limited_targets;

	ctx->targets_to_test = cairo_boilerplate_get_targets (&tmp_num_targets, &tmp_limited_targets);
	ctx->num_targets = tmp_num_targets;
	ctx->limited_targets = tmp_limited_targets;
	ctx->own_targets = TRUE;

	ctx->srcdir = getenv ("srcdir");
	if (ctx->srcdir == NULL) {
            ctx->srcdir = ".";
#if HAVE_SYS_STAT_H
            struct stat st;
            if (stat ("srcdir", &st) == 0 && (st.st_mode & S_IFDIR))
                ctx->srcdir = "srcdir";
#endif
        }

	ctx->refdir = xstrdup (getenv ("CAIRO_REF_DIR"));
        if (ctx->refdir == NULL)
            xasprintf (&ctx->refdir, "%s/reference", ctx->srcdir);
    }

#ifdef HAVE_UNISTD_H
    if (*fail_face == '\0' && isatty (2)) {
	fail_face = "\033[41;37;1m";
	xfail_face = "\033[43;37;1m";
	normal_face = "\033[m";
	if (isatty (1))
	    print_fail_on_stdout = FALSE;
    }
#endif

    printf ("\nTESTING %s\n", ctx->test_name);
}

void
_cairo_test_context_init_for_test (cairo_test_context_t *ctx,
				   const cairo_test_context_t *parent,
				   const cairo_test_t *test)
{
    _cairo_test_init (ctx, parent, test, test->name, CAIRO_TEST_OUTPUT_DIR);
}

void
cairo_test_init (cairo_test_context_t *ctx,
		 const char *test_name,
		 const char *output)
{
    _cairo_test_init (ctx, NULL, NULL, test_name, output);
}

void
cairo_test_fini (cairo_test_context_t *ctx)
{
    if (ctx->log_file == NULL)
	return;

    if (ctx->log_file != stderr)
	fclose (ctx->log_file);
    ctx->log_file = NULL;

    free (ctx->refdir);
    free (ctx->ref_name);
    cairo_surface_destroy (ctx->ref_image);
    cairo_surface_destroy (ctx->ref_image_flattened);

    if (ctx->test_name != NULL)
	free ((char *) ctx->test_name);

    if (ctx->own_targets)
	cairo_boilerplate_free_targets (ctx->targets_to_test);

    cairo_boilerplate_fini ();

    cairo_debug_reset_static_data ();
#if HAVE_FCFINI
    FcFini ();
#endif
}

void
cairo_test_logv (const cairo_test_context_t *ctx,
	        const char *fmt, va_list va)
{
    FILE *file = ctx && ctx->log_file ? ctx->log_file : stderr;
    vfprintf (file, fmt, va);
}

void
cairo_test_log (const cairo_test_context_t *ctx, const char *fmt, ...)
{
    va_list va;

    va_start (va, fmt);
    cairo_test_logv (ctx, fmt, va);
    va_end (va);
}

static void
_xunlink (const cairo_test_context_t *ctx, const char *pathname)
{
    if (unlink (pathname) < 0 && errno != ENOENT) {
	cairo_test_log (ctx, "Error: Cannot remove %s: %s\n",
			pathname, strerror (errno));
	exit (1);
    }
}

char *
cairo_test_reference_filename (const cairo_test_context_t *ctx,
			       const char *base_name,
			       const char *test_name,
			       const char *target_name,
			       const char *base_target_name,
			       const char *format,
			       const char *suffix,
			       const char *extension)
{
    char *ref_name = NULL;

    /* First look for a previous build for comparison. */
    if (ctx->refdir != NULL && strcmp(suffix, CAIRO_TEST_REF_SUFFIX) == 0) {
	xasprintf (&ref_name, "%s/%s" CAIRO_TEST_OUT_SUFFIX "%s",
		   ctx->refdir,
		   base_name,
		   extension);
	if (access (ref_name, F_OK) != 0)
	    free (ref_name);
	else
	    goto done;
    }

    if (target_name != NULL) {
	/* Next look for a target/format-specific reference image. */
	xasprintf (&ref_name, "%s/reference/%s.%s.%s%s%s",
		   ctx->srcdir,
		   test_name,
		   target_name,
		   format,
		   suffix,
		   extension);
	if (access (ref_name, F_OK) != 0)
	    free (ref_name);
	else
	    goto done;

	/* Next, look for target-specific reference image. */
	xasprintf (&ref_name, "%s/reference/%s.%s%s%s",
		   ctx->srcdir,
		   test_name,
		   target_name,
		   suffix,
		   extension);
	if (access (ref_name, F_OK) != 0)
	    free (ref_name);
	else
	    goto done;
    }

    if (base_target_name != NULL) {
	/* Next look for a base/format-specific reference image. */
	xasprintf (&ref_name, "%s/reference/%s.%s.%s%s%s",
		   ctx->srcdir,
		   test_name,
		   base_target_name,
		   format,
		   suffix,
		   extension);
	if (access (ref_name, F_OK) != 0)
	    free (ref_name);
	else
	    goto done;

	/* Next, look for base-specific reference image. */
	xasprintf (&ref_name, "%s/reference/%s.%s%s%s",
		   ctx->srcdir,
		   test_name,
		   base_target_name,
		   suffix,
		   extension);
	if (access (ref_name, F_OK) != 0)
	    free (ref_name);
	else
	    goto done;
    }

    /* Next, look for format-specific reference image. */
    xasprintf (&ref_name, "%s/reference/%s.%s%s%s",
	       ctx->srcdir,
	       test_name,
	       format,
	       suffix,
	       extension);
    if (access (ref_name, F_OK) != 0)
	free (ref_name);
    else
	goto done;

    /* Finally, look for the standard reference image. */
    xasprintf (&ref_name, "%s/reference/%s%s%s", ctx->srcdir,
	       test_name,
	       suffix,
	       extension);
    if (access (ref_name, F_OK) != 0)
	free (ref_name);
    else
	goto done;

    ref_name = NULL;

done:
    return ref_name;
}

cairo_test_similar_t
cairo_test_target_has_similar (const cairo_test_context_t *ctx,
			       const cairo_boilerplate_target_t *target)
{
    cairo_surface_t *surface;
    cairo_test_similar_t has_similar;
    cairo_t * cr;
    cairo_surface_t *similar;
    cairo_status_t status;
    void *closure;
    char *path;

    /* ignore image intermediate targets */
    if (target->expected_type == CAIRO_SURFACE_TYPE_IMAGE)
	return DIRECT;

    if (getenv ("CAIRO_TEST_IGNORE_SIMILAR"))
	return DIRECT;

    xasprintf (&path, "%s/%s",
	       cairo_test_mkdir (ctx->output) ? ctx->output : ".",
	       ctx->test_name);

    has_similar = DIRECT;
    do {
	do {
	    surface = (target->create_surface) (path,
						target->content,
						ctx->test->width,
						ctx->test->height,
						ctx->test->width* NUM_DEVICE_SCALE + 25 * NUM_DEVICE_OFFSETS,
						ctx->test->height* NUM_DEVICE_SCALE + 25 * NUM_DEVICE_OFFSETS,
						CAIRO_BOILERPLATE_MODE_TEST,
						&closure);
	    if (surface == NULL)
		goto out;
	} while (cairo_test_malloc_failure (ctx, cairo_surface_status (surface)));

	if (cairo_surface_status (surface))
	    goto out;

	cr = cairo_create (surface);
	cairo_push_group_with_content (cr,
				       cairo_boilerplate_content (target->content));
	similar = cairo_get_group_target (cr);
	status = cairo_surface_status (similar);

	if (cairo_surface_get_type (similar) == cairo_surface_get_type (surface))
	    has_similar = SIMILAR;
	else
	    has_similar = DIRECT;

	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	if (target->cleanup)
	    target->cleanup (closure);
    } while (! has_similar && cairo_test_malloc_failure (ctx, status));
out:
    free (path);

    return has_similar;
}

static cairo_surface_t *
_cairo_test_flatten_reference_image (cairo_test_context_t *ctx,
				     cairo_bool_t flatten)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    if (! flatten)
	return ctx->ref_image;

    if (ctx->ref_image_flattened != NULL)
	return ctx->ref_image_flattened;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					  cairo_image_surface_get_width (ctx->ref_image),
					  cairo_image_surface_get_height (ctx->ref_image));
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_set_source_surface (cr, ctx->ref_image, 0, 0);
    cairo_paint (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    if (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS)
	ctx->ref_image_flattened = surface;
    return surface;
}

cairo_surface_t *
cairo_test_get_reference_image (cairo_test_context_t *ctx,
				const char *filename,
				cairo_bool_t flatten)
{
    cairo_surface_t *surface;

    if (ctx->ref_name != NULL) {
	if (strcmp (ctx->ref_name, filename) == 0)
	    return _cairo_test_flatten_reference_image (ctx, flatten);

	cairo_surface_destroy (ctx->ref_image);
	ctx->ref_image = NULL;

	cairo_surface_destroy (ctx->ref_image_flattened);
	ctx->ref_image_flattened = NULL;

	free (ctx->ref_name);
	ctx->ref_name = NULL;
    }

    surface = cairo_image_surface_create_from_png (filename);
    if (cairo_surface_status (surface))
	return surface;

    ctx->ref_name = xstrdup (filename);
    ctx->ref_image = surface;
    return _cairo_test_flatten_reference_image (ctx, flatten);
}

static cairo_bool_t
cairo_test_file_is_older (const char *filename,
	                  char **ref_filenames,
			  int num_ref_filenames)
{
#if HAVE_SYS_STAT_H
    struct stat st;

    if (stat (filename, &st) < 0)
	return FALSE;

    while (num_ref_filenames--) {
	struct stat ref;
	char *ref_filename = *ref_filenames++;

	if (ref_filename == NULL)
	    continue;

	if (stat (ref_filename++, &ref) < 0)
	    continue;

	if (st.st_mtime <= ref.st_mtime)
	    return TRUE;
    }
#endif

    return FALSE;
}

static cairo_bool_t
cairo_test_files_equal (const char *test_filename,
			const char *pass_filename)
{
    FILE *test, *pass;
    int t, p;

    if (test_filename == NULL || pass_filename == NULL)
	return FALSE;

    test = fopen (test_filename, "rb");
    if (test == NULL)
	return FALSE;

    pass = fopen (pass_filename, "rb");
    if (pass == NULL) {
	fclose (test);
	return FALSE;
    }

    /* as simple as it gets */
    do {
	t = getc (test);
	p = getc (pass);
	if (t != p)
	    break;
    } while (t != EOF && p != EOF);

    fclose (pass);
    fclose (test);

    return t == p; /* both EOF */
}

static cairo_bool_t
cairo_test_copy_file (const char *src_filename,
		      const char *dst_filename)
{
    FILE *src, *dst;
    int c;

#if HAVE_LINK
    if (link (src_filename, dst_filename) == 0)
	return TRUE;

    unlink (dst_filename);
#endif

    src = fopen (src_filename, "rb");
    if (src == NULL)
	return FALSE;

    dst = fopen (dst_filename, "wb");
    if (dst == NULL) {
	fclose (src);
	return FALSE;
    }

    /* as simple as it gets */
    while ((c = getc (src)) != EOF)
	putc (c, dst);

    fclose (src);
    fclose (dst);

    return TRUE;
}

static cairo_test_status_t
cairo_test_for_target (cairo_test_context_t		 *ctx,
		       const cairo_boilerplate_target_t	 *target,
		       int				  dev_offset,
		       int				  dev_scale,
		       cairo_bool_t                       similar)
{
    cairo_status_t finish_status;
    cairo_surface_t *surface = NULL;
    cairo_t *cr;
    const char *empty_str = "";
    char *offset_str;
    char *scale_str;
    char *base_name, *base_path;
    char *out_png_path;
    char *ref_path = NULL, *ref_png_path, *cmp_png_path = NULL;
    char *new_path = NULL, *new_png_path;
    char *xfail_path = NULL, *xfail_png_path;
    char *base_ref_png_path;
    char *base_new_png_path;
    char *base_xfail_png_path;
    char *diff_png_path;
    char *test_filename = NULL, *pass_filename = NULL, *fail_filename = NULL;
    cairo_test_status_t ret, test_status;
    cairo_content_t expected_content;
    cairo_font_options_t *font_options;
    const char *format;
    cairo_bool_t have_output = FALSE;
    cairo_bool_t have_result = FALSE;
    void *closure;
    double width, height;
    cairo_bool_t have_output_dir;
#if HAVE_MEMFAULT
    int malloc_failure_iterations = ctx->malloc_failure;
    int last_fault_count = 0;
#endif

    /* Get the strings ready that we'll need. */
    format = cairo_boilerplate_content_name (target->content);
    if (dev_offset)
	xasprintf (&offset_str, ".%d", dev_offset);
    else
	offset_str = (char *) empty_str;

    if (dev_scale != 1)
	xasprintf (&scale_str, ".x%d", dev_scale);
    else
	scale_str = (char *) empty_str;

    xasprintf (&base_name, "%s.%s.%s%s%s%s",
	       ctx->test_name,
	       target->name,
	       format,
	       similar ? ".similar" : "",
	       offset_str,
               scale_str);

    if (offset_str != empty_str)
      free (offset_str);
    if (scale_str != empty_str)
      free (scale_str);

    ref_png_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  target->name,
						  target->basename,
						  format,
						  CAIRO_TEST_REF_SUFFIX,
						  CAIRO_TEST_PNG_EXTENSION);
    new_png_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  target->name,
						  target->basename,
						  format,
						  CAIRO_TEST_NEW_SUFFIX,
						  CAIRO_TEST_PNG_EXTENSION);
    xfail_png_path = cairo_test_reference_filename (ctx,
						    base_name,
						    ctx->test_name,
						    target->name,
						    target->basename,
						    format,
						    CAIRO_TEST_XFAIL_SUFFIX,
						    CAIRO_TEST_PNG_EXTENSION);

    base_ref_png_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  NULL, NULL,
						  format,
						  CAIRO_TEST_REF_SUFFIX,
						  CAIRO_TEST_PNG_EXTENSION);
    base_new_png_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  NULL, NULL,
						  format,
						  CAIRO_TEST_NEW_SUFFIX,
						  CAIRO_TEST_PNG_EXTENSION);
    base_xfail_png_path = cairo_test_reference_filename (ctx,
						    base_name,
						    ctx->test_name,
						    NULL, NULL,
						    format,
						    CAIRO_TEST_XFAIL_SUFFIX,
						    CAIRO_TEST_PNG_EXTENSION);

    if (target->file_extension != NULL) {
	ref_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  target->name,
						  target->basename,
						  format,
						  CAIRO_TEST_REF_SUFFIX,
						  target->file_extension);
	new_path = cairo_test_reference_filename (ctx,
						  base_name,
						  ctx->test_name,
						  target->name,
						  target->basename,
						  format,
						  CAIRO_TEST_NEW_SUFFIX,
						  target->file_extension);
	xfail_path = cairo_test_reference_filename (ctx,
						    base_name,
						    ctx->test_name,
						    target->name,
						    target->basename,
						    format,
						    CAIRO_TEST_XFAIL_SUFFIX,
						    target->file_extension);
    }

    have_output_dir = cairo_test_mkdir (ctx->output);
    xasprintf (&base_path, "%s/%s",
	       have_output_dir ? ctx->output : ".",
	       base_name);
    xasprintf (&out_png_path, "%s" CAIRO_TEST_OUT_PNG, base_path);
    xasprintf (&diff_png_path, "%s" CAIRO_TEST_DIFF_PNG, base_path);

    if (ctx->test->requirements != NULL) {
	const char *required;

	required = target->is_vector ? "target=raster" : "target=vector";
	if (strstr (ctx->test->requirements, required) != NULL) {
	    cairo_test_log (ctx, "Error: Skipping for %s target %s\n",
			    target->is_vector ? "vector" : "raster",
			    target->name);
	    ret = CAIRO_TEST_UNTESTED;
	    goto UNWIND_STRINGS;
	}

	required = target->is_recording ? "target=!recording" : "target=recording";
	if (strstr (ctx->test->requirements, required) != NULL) {
	    cairo_test_log (ctx, "Error: Skipping for %s target %s\n",
			    target->is_recording ? "recording" : "non-recording",
			    target->name);
	    ret = CAIRO_TEST_UNTESTED;
	    goto UNWIND_STRINGS;
	}
    }

    width = ctx->test->width;
    height = ctx->test->height;
    if (width && height) {
	width *= dev_scale;
	height *= dev_scale;
	width += dev_offset;
	height += dev_offset;
    }

#if HAVE_MEMFAULT
REPEAT:
    MEMFAULT_CLEAR_FAULTS ();
    MEMFAULT_RESET_LEAKS ();
    ctx->last_fault_count = 0;
    last_fault_count = MEMFAULT_COUNT_FAULTS ();

    /* Pre-initialise fontconfig so that the configuration is loaded without
     * malloc failures (our primary goal is to test cairo fault tolerance).
     */
#if HAVE_FCINIT
    FcInit ();
#endif

    MEMFAULT_ENABLE_FAULTS ();
#endif
    have_output = FALSE;
    have_result = FALSE;

    /* Run the actual drawing code. */
    ret = CAIRO_TEST_SUCCESS;
    surface = (target->create_surface) (base_path,
					target->content,
					width, height,
					ctx->test->width * NUM_DEVICE_SCALE + 25 * NUM_DEVICE_OFFSETS,
					ctx->test->height * NUM_DEVICE_SCALE + 25 * NUM_DEVICE_OFFSETS,
					CAIRO_BOILERPLATE_MODE_TEST,
					&closure);
    if (surface == NULL) {
	cairo_test_log (ctx, "Error: Failed to set %s target\n", target->name);
	ret = CAIRO_TEST_UNTESTED;
	goto UNWIND_STRINGS;
    }

#if HAVE_MEMFAULT
    if (ctx->malloc_failure &&
	MEMFAULT_COUNT_FAULTS () - last_fault_count > 0 &&
	cairo_surface_status (surface) == CAIRO_STATUS_NO_MEMORY)
    {
	goto REPEAT;
    }
#endif

    if (cairo_surface_status (surface)) {
	MF (MEMFAULT_PRINT_FAULTS ());
	cairo_test_log (ctx, "Error: Created an error surface: %s\n",
			cairo_status_to_string (cairo_surface_status (surface)));
	ret = CAIRO_TEST_FAILURE;
	goto UNWIND_STRINGS;
    }

    /* Check that we created a surface of the expected type. */
    if (cairo_surface_get_type (surface) != target->expected_type) {
	MF (MEMFAULT_PRINT_FAULTS ());
	cairo_test_log (ctx, "Error: Created surface is of type %d (expected %d)\n",
			cairo_surface_get_type (surface), target->expected_type);
	ret = CAIRO_TEST_UNTESTED;
	goto UNWIND_SURFACE;
    }

    /* Check that we created a surface of the expected content,
     * (ignore the artificial CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED value).
     */
    expected_content = cairo_boilerplate_content (target->content);

    if (cairo_surface_get_content (surface) != expected_content) {
	MF (MEMFAULT_PRINT_FAULTS ());
	cairo_test_log (ctx, "Error: Created surface has content %d (expected %d)\n",
			cairo_surface_get_content (surface), expected_content);
	ret = CAIRO_TEST_FAILURE;
	goto UNWIND_SURFACE;
    }

    if (cairo_surface_set_user_data (surface,
				     &cairo_boilerplate_output_basename_key,
				     base_path,
				     NULL))
    {
#if HAVE_MEMFAULT
	cairo_surface_destroy (surface);

	if (target->cleanup)
	    target->cleanup (closure);

	goto REPEAT;
#else
	ret = CAIRO_TEST_FAILURE;
	goto UNWIND_SURFACE;
#endif
    }

    cairo_surface_set_device_offset (surface, dev_offset, dev_offset);
    cairo_surface_set_device_scale (surface, dev_scale, dev_scale);

    cr = cairo_create (surface);
    if (cairo_set_user_data (cr, &_cairo_test_context_key, (void*) ctx, NULL)) {
#if HAVE_MEMFAULT
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	if (target->cleanup)
	    target->cleanup (closure);

	goto REPEAT;
#else
	ret = CAIRO_TEST_FAILURE;
	goto UNWIND_CAIRO;
#endif
    }

    if (similar)
	cairo_push_group_with_content (cr, expected_content);

    /* Clear to transparent (or black) depending on whether the target
     * surface supports alpha. */
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    
    /* Set all components of font_options to avoid backend differences
     * and reduce number of needed reference images. */
    font_options = cairo_font_options_create ();
    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_ON);
    cairo_font_options_set_antialias (font_options, CAIRO_ANTIALIAS_GRAY);
    cairo_set_font_options (cr, font_options);
    cairo_font_options_destroy (font_options);

    cairo_save (cr);
    alarm (ctx->timeout);
    test_status = (ctx->test->draw) (cr, ctx->test->width, ctx->test->height);
    alarm (0);
    cairo_restore (cr);

    if (similar) {
	cairo_pop_group_to_source (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
    }

#if HAVE_MEMFAULT
    MEMFAULT_DISABLE_FAULTS ();

    /* repeat test after malloc failure injection */
    if (ctx->malloc_failure &&
	MEMFAULT_COUNT_FAULTS () - last_fault_count > 0 &&
	(test_status == CAIRO_TEST_NO_MEMORY ||
	 cairo_status (cr) == CAIRO_STATUS_NO_MEMORY ||
	 cairo_surface_status (surface) == CAIRO_STATUS_NO_MEMORY))
    {
	cairo_destroy (cr);
	cairo_surface_destroy (surface);
	if (target->cleanup)
	    target->cleanup (closure);
	cairo_debug_reset_static_data ();
#if HAVE_FCFINI
	FcFini ();
#endif
	if (MEMFAULT_COUNT_LEAKS () > 0) {
	    MEMFAULT_PRINT_FAULTS ();
	    MEMFAULT_PRINT_LEAKS ();
	}

	goto REPEAT;
    }
#endif

    /* Then, check all the different ways it could fail. */
    if (test_status) {
	cairo_test_log (ctx, "Error: Function under test failed\n");
	ret = test_status;
	goto UNWIND_CAIRO;
    }

#if HAVE_MEMFAULT
    if (MEMFAULT_COUNT_FAULTS () - last_fault_count > 0 &&
	MEMFAULT_HAS_FAULTS ())
    {
	VALGRIND_PRINTF ("Unreported memfaults...");
	MEMFAULT_PRINT_FAULTS ();
    }
#endif

    if (target->finish_surface != NULL) {
#if HAVE_MEMFAULT
	/* We need to re-enable faults as most recording-surface processing
	 * is done during cairo_surface_finish().
	 */
	MEMFAULT_CLEAR_FAULTS ();
	last_fault_count = MEMFAULT_COUNT_FAULTS ();
	MEMFAULT_ENABLE_FAULTS ();
#endif

	/* also check for infinite loops whilst replaying */
	alarm (ctx->timeout);
	finish_status = target->finish_surface (surface);
	alarm (0);

#if HAVE_MEMFAULT
	MEMFAULT_DISABLE_FAULTS ();

	if (ctx->malloc_failure &&
	    MEMFAULT_COUNT_FAULTS () - last_fault_count > 0 &&
	    finish_status == CAIRO_STATUS_NO_MEMORY)
	{
	    cairo_destroy (cr);
	    cairo_surface_destroy (surface);
	    if (target->cleanup)
		target->cleanup (closure);
	    cairo_debug_reset_static_data ();
#if HAVE_FCFINI
	    FcFini ();
#endif
	    if (MEMFAULT_COUNT_LEAKS () > 0) {
		MEMFAULT_PRINT_FAULTS ();
		MEMFAULT_PRINT_LEAKS ();
	    }

	    goto REPEAT;
	}
#endif
	if (finish_status) {
	    cairo_test_log (ctx, "Error: Failed to finish surface: %s\n",
			    cairo_status_to_string (finish_status));
	    ret = CAIRO_TEST_FAILURE;
	    goto UNWIND_CAIRO;
	}
    }

    /* Skip image check for tests with no image (width,height == 0,0) */
    if (ctx->test->width != 0 && ctx->test->height != 0) {
	cairo_surface_t *ref_image;
	cairo_surface_t *test_image;
	cairo_surface_t *diff_image;
	buffer_diff_result_t result;
	cairo_status_t diff_status;

	if (ref_png_path == NULL) {
	    cairo_test_log (ctx, "Error: Cannot find reference image for %s\n",
			    base_name);

	    /* we may be running this test to generate reference images */
	    _xunlink (ctx, out_png_path);
	    /* be more generous as we may need to use external renderers */
	    alarm (4 * ctx->timeout);
	    test_image = target->get_image_surface (surface, 0,
		                                    ctx->test->width,
						    ctx->test->height);
	    alarm (0);
	    diff_status = cairo_surface_write_to_png (test_image, out_png_path);
	    cairo_surface_destroy (test_image);
	    if (diff_status) {
		if (cairo_surface_status (test_image) == CAIRO_STATUS_INVALID_STATUS)
		    ret = CAIRO_TEST_CRASHED;
		else
		    ret = CAIRO_TEST_FAILURE;
		cairo_test_log (ctx,
			        "Error: Failed to write output image: %s\n",
			        cairo_status_to_string (diff_status));
	    }
	    have_output = TRUE;

	    ret = CAIRO_TEST_XFAILURE;
	    goto UNWIND_CAIRO;
	}

	if (target->file_extension != NULL) { /* compare vector surfaces */
	    char *filenames[] = {
		ref_png_path,
		ref_path,
		new_png_path,
		new_path,
		xfail_png_path,
		xfail_path,
		base_ref_png_path,
		base_new_png_path,
		base_xfail_png_path,
	    };

	    xasprintf (&test_filename, "%s.out%s",
		       base_path, target->file_extension);
	    xasprintf (&pass_filename, "%s.pass%s",
		       base_path, target->file_extension);
	    xasprintf (&fail_filename, "%s.fail%s",
		       base_path, target->file_extension);

	    if (cairo_test_file_is_older (pass_filename,
					  filenames,
					  ARRAY_LENGTH (filenames)))
	    {
		_xunlink (ctx, pass_filename);
	    }
	    if (cairo_test_file_is_older (fail_filename,
					  filenames,
					  ARRAY_LENGTH (filenames)))
	    {
		_xunlink (ctx, fail_filename);
	    }

	    if (cairo_test_files_equal (out_png_path, ref_path)) {
		cairo_test_log (ctx, "Vector surface matches reference.\n");
		have_output = FALSE;
		ret = CAIRO_TEST_SUCCESS;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, new_path)) {
		cairo_test_log (ctx, "Vector surface matches current failure.\n");
		have_output = FALSE;
		ret = CAIRO_TEST_NEW;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, xfail_path)) {
		cairo_test_log (ctx, "Vector surface matches known failure.\n");
		have_output = FALSE;
		ret = CAIRO_TEST_XFAILURE;
		goto UNWIND_CAIRO;
	    }

	    if (cairo_test_files_equal (test_filename, pass_filename)) {
		/* identical output as last known PASS */
		cairo_test_log (ctx, "Vector surface matches last pass.\n");
		have_output = TRUE;
		ret = CAIRO_TEST_SUCCESS;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (test_filename, fail_filename)) {
		/* identical output as last known FAIL, fail */
		cairo_test_log (ctx, "Vector surface matches last fail.\n");
		have_result = TRUE; /* presume these were kept around as well */
		have_output = TRUE;
		ret = CAIRO_TEST_FAILURE;
		goto UNWIND_CAIRO;
	    }
	}

	/* be more generous as we may need to use external renderers */
	alarm (4 * ctx->timeout);
	test_image = target->get_image_surface (surface, 0,
					        ctx->test->width,
						ctx->test->height);
	alarm (0);
	if (cairo_surface_status (test_image)) {
	    cairo_test_log (ctx, "Error: Failed to extract image: %s\n",
			    cairo_status_to_string (cairo_surface_status (test_image)));
	    if (cairo_surface_status (test_image) == CAIRO_STATUS_INVALID_STATUS)
		ret = CAIRO_TEST_CRASHED;
	    else
		ret = CAIRO_TEST_FAILURE;
	    cairo_surface_destroy (test_image);
	    goto UNWIND_CAIRO;
	}

	_xunlink (ctx, out_png_path);
	diff_status = cairo_surface_write_to_png (test_image, out_png_path);
	if (diff_status) {
	    cairo_test_log (ctx, "Error: Failed to write output image: %s\n",
			    cairo_status_to_string (diff_status));
	    cairo_surface_destroy (test_image);
	    ret = CAIRO_TEST_FAILURE;
	    goto UNWIND_CAIRO;
	}
	have_output = TRUE;

	/* binary compare png files (no decompression) */
	if (target->file_extension == NULL) {
	    char *filenames[] = {
		ref_png_path,
		new_png_path,
		xfail_png_path,
		base_ref_png_path,
		base_new_png_path,
		base_xfail_png_path,
	    };

	    xasprintf (&test_filename, "%s", out_png_path);
	    xasprintf (&pass_filename, "%s.pass.png", base_path);
	    xasprintf (&fail_filename, "%s.fail.png", base_path);

	    if (cairo_test_file_is_older (pass_filename,
					  filenames,
					  ARRAY_LENGTH (filenames)))
	    {
		_xunlink (ctx, pass_filename);
	    }
	    if (cairo_test_file_is_older (fail_filename,
					  filenames,
					  ARRAY_LENGTH (filenames)))
	    {
		_xunlink (ctx, fail_filename);
	    }

	    if (cairo_test_files_equal (test_filename, pass_filename)) {
		cairo_test_log (ctx, "PNG file exactly matches last pass.\n");
                have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_SUCCESS;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, ref_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches reference image.\n");
                have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_SUCCESS;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, new_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches current failure image.\n");
                have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_NEW;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, xfail_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches known failure image.\n");
                have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_XFAILURE;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (test_filename, fail_filename)) {
		cairo_test_log (ctx, "PNG file exactly matches last fail.\n");
		have_result = TRUE; /* presume these were kept around as well */
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_FAILURE;
		goto UNWIND_CAIRO;
	    }
	} else {
	    if (cairo_test_files_equal (out_png_path, ref_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches reference image.\n");
		have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_SUCCESS;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, new_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches current failure image.\n");
		have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_NEW;
		goto UNWIND_CAIRO;
	    }
	    if (cairo_test_files_equal (out_png_path, xfail_png_path)) {
		cairo_test_log (ctx, "PNG file exactly matches known failure image.\n");
		have_result = TRUE;
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_XFAILURE;
		goto UNWIND_CAIRO;
	    }
	}

	if (cairo_test_files_equal (out_png_path, base_ref_png_path)) {
	    cairo_test_log (ctx, "PNG file exactly reference image.\n");
	    have_result = TRUE;
	    cairo_surface_destroy (test_image);
	    ret = CAIRO_TEST_SUCCESS;
	    goto UNWIND_CAIRO;
	}
	if (cairo_test_files_equal (out_png_path, base_new_png_path)) {
	    cairo_test_log (ctx, "PNG file exactly current failure image.\n");
	    have_result = TRUE;
	    cairo_surface_destroy (test_image);
	    ret = CAIRO_TEST_NEW;
	    goto UNWIND_CAIRO;
	}
	if (cairo_test_files_equal (out_png_path, base_xfail_png_path)) {
	    cairo_test_log (ctx, "PNG file exactly known failure image.\n");
	    have_result = TRUE;
	    cairo_surface_destroy (test_image);
	    ret = CAIRO_TEST_XFAILURE;
	    goto UNWIND_CAIRO;
	}

	/* first compare against the ideal reference */
	ref_image = cairo_test_get_reference_image (ctx, base_ref_png_path,
						    target->content == CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED);
	if (cairo_surface_status (ref_image)) {
	    cairo_test_log (ctx, "Error: Cannot open reference image for %s: %s\n",
			    base_ref_png_path,
			    cairo_status_to_string (cairo_surface_status (ref_image)));
	    cairo_surface_destroy (test_image);
	    ret = CAIRO_TEST_FAILURE;
	    goto UNWIND_CAIRO;
	}

	diff_image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						 ctx->test->width,
						 ctx->test->height);

	cmp_png_path = base_ref_png_path;
	diff_status = image_diff (ctx,
				  test_image, ref_image, diff_image,
				  &result);
	_xunlink (ctx, diff_png_path);
	if (diff_status ||
            image_diff_is_failure (&result, target->error_tolerance))
	{
	    /* that failed, so check against the specific backend */
	    ref_image = cairo_test_get_reference_image (ctx, ref_png_path,
							target->content == CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED);
	    if (cairo_surface_status (ref_image)) {
		cairo_test_log (ctx, "Error: Cannot open reference image for %s: %s\n",
				ref_png_path,
				cairo_status_to_string (cairo_surface_status (ref_image)));
		cairo_surface_destroy (test_image);
		ret = CAIRO_TEST_FAILURE;
		goto UNWIND_CAIRO;
	    }

	    cmp_png_path = ref_png_path;
	    diff_status = image_diff (ctx,
				      test_image, ref_image,
				      diff_image,
				      &result);
	    if (diff_status)
	    {
		cairo_test_log (ctx, "Error: Failed to compare images: %s\n",
				cairo_status_to_string (diff_status));
		ret = CAIRO_TEST_FAILURE;
	    }
	    else if (image_diff_is_failure (&result, target->error_tolerance))
	    {
		ret = CAIRO_TEST_FAILURE;

		diff_status = cairo_surface_write_to_png (diff_image,
							  diff_png_path);
		if (diff_status) {
		    cairo_test_log (ctx, "Error: Failed to write differences image: %s\n",
				    cairo_status_to_string (diff_status));
		} else {
		    have_result = TRUE;
		}

		cairo_test_copy_file (test_filename, fail_filename);
	    }
	    else
	    { /* success */
		cairo_test_copy_file (test_filename, pass_filename);
	    }
	}
	else
	{ /* success */
	    cairo_test_copy_file (test_filename, pass_filename);
	}

	/* If failed, compare against the current image output,
	 * and attempt to detect systematic failures.
	 */
	if (ret == CAIRO_TEST_FAILURE) {
	    char *image_out_path;

	    image_out_path =
		cairo_test_reference_filename (ctx,
					       base_name,
					       ctx->test_name,
					       "image",
					       "image",
					       format,
					       CAIRO_TEST_OUT_SUFFIX,
					       CAIRO_TEST_PNG_EXTENSION);
	    if (image_out_path != NULL) {
		if (cairo_test_files_equal (out_png_path,
					    image_out_path))
		{
		    ret = CAIRO_TEST_XFAILURE;
		}
		else
		{
		    ref_image =
			cairo_image_surface_create_from_png (image_out_path);
		    if (cairo_surface_status (ref_image) == CAIRO_STATUS_SUCCESS)
		    {
			diff_status = image_diff (ctx,
						  test_image, ref_image,
						  diff_image,
						  &result);
			if (diff_status == CAIRO_STATUS_SUCCESS &&
			    !image_diff_is_failure (&result, target->error_tolerance))
			{
			    ret = CAIRO_TEST_XFAILURE;
			}

			cairo_surface_destroy (ref_image);
		    }
		}

		free (image_out_path);
	    }
	}

	cairo_surface_destroy (test_image);
	cairo_surface_destroy (diff_image);
    }

    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) {
	cairo_test_log (ctx, "Error: Function under test left cairo status in an error state: %s\n",
			cairo_status_to_string (cairo_status (cr)));
	ret = CAIRO_TEST_ERROR;
	goto UNWIND_CAIRO;
    }

UNWIND_CAIRO:
    free (test_filename);
    free (fail_filename);
    free (pass_filename);

    test_filename = fail_filename = pass_filename = NULL;

#if HAVE_MEMFAULT
    if (ret == CAIRO_TEST_FAILURE)
	MEMFAULT_PRINT_FAULTS ();
#endif
    cairo_destroy (cr);
UNWIND_SURFACE:
    cairo_surface_destroy (surface);

    if (target->cleanup)
	target->cleanup (closure);

#if HAVE_MEMFAULT
    cairo_debug_reset_static_data ();

#if HAVE_FCFINI
    FcFini ();
#endif

    if (MEMFAULT_COUNT_LEAKS () > 0) {
	if (ret != CAIRO_TEST_FAILURE)
	    MEMFAULT_PRINT_FAULTS ();
	MEMFAULT_PRINT_LEAKS ();
    }

    if (ret == CAIRO_TEST_SUCCESS && --malloc_failure_iterations > 0)
	goto REPEAT;
#endif

    if (have_output)
	cairo_test_log (ctx, "OUTPUT: %s\n", out_png_path);

    if (have_result) {
	if (cmp_png_path == NULL) {
	    /* XXX presume we matched the normal ref last time */
	    cmp_png_path = ref_png_path;
	}
	cairo_test_log (ctx,
			"REFERENCE: %s\nDIFFERENCE: %s\n",
			cmp_png_path, diff_png_path);
    }

UNWIND_STRINGS:
    free (out_png_path);
    free (ref_png_path);
    free (base_ref_png_path);
    free (ref_path);
    free (new_png_path);
    free (base_new_png_path);
    free (new_path);
    free (xfail_png_path);
    free (base_xfail_png_path);
    free (xfail_path);
    free (diff_png_path);
    free (base_path);
    free (base_name);

    return ret;
}

#if defined(HAVE_SIGNAL_H) && defined(HAVE_SETJMP_H)
#include <signal.h>
#include <setjmp.h>
/* Used to catch crashes in a test, so that we report it as such and
 * continue testing, although one crash may already have corrupted memory in
 * an nonrecoverable fashion. */
static jmp_buf jmpbuf;

static void
segfault_handler (int signal)
{
    longjmp (jmpbuf, signal);
}
#endif

cairo_test_status_t
_cairo_test_context_run_for_target (cairo_test_context_t *ctx,
				    const cairo_boilerplate_target_t *target,
				    cairo_bool_t similar,
				    int dev_offset, int dev_scale)
{
    cairo_test_status_t status;

    if (target->get_image_surface == NULL)
	return CAIRO_TEST_UNTESTED;

    if (similar && ! cairo_test_target_has_similar (ctx, target))
	return CAIRO_TEST_UNTESTED;

    cairo_test_log (ctx,
		    "Testing %s with %s%s target (dev offset %d scale: %d)\n",
		    ctx->test_name,
		    similar ? " (similar) " : "",
		    target->name,
		    dev_offset, dev_scale);

    printf ("%s.%s.%s [%dx%d]%s:\t", ctx->test_name, target->name,
	    cairo_boilerplate_content_name (target->content),
	    dev_offset, dev_scale,
	    similar ? " (similar)": "");
    fflush (stdout);

#if defined(HAVE_SIGNAL_H) && defined(HAVE_SETJMP_H)
    if (! RUNNING_ON_VALGRIND) {
	void (* volatile old_segfault_handler)(int);
	void (* volatile old_segfpe_handler)(int);
#ifdef SIGPIPE
	void (* volatile old_sigpipe_handler)(int);
#endif
	void (* volatile old_sigabrt_handler)(int);
#ifdef SIGALRM
	void (* volatile old_sigalrm_handler)(int);
#endif

	/* Set up a checkpoint to get back to in case of segfaults. */
#ifdef SIGSEGV
	old_segfault_handler = signal (SIGSEGV, segfault_handler);
#endif
#ifdef SIGFPE
	old_segfpe_handler = signal (SIGFPE, segfault_handler);
#endif
#ifdef SIGPIPE
	old_sigpipe_handler = signal (SIGPIPE, segfault_handler);
#endif
#ifdef SIGABRT
	old_sigabrt_handler = signal (SIGABRT, segfault_handler);
#endif
#ifdef SIGALRM
	old_sigalrm_handler = signal (SIGALRM, segfault_handler);
#endif
	if (0 == setjmp (jmpbuf))
	    status = cairo_test_for_target (ctx, target, dev_offset, dev_scale, similar);
	else
	    status = CAIRO_TEST_CRASHED;
#ifdef SIGSEGV
	signal (SIGSEGV, old_segfault_handler);
#endif
#ifdef SIGFPE
	signal (SIGFPE, old_segfpe_handler);
#endif
#ifdef SIGPIPE
	signal (SIGPIPE, old_sigpipe_handler);
#endif
#ifdef SIGABRT
	signal (SIGABRT, old_sigabrt_handler);
#endif
#ifdef SIGALRM
	signal (SIGALRM, old_sigalrm_handler);
#endif
    } else {
	status = cairo_test_for_target (ctx, target, dev_offset, dev_scale, similar);
    }
#else
    status = cairo_test_for_target (ctx, target, dev_offset, dev_scale, similar);
#endif

    cairo_test_log (ctx,
		    "TEST: %s TARGET: %s FORMAT: %s OFFSET: %d SCALE: %d SIMILAR: %d RESULT: ",
		    ctx->test_name, target->name,
		    cairo_boilerplate_content_name (target->content),
		    dev_offset, dev_scale, similar);
    switch (status) {
    case CAIRO_TEST_SUCCESS:
	printf ("PASS\n");
	cairo_test_log (ctx, "PASS\n");
	break;

    case CAIRO_TEST_UNTESTED:
	printf ("UNTESTED\n");
	cairo_test_log (ctx, "UNTESTED\n");
	break;

    default:
    case CAIRO_TEST_CRASHED:
	if (print_fail_on_stdout) {
	    printf ("!!!CRASHED!!!\n");
	} else {
	    /* eat the test name */
	    printf ("\r");
	    fflush (stdout);
	}
	cairo_test_log (ctx, "CRASHED\n");
	fprintf (stderr, "%s.%s.%s [%dx%d]%s:\t%s!!!CRASHED!!!%s\n",
		 ctx->test_name, target->name,
		 cairo_boilerplate_content_name (target->content), dev_offset, dev_scale, similar ? " (similar)" : "",
		 fail_face, normal_face);
	break;

    case CAIRO_TEST_ERROR:
	if (print_fail_on_stdout) {
	    printf ("!!!ERROR!!!\n");
	} else {
	    /* eat the test name */
	    printf ("\r");
	    fflush (stdout);
	}
	cairo_test_log (ctx, "ERROR\n");
	fprintf (stderr, "%s.%s.%s [%dx%d]%s:\t%s!!!ERROR!!!%s\n",
		 ctx->test_name, target->name,
		 cairo_boilerplate_content_name (target->content), dev_offset, dev_scale, similar ? " (similar)" : "",
		 fail_face, normal_face);
	break;

    case CAIRO_TEST_XFAILURE:
	if (print_fail_on_stdout) {
	    printf ("XFAIL\n");
	} else {
	    /* eat the test name */
	    printf ("\r");
	    fflush (stdout);
	}
	fprintf (stderr, "%s.%s.%s [%dx%d]%s:\t%sXFAIL%s\n",
		 ctx->test_name, target->name,
		 cairo_boilerplate_content_name (target->content), dev_offset, dev_scale, similar ? " (similar)" : "",
		 xfail_face, normal_face);
	cairo_test_log (ctx, "XFAIL\n");
	break;

    case CAIRO_TEST_NEW:
	if (print_fail_on_stdout) {
	    printf ("NEW\n");
	} else {
	    /* eat the test name */
	    printf ("\r");
	    fflush (stdout);
	}
	fprintf (stderr, "%s.%s.%s [%dx%d]%s:\t%sNEW%s\n",
		 ctx->test_name, target->name,
		 cairo_boilerplate_content_name (target->content), dev_offset, dev_scale, similar ? " (similar)" : "",
		 fail_face, normal_face);
	cairo_test_log (ctx, "NEW\n");
	break;

    case CAIRO_TEST_NO_MEMORY:
    case CAIRO_TEST_FAILURE:
	if (print_fail_on_stdout) {
	    printf ("FAIL\n");
	} else {
	    /* eat the test name */
	    printf ("\r");
	    fflush (stdout);
	}
	fprintf (stderr, "%s.%s.%s [%dx%d]%s:\t%sFAIL%s\n",
		 ctx->test_name, target->name,
		 cairo_boilerplate_content_name (target->content), dev_offset, dev_scale, similar ? " (similar)" : "",
		 fail_face, normal_face);
	cairo_test_log (ctx, "FAIL\n");
	break;
    }
    fflush (stdout);

    return status;
}

const cairo_test_context_t *
cairo_test_get_context (cairo_t *cr)
{
    return cairo_get_user_data (cr, &_cairo_test_context_key);
}

cairo_t *
cairo_test_create (cairo_surface_t *surface,
		   const cairo_test_context_t *ctx)
{
    cairo_t *cr = cairo_create (surface);
    cairo_set_user_data (cr, &_cairo_test_context_key,
			 (void*) ctx, NULL);
    return cr;
}

cairo_surface_t *
cairo_test_create_surface_from_png (const cairo_test_context_t *ctx,
	                            const char *filename)
{
    cairo_surface_t *image;
    cairo_status_t status;
    char *unique_id;

    image = cairo_image_surface_create_from_png (filename);
    status = cairo_surface_status (image);
    if (status == CAIRO_STATUS_FILE_NOT_FOUND) {
        /* expect not found when running with srcdir != builddir
         * such as when 'make distcheck' is run
         */
	if (ctx->srcdir) {
	    char *srcdir_filename;
	    xasprintf (&srcdir_filename, "%s/%s", ctx->srcdir, filename);
	    cairo_surface_destroy (image);
	    image = cairo_image_surface_create_from_png (srcdir_filename);
	    free (srcdir_filename);
	}
    }
    unique_id = strdup(filename);
    cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_UNIQUE_ID,
				 (unsigned char *)unique_id, strlen(unique_id),
				 free, unique_id);

    return image;
}

cairo_pattern_t *
cairo_test_create_pattern_from_png (const cairo_test_context_t *ctx,
	                            const char *filename)
{
    cairo_surface_t *image;
    cairo_pattern_t *pattern;

    image = cairo_test_create_surface_from_png (ctx, filename);

    pattern = cairo_pattern_create_for_surface (image);

    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

    cairo_surface_destroy (image);

    return pattern;
}

static cairo_surface_t *
_draw_check (int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 12, 12);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 0.75, 0.75, 0.75); /* light gray */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.25, 0.25, 0.25); /* dark gray */
    cairo_rectangle (cr, width / 2,  0, width / 2, height / 2);
    cairo_rectangle (cr, 0, height / 2, width / 2, height / 2);
    cairo_fill (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

void
cairo_test_paint_checkered (cairo_t *cr)
{
    cairo_surface_t *check;

    check = _draw_check (12, 12);

    cairo_save (cr);
    cairo_set_source_surface (cr, check, 0, 0);
    cairo_surface_destroy (check);

    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);
    cairo_paint (cr);

    cairo_restore (cr);
}

cairo_bool_t
cairo_test_is_target_enabled (const cairo_test_context_t *ctx,
			      const char *target)
{
    size_t i;

    for (i = 0; i < ctx->num_targets; i++) {
	const cairo_boilerplate_target_t *t = ctx->targets_to_test[i];
	if (strcmp (t->name, target) == 0) {
	    /* XXX ask the target whether is it possible to run?
	     * e.g. the xlib backend could check whether it is able to connect
	     * to the Display.
	     */
	    return t->get_image_surface != NULL;
	}
    }

    return FALSE;
}

cairo_bool_t
cairo_test_malloc_failure (const cairo_test_context_t *ctx,
			   cairo_status_t status)
{
    if (! ctx->malloc_failure)
	return FALSE;

    if (status != CAIRO_STATUS_NO_MEMORY)
	return FALSE;

#if HAVE_MEMFAULT
    {
	int n_faults;

	/* prevent infinite loops... */
	n_faults = MEMFAULT_COUNT_FAULTS ();
	if (n_faults == ctx->last_fault_count)
	    return FALSE;

	((cairo_test_context_t *) ctx)->last_fault_count = n_faults;
    }
#endif

    return TRUE;
}

cairo_test_status_t
cairo_test_status_from_status (const cairo_test_context_t *ctx,
			       cairo_status_t status)
{
    if (status == CAIRO_STATUS_SUCCESS)
	return CAIRO_TEST_SUCCESS;

    if (cairo_test_malloc_failure (ctx, status))
	return CAIRO_TEST_NO_MEMORY;

    return CAIRO_TEST_FAILURE;
}

#if CAIRO_HAS_FT_FONT

#include "cairo-ft.h"

static void
_free_face (void *face)
{
    FT_Done_Face ((FT_Face) face);
}

static FT_Library ft_library = NULL;

#endif

static const cairo_user_data_key_t ft_font_key;

cairo_test_status_t
cairo_test_ft_select_font_from_file (cairo_t      *cr,
                                     const char   *filename)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
#if CAIRO_HAS_FT_FONT
    FT_Face face;
    cairo_font_face_t *font_face;
    char *srcdir_filename = NULL;

    if (access (filename, F_OK) != 0) {
	if (ctx->srcdir) {
	    xasprintf (&srcdir_filename, "%s/%s", ctx->srcdir, filename);
            filename = srcdir_filename;
	}
    }

    if (access (filename, F_OK) != 0) {
        cairo_test_log (ctx, "Could not find font file: %s\n", filename);
        return CAIRO_TEST_FAILURE;
    }

    if (!ft_library) {
        if (FT_Init_FreeType (&ft_library))
            return CAIRO_TEST_FAILURE;
    }

    if (FT_New_Face (ft_library, filename, 0, &face)) {
        cairo_test_log (ctx, "FT_New_Face failed loading font file: %s\n", filename);
        return CAIRO_TEST_FAILURE;
    }

    free (srcdir_filename);
    font_face = cairo_ft_font_face_create_for_ft_face (face, 0);
    if (cairo_font_face_status (font_face))
        return CAIRO_TEST_FAILURE;

    cairo_font_face_set_user_data (font_face, &ft_font_key, face, _free_face);
    cairo_set_font_face (cr, font_face);
    if (cairo_status (cr))
        return CAIRO_TEST_FAILURE;

    cairo_font_face_destroy (font_face);

    return CAIRO_TEST_SUCCESS;
#else
    cairo_test_log (ctx, "cairo_test_ft_select_font_from_file() requires the FreeType backend\n");
    return CAIRO_TEST_FAILURE;
#endif
}
