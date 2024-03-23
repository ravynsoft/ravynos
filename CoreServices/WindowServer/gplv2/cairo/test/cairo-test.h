/*
 * Copyright © 2004 Red Hat, Inc.
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
 */

#ifndef _CAIRO_TEST_H_
#define _CAIRO_TEST_H_

#include "cairo-boilerplate.h"

#include <stdarg.h>

CAIRO_BEGIN_DECLS

#if   HAVE_STDINT_H
# include <stdint.h>
#elif HAVE_INTTYPES_H
# include <inttypes.h>
#elif HAVE_SYS_INT_TYPES_H
# include <sys/int_types.h>
#elif defined(_MSC_VER)
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
# ifndef HAVE_UINT64_T
#  define HAVE_UINT64_T 1
# endif
#else
#error Cannot find definitions for fixed-width integral types (uint8_t, uint32_t, \etc.)
#endif

#ifdef _MSC_VER
#define _USE_MATH_DEFINES

#include <float.h>
#if _MSC_VER <= 1600
#define isnan(x) _isnan(x)
#endif

#endif

#if HAVE_FENV_H
# include <fenv.h>
#endif
/* The following are optional in C99, so define them if they aren't yet */
#ifndef FE_DIVBYZERO
#define FE_DIVBYZERO 0
#endif
#ifndef FE_INEXACT
#define FE_INEXACT 0
#endif
#ifndef FE_INVALID
#define FE_INVALID 0
#endif
#ifndef FE_OVERFLOW
#define FE_OVERFLOW 0
#endif
#ifndef FE_UNDERFLOW
#define FE_UNDERFLOW 0
#endif

#include <math.h>

static inline double
cairo_test_NaN (void)
{
#ifdef _MSC_VER
    /* MSVC strtod("NaN", NULL) returns 0.0 */
    union {
	uint32_t i[2];
	double d;
    } nan = {{0xffffffff, 0x7fffffff}};
    return nan.d;
#else
    return strtod("NaN", NULL);
#endif
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define CAIRO_TEST_OUTPUT_DIR "output"

#define CAIRO_TEST_LOG_SUFFIX ".log"

#define CAIRO_TEST_FONT_FAMILY "DejaVu"

/* What is a fail and what isn't?
 * When running the test suite we want to detect unexpected output. This
 * can be caused by a change we have made to cairo itself, or a change
 * in our environment. To capture this we classify the expected output into 3
 * classes:
 *
 *   REF  -- Perfect output.
 *           Might be different for each backend, due to slight implementation
 *           differences.
 *
 *   NEW  -- A new failure. We have uncovered a bug within cairo and have
 *           recorded the current failure (along with the expected output
 *           if possible!) so we can detect any changes in our attempt to
 *           fix the bug.
 *
 *  XFAIL -- An external failure. We believe the cairo output is perfect,
 *           but an external renderer is causing gross failure.
 *           (We also use this to capture current WONTFIX issues within cairo,
 *           such as overflow in internal coordinates, so as not to distract
 *           us when regression testing.)
 *
 *  If no REF is given for a test, then it is assumed to be XFAIL.
 */
#define CAIRO_TEST_REF_SUFFIX ".ref"
#define CAIRO_TEST_XFAIL_SUFFIX ".xfail"
#define CAIRO_TEST_NEW_SUFFIX ".new"

#define CAIRO_TEST_OUT_SUFFIX ".out"
#define CAIRO_TEST_DIFF_SUFFIX ".diff"

#define CAIRO_TEST_PNG_EXTENSION ".png"
#define CAIRO_TEST_OUT_PNG CAIRO_TEST_OUT_SUFFIX CAIRO_TEST_PNG_EXTENSION
#define CAIRO_TEST_REF_PNG CAIRO_TEST_REF_SUFFIX CAIRO_TEST_PNG_EXTENSION
#define CAIRO_TEST_DIFF_PNG CAIRO_TEST_DIFF_SUFFIX CAIRO_TEST_PNG_EXTENSION

typedef enum cairo_test_status {
    CAIRO_TEST_SUCCESS = 0,
    CAIRO_TEST_NO_MEMORY,
    CAIRO_TEST_FAILURE,
    CAIRO_TEST_NEW,
    CAIRO_TEST_XFAILURE,
    CAIRO_TEST_ERROR,
    CAIRO_TEST_CRASHED,
    CAIRO_TEST_UNTESTED = 77 /* match automake's skipped exit status */
} cairo_test_status_t;

typedef struct _cairo_test_context cairo_test_context_t;
typedef struct _cairo_test cairo_test_t;

typedef cairo_test_status_t
(cairo_test_preamble_function_t) (cairo_test_context_t *ctx);

typedef cairo_test_status_t
(cairo_test_draw_function_t) (cairo_t *cr, int width, int height);

struct _cairo_test {
    const char *name;
    const char *description;
    const char *keywords;
    const char *requirements;
    double width;
    double height;
    cairo_test_preamble_function_t *preamble;
    cairo_test_draw_function_t *draw;
};

/* The standard test interface which works by examining result image.
 *
 * CAIRO_TEST() constructs a test which will be called once before (the
 * preamble callback), and then once for each testable backend (the draw
 * callback). The following checks will be performed for each backend:
 *
 * 1) If preamble() returns CAIRO_TEST_UNTESTED, the test is skipped.
 *
 * 2) If preamble() does not return CAIRO_TEST_SUCCESS, the test fails.
 *
 * 3) If draw() does not return CAIRO_TEST_SUCCESS then this backend
 *    fails.
 *
 * 4) Otherwise, if cairo_status(cr) indicates an error then this
 *    backend fails.
 *
 * 5) Otherwise, if the image size is 0, then this backend passes.
 *
 * 6) Otherwise, if every channel of every pixel exactly matches the
 *    reference image then this backend passes. If not, this backend
 *    fails.
 *
 * The overall test result is PASS if and only if there is at least
 * one backend that is tested and if all tested backend pass according
 * to the four criteria above.
 */
#define CAIRO_TEST(name, description, keywords, requirements, width, height, preamble, draw) \
void _register_##name (void); \
void _register_##name (void) { \
    static cairo_test_t test = { \
	#name, description, \
	keywords, requirements, \
	width, height, \
	preamble, draw \
    }; \
    cairo_test_register (&test); \
}

void
cairo_test_register (cairo_test_t *test);

/* The full context for the test.
 * For ordinary tests (using the CAIRO_TEST()->draw interface) the context
 * is passed to the draw routine via user_data on the cairo_t.
 * The reason why the context is not passed as an explicit parameter is that
 * it is rarely required by the test itself and by removing the parameter
 * we can keep the draw routines simple and serve as example code.
 *
 * In contrast, for the preamble phase the context is passed as the only
 * parameter.
 */
struct _cairo_test_context {
    const cairo_test_t *test;
    const char *test_name;

    FILE *log_file;
    const char *output;
    const char *srcdir; /* directory containing sources and input data */
    char *refdir; /* directory containing reference images */

    char *ref_name; /* cache of the current reference image */
    cairo_surface_t *ref_image;
    cairo_surface_t *ref_image_flattened;

    size_t num_targets;
    cairo_bool_t limited_targets;
    const cairo_boilerplate_target_t **targets_to_test;
    cairo_bool_t own_targets;

    int malloc_failure;
    int last_fault_count;

    int timeout;
};

/* Retrieve the test context from the cairo_t, used for logging, paths etc */
const cairo_test_context_t *
cairo_test_get_context (cairo_t *cr);


/* Print a message to the log file, ala printf. */
void
cairo_test_log (const cairo_test_context_t *ctx,
	        const char *fmt, ...) CAIRO_BOILERPLATE_PRINTF_FORMAT(2, 3);
void
cairo_test_logv (const cairo_test_context_t *ctx,
	        const char *fmt, va_list ap) CAIRO_BOILERPLATE_PRINTF_FORMAT(2, 0);

/* Helper functions that take care of finding source images even when
 * building in a non-srcdir manner, (i.e. the tests will be run in a
 * directory that is different from the one where the source image
 * exists). */
cairo_surface_t *
cairo_test_create_surface_from_png (const cairo_test_context_t *ctx,
	                            const char *filename);

cairo_pattern_t *
cairo_test_create_pattern_from_png (const cairo_test_context_t *ctx,
	                            const char *filename);

void
cairo_test_paint_checkered (cairo_t *cr);

#define CAIRO_TEST_DOUBLE_EQUALS(a,b)  (fabs((a)-(b)) < 0.00001)

cairo_bool_t
cairo_test_is_target_enabled (const cairo_test_context_t *ctx,
	                      const char *target);

char *
cairo_test_get_name (const cairo_test_t *test);

cairo_bool_t
cairo_test_malloc_failure (const cairo_test_context_t *ctx,
	                   cairo_status_t status);

cairo_test_status_t
cairo_test_status_from_status (const cairo_test_context_t *ctx,
			       cairo_status_t status);

char *
cairo_test_reference_filename (const cairo_test_context_t *ctx,
			       const char *base_name,
			       const char *test_name,
			       const char *target_name,
			       const char *base_target_name,
			       const char *format,
			       const char *suffix,
			       const char *extension);

cairo_surface_t *
cairo_test_get_reference_image (cairo_test_context_t *ctx,
				const char *filename,
				cairo_bool_t flatten);

cairo_bool_t
cairo_test_mkdir (const char *path);

cairo_t *
cairo_test_create (cairo_surface_t *surface,
		   const cairo_test_context_t *ctx);

/* Set font face from a font file in build or src dir, using the FT backend. */
cairo_test_status_t
cairo_test_ft_select_font_from_file (cairo_t      *cr,
                                     const char   *filename);

CAIRO_END_DECLS

#endif
