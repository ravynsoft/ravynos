/*
 * Copyright © 2004,2006 Red Hat, Inc.
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

#ifndef _CAIRO_BOILERPLATE_H_
#define _CAIRO_BOILERPLATE_H_

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cairo.h>
#include <string.h>

#include "cairo-compiler-private.h"

#if   HAVE_STDINT_H
# include <stdint.h>
#elif HAVE_INTTYPES_H
# include <inttypes.h>
#elif HAVE_SYS_INT_TYPES_H
# include <sys/int_types.h>
#elif defined(_MSC_VER)
# include <stdint.h>
  typedef __int8 int8_t;
  typedef unsigned __int8 uint8_t;
  typedef __int16 int16_t;
  typedef unsigned __int16 uint16_t;
  typedef __int32 int32_t;
  typedef unsigned __int32 uint32_t;
  typedef __int64 int64_t;
  typedef unsigned __int64 uint64_t;
#else
#error Cannot find definitions for fixed-width integral types (uint8_t, uint32_t, etc.)
#endif

#ifndef HAVE_UINT64_T
# define HAVE_UINT64_T 1
#endif
#ifndef INT16_MIN
# define INT16_MIN	(-32767-1)
#endif
#ifndef INT16_MAX
# define INT16_MAX	(32767)
#endif
#ifndef UINT16_MAX
# define UINT16_MAX	(65535)
#endif

#ifndef CAIRO_BOILERPLATE_DEBUG
#define CAIRO_BOILERPLATE_DEBUG(x)
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#ifdef __MINGW32__
#define CAIRO_BOILERPLATE_PRINTF_FORMAT(fmt_index, va_index)            \
	__attribute__((__format__(__MINGW_PRINTF_FORMAT, fmt_index, va_index)))
#else
#define CAIRO_BOILERPLATE_PRINTF_FORMAT(fmt_index, va_index)            \
	__attribute__((__format__(__printf__, fmt_index, va_index)))
#endif
#else
#define CAIRO_BOILERPLATE_PRINTF_FORMAT(fmt_index, va_index)
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(__array) ((int) (sizeof (__array) / sizeof (__array[0])))
#endif

CAIRO_BEGIN_DECLS

/* A fake format we use for the flattened ARGB output of the PS and
 * PDF surfaces. */
#define CAIRO_TEST_CONTENT_COLOR_ALPHA_FLATTENED ((unsigned int) -1)

extern const cairo_user_data_key_t cairo_boilerplate_output_basename_key;

cairo_content_t
cairo_boilerplate_content (cairo_content_t content);

const char *
cairo_boilerplate_content_name (cairo_content_t content);

cairo_format_t
cairo_boilerplate_format_from_content (cairo_content_t content);

typedef enum {
    CAIRO_BOILERPLATE_MODE_TEST,
    CAIRO_BOILERPLATE_MODE_PERF,

    /* This will allow running performance test with threads. The
     * GL backend is very slow on some drivers when run with thread
     * awareness turned on. */
    CAIRO_BOILERPLATE_MODE_PERF_THREADS,
} cairo_boilerplate_mode_t;

typedef cairo_surface_t *
(*cairo_boilerplate_create_surface_t) (const char		 *name,
				       cairo_content_t		  content,
				       double			  width,
				       double			  height,
				       double			  max_width,
				       double			  max_height,
				       cairo_boilerplate_mode_t   mode,
				       void			**closure);

typedef cairo_surface_t *
(*cairo_boilerplate_create_similar_t) (cairo_surface_t		 *other,
				       cairo_content_t		  content,
				       int			  width,
				       int			  height);

typedef void
(*cairo_boilerplate_force_fallbacks_t) (cairo_surface_t *surface,
				       double		 x_pixels_per_inch,
				       double		 y_pixels_per_inch);

typedef cairo_status_t
(*cairo_boilerplate_finish_surface_t) (cairo_surface_t *surface);

typedef cairo_surface_t *
(*cairo_boilerplate_get_image_surface_t) (cairo_surface_t *surface,
					  int page,
					  int width,
					  int height);

typedef cairo_status_t
(*cairo_boilerplate_write_to_png_t) (cairo_surface_t *surface,
				     const char *filename);

typedef void
(*cairo_boilerplate_cleanup_t) (void *closure);

typedef void
(*cairo_boilerplate_wait_t) (void *closure);

typedef char *
(*cairo_boilerplate_describe_t) (void *closure);

typedef struct _cairo_boilerplate_target {
    const char					*name;
    const char					*basename;
    const char					*file_extension;
    const char					*reference_target;
    cairo_surface_type_t			 expected_type;
    cairo_content_t				 content;
    unsigned int				 error_tolerance;
    const char					*probe; /* runtime dl check */
    cairo_boilerplate_create_surface_t		 create_surface;
    cairo_boilerplate_create_similar_t		 create_similar;
    cairo_boilerplate_force_fallbacks_t		 force_fallbacks;
    cairo_boilerplate_finish_surface_t		 finish_surface;
    cairo_boilerplate_get_image_surface_t	 get_image_surface;
    cairo_boilerplate_write_to_png_t		 write_to_png;
    cairo_boilerplate_cleanup_t			 cleanup;
    cairo_boilerplate_wait_t			 synchronize;
    cairo_boilerplate_describe_t                 describe;
    cairo_bool_t				 is_measurable;
    cairo_bool_t				 is_vector;
    cairo_bool_t				 is_recording;
} cairo_boilerplate_target_t;

const cairo_boilerplate_target_t *
cairo_boilerplate_get_image_target (cairo_content_t content);

const cairo_boilerplate_target_t *
cairo_boilerplate_get_target_by_name (const char      *name,
				      cairo_content_t  content);

const cairo_boilerplate_target_t **
cairo_boilerplate_get_targets (int	    *num_targets,
			       cairo_bool_t *limited_targets);

void
cairo_boilerplate_free_targets (const cairo_boilerplate_target_t **targets);

cairo_surface_t *
_cairo_boilerplate_get_image_surface (cairo_surface_t *src,
				      int	       page,
				      int	       width,
				      int	       height);
cairo_surface_t *
cairo_boilerplate_get_image_surface_from_png (const char   *filename,
					      int	    width,
					      int	    height,
					      cairo_bool_t  flatten);

cairo_surface_t *
cairo_boilerplate_surface_create_in_error (cairo_status_t status);

enum {
    CAIRO_BOILERPLATE_OPEN_NO_DAEMON = 0x1,
};

FILE *
cairo_boilerplate_open_any2ppm (const char   *filename,
				int	      page,
				unsigned int  flags,
				int        (**close_cb) (FILE *));

cairo_surface_t *
cairo_boilerplate_image_surface_create_from_ppm_stream (FILE *file);

cairo_surface_t *
cairo_boilerplate_convert_to_image (const char *filename,
				    int 	page);

int
cairo_boilerplate_version (void);

const char*
cairo_boilerplate_version_string (void);

void
cairo_boilerplate_fini (void);

#include "cairo-boilerplate-system.h"

CAIRO_END_DECLS

#endif
