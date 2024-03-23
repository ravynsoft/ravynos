/* cairo-trace - a utility to record and replay calls to the Cairo library.
 *
 * Copyright © 2008 Chris Wilson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

/* The autoconf on OpenBSD 4.5 produces the malformed constant name
 * SIZEOF_VOID__ rather than SIZEOF_VOID_P.  Work around that here. */
#if !defined(SIZEOF_VOID_P) && defined(SIZEOF_VOID__)
# define SIZEOF_VOID_P SIZEOF_VOID__
#endif

#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <zlib.h>
#include <math.h>
#include <locale.h> /* for locale independent %f printing */
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#include <cairo.h>
#if CAIRO_HAS_FT_FONT
# include <cairo-ft.h>
#endif

#include "cairo-ctype-inline.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef CAIRO_TRACE_OUTDIR
#define CAIRO_TRACE_OUTDIR "."
#endif

#define DEBUG_STACK 0

#if HAVE_BYTESWAP_H
# include <byteswap.h>
#endif
#ifndef bswap_16
# define bswap_16(p) \
	(((((uint16_t)(p)) & 0x00ff) << 8) | \
	  (((uint16_t)(p))           >> 8))
#endif
#ifndef bswap_32
# define bswap_32(p) \
         (((((uint32_t)(p)) & 0x000000ff) << 24) | \
	  ((((uint32_t)(p)) & 0x0000ff00) << 8)  | \
	  ((((uint32_t)(p)) & 0x00ff0000) >> 8)  | \
	  ((((uint32_t)(p)))              >> 24))
#endif

#if WORDS_BIGENDIAN
#define le16(x) bswap_16 (x)
#define le32(x) bswap_32 (x)
#define be16(x) x
#define be32(x) x
#define to_be32(x) x
#else
#define le16(x) x
#define le32(x) x
#define be16(x) bswap_16 (x)
#define be32(x) bswap_32 (x)
#define to_be32(x) bswap_32 (x)
#endif

#if CAIRO_HAS_SYMBOL_LOOKUP
#include "lookup-symbol.h"
#endif

/* Reverse the bits in a byte with 7 operations (no 64-bit):
 * Devised by Sean Anderson, July 13, 2001.
 * Source: http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
 */
#define CAIRO_BITSWAP8(c) ((((c) * 0x0802LU & 0x22110LU) | ((c) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#ifdef __MINGW32__
#define CAIRO_PRINTF_FORMAT(fmt_index, va_index)                        \
	__attribute__((__format__(__MINGW_PRINTF_FORMAT, fmt_index, va_index)))
#else
#define CAIRO_PRINTF_FORMAT(fmt_index, va_index)                        \
	__attribute__((__format__(__printf__, fmt_index, va_index)))
#endif
#else
#define CAIRO_PRINTF_FORMAT(fmt_index, va_index)
#endif

/* XXX implement manual vprintf so that the user can control precision of
 * printed numbers.
 */

static void *_dlhandle = RTLD_NEXT;
#define DLCALL(name, args...) ({ \
    static typeof (&name) name##_real; \
    if (name##_real == NULL) { \
	name##_real = (typeof (&name))(dlsym (_dlhandle, #name));	\
	if (name##_real == NULL && _dlhandle == RTLD_NEXT) { \
	    _dlhandle = dlopen ("libcairo." SHARED_LIB_EXT, RTLD_LAZY); \
	    name##_real = (typeof (&name))(dlsym (_dlhandle, #name));	\
	    assert (name##_real != NULL); \
	} \
    } \
    (*name##_real) (args);  \
})

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(__array) ((int) (sizeof (__array) / sizeof (__array[0])))
#endif

#if SIZEOF_VOID_P == 4
#define PTR_SHIFT 2
#elif SIZEOF_VOID_P == 8
#define PTR_SHIFT 3
#else
#error Unexpected pointer size
#endif
#define BUCKET(b, ptr) (((uintptr_t) (ptr) >> PTR_SHIFT) % ARRAY_LENGTH (b))

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _BOOLEAN_EXPR(expr)                   \
 __extension__ ({                             \
   int _boolean_var_;                         \
   if (expr)                                  \
      _boolean_var_ = 1;                      \
   else                                       \
      _boolean_var_ = 0;                      \
   _boolean_var_;                             \
})
#define LIKELY(expr) (__builtin_expect (_BOOLEAN_EXPR(expr), 1))
#define UNLIKELY(expr) (__builtin_expect (_BOOLEAN_EXPR(expr), 0))
#else
#define LIKELY(expr) (expr)
#define UNLIKELY(expr) (expr)
#endif

typedef struct _object Object;
typedef struct _type Type;

struct _object {
    const void *addr;
    Type *type;
    unsigned long int token;
    int width, height;
    cairo_bool_t foreign;
    cairo_bool_t defined;
    cairo_bool_t unknown;
    int operand;
    void *data;
    void (*destroy)(void *);
    Object *next, *prev;
};

struct _type {
    const char *name;

    enum operand_type {
	NONE,
	SURFACE,
	CONTEXT,
	FONT_FACE,
	PATTERN,
	SCALED_FONT,
	_N_OP_TYPES
    } op_type;
    const char *op_code;

    pthread_mutex_t mutex;
    struct _bitmap {
	unsigned long int min;
	unsigned long int count;
	unsigned int map[64];
	struct _bitmap *next;
    } map;
    Object *objects[607];
    Type *next;
};

static struct _type_table {
    pthread_mutex_t mutex;
    Type *op_types[_N_OP_TYPES];
} Types;

static FILE *logfile;
static cairo_bool_t _flush;
static cairo_bool_t _error;
static cairo_bool_t _line_info;
static cairo_bool_t _mark_dirty;
static const cairo_user_data_key_t destroy_key;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static pthread_key_t counter_key;

static void _init_trace (void);

#define INIT_TRACE_ONCE() pthread_once (&once_control, _init_trace)

#if __GNUC__ >= 3 && defined(__ELF__) && !defined(__sun)
# define _enter_trace() INIT_TRACE_ONCE ()
# define _exit_trace()  do { } while (0)
# define _should_trace() 1
# define USE_ENTER_EXIT 0
#else
static void _enter_trace (void);
static void _exit_trace (void);
static cairo_bool_t _should_trace (void);
# define USE_ENTER_EXIT 1
#endif

#if HAVE_BUILTIN_RETURN_ADDRESS && CAIRO_HAS_SYMBOL_LOOKUP
#define _emit_line_info() do { \
    if (_line_info && _write_lock ()) {	\
	void *addr = __builtin_return_address(0); \
	char caller[1024]; \
	_trace_printf ("%% %s() called by %s\n", __FUNCTION__, \
		       lookup_symbol (caller, sizeof (caller), addr)); \
	_write_unlock (); \
    } \
} while (0)
#else
#define _emit_line_info()
#endif

static void
_type_release_token (Type *t, unsigned long int token)
{
    struct _bitmap *b, **prev = NULL;

    b = &t->map;
    while (b != NULL) {
	if (token < b->min + sizeof (b->map) * CHAR_BIT) {
	    unsigned int bit, elem;

	    token -= b->min;
	    elem = token / (sizeof (b->map[0]) * CHAR_BIT);
	    bit  = token % (sizeof (b->map[0]) * CHAR_BIT);
	    b->map[elem] &= ~(1 << bit);
	    if (! --b->count && prev) {
		*prev = b->next;
		free (b);
	    }
	    return;
	}
	prev = &b->next;
	b = b->next;
    }
}

static unsigned long int
_type_next_token (Type *t)
{
    struct _bitmap *b, *bb, **prev = NULL;
    unsigned long int min = 0;

    b = &t->map;
    while (b != NULL) {
	if (b->min != min)
	    break;

	if (b->count < sizeof (b->map) * CHAR_BIT) {
	    unsigned int n, m, bit;
	    for (n = 0; n < ARRAY_LENGTH (b->map); n++) {
		if (b->map[n] == (unsigned int) -1)
		    continue;

		for (m=0, bit=1; m<sizeof (b->map[0])*CHAR_BIT; m++, bit<<=1) {
		    if ((b->map[n] & bit) == 0) {
			b->map[n] |= bit;
			b->count++;
			return n * sizeof (b->map[0])*CHAR_BIT + m + b->min;
		    }
		}
	    }
	}
	min += sizeof (b->map) * CHAR_BIT;

	prev = &b->next;
	b = b->next;
    }
    assert (prev != NULL);

    bb = malloc (sizeof (struct _bitmap));

    *prev = bb;
    bb->next = b;
    bb->min = min;
    bb->count = 1;
    bb->map[0] = 0x1;
    memset (bb->map + 1, 0, sizeof (bb->map) - sizeof (bb->map[0]));

    return min;
}

static void
_object_destroy (Object *obj)
{
    int bucket;

    pthread_mutex_lock (&obj->type->mutex);
    bucket = BUCKET (obj->type->objects, obj->addr);
    _type_release_token (obj->type, obj->token);

    if (obj->prev != NULL)
	obj->prev->next = obj->next;
    else
	obj->type->objects[bucket] = obj->next;

    if (obj->next != NULL)
	obj->next->prev = obj->prev;
    pthread_mutex_unlock (&obj->type->mutex);

    if (obj->data != NULL && obj->destroy != NULL)
	obj->destroy (obj->data);

    free (obj);
}

static void
_type_create (const char *typename,
	      enum operand_type op_type,
	      const char *op_code)
{
    Type *t;

    pthread_mutex_lock (&Types.mutex);

    t = malloc (sizeof (Type));
    t->name = typename;
    t->op_type = op_type;
    t->op_code = op_code;

    pthread_mutex_init (&t->mutex, NULL);

    t->map.min = 0;
    t->map.count = 0;
    memset (t->map.map, 0, sizeof (t->map.map));
    t->map.next = NULL;

    memset (t->objects, 0, sizeof (t->objects));

    t->next = NULL;

    Types.op_types[op_type] = t;
    pthread_mutex_unlock (&Types.mutex);
}

static Type *
_get_type (enum operand_type type)
{
    return Types.op_types[type];
}

static void
_type_destroy (Type *t)
{
    int n;
    struct _bitmap *b;

    for (n = 0; n < ARRAY_LENGTH (t->objects); n++) {
	Object *obj = t->objects[n];
	while (obj != NULL) {
	    Object *next = obj->next;
	    _object_destroy (obj);
	    obj = next;
	}
    }

    b = t->map.next;
    while (b != NULL) {
	struct _bitmap *next = b->next;
	free (b);
	b = next;
    }

    pthread_mutex_destroy (&t->mutex);
    free (t);
}

static Object *
_type_get_object (Type *type, const void *ptr)
{
    Object *obj;
    int bucket = BUCKET (type->objects, ptr);

    for (obj = type->objects[bucket]; obj != NULL; obj = obj->next) {
	if (obj->addr == ptr) {
	    if (obj->prev != NULL) { /* mru */
		obj->prev->next = obj->next;
		if (obj->next != NULL)
		    obj->next->prev = obj->prev;
		obj->prev = NULL;
		type->objects[bucket]->prev = obj;
		obj->next = type->objects[bucket];
		type->objects[bucket] = obj;
	    }
	    return obj;
	}
    }

    return NULL;
}

static Object *
_object_create (Type *type, const void *ptr)
{
    Object *obj;
    int bucket = BUCKET (type->objects, ptr);

    obj = malloc (sizeof (Object));
    obj->unknown = TRUE;
    obj->defined = FALSE;
    obj->foreign = FALSE;
    obj->operand = -1;
    obj->type = type;
    obj->addr = ptr;
    obj->token = _type_next_token (type);
    obj->data = NULL;
    obj->destroy = NULL;
    obj->prev = NULL;
    obj->next = type->objects[bucket];
    if (type->objects[bucket] != NULL)
	type->objects[bucket]->prev = obj;
    type->objects[bucket] = obj;

    return obj;
}

#if USE_ENTER_EXIT
static int *
_get_counter (void)
{
    int *counter = pthread_getspecific (counter_key);
    if (counter == NULL) {
	counter = calloc(1, sizeof(int));
	pthread_setspecific (counter_key, counter);
    }
    return counter;
}

static void
_enter_trace (void)
{
    INIT_TRACE_ONCE ();
    _get_counter ()[0]++;
}

static void
_exit_trace (void)
{
    _get_counter ()[0]--;
}

static cairo_bool_t
_should_trace (void)
{
    return _get_counter ()[0] <= 1;
}
#endif /* USE_ENTER_EXIT */

static void
_init_trace (void)
{
    pthread_mutex_init (&Types.mutex, NULL);
    pthread_key_create (&counter_key, free);

    _type_create ("unclassed", NONE, "");
    _type_create ("cairo_t", CONTEXT, "c");
    _type_create ("cairo_font_face_t", FONT_FACE, "f");
    _type_create ("cairo_pattern_t", PATTERN, "p");
    _type_create ("cairo_scaled_font_t", SCALED_FONT, "sf");
    _type_create ("cairo_surface_t", SURFACE, "s");
}

static void
_close_trace (void)
{
    if (logfile != NULL) {
	fclose (logfile);
	logfile = NULL;
    }
}

static void __attribute__ ((destructor))
_fini_trace (void)
{
    int n;

    _close_trace ();

    for (n = 0; n < ARRAY_LENGTH (Types.op_types); n++) {
	if (Types.op_types[n]) {
	    _type_destroy (Types.op_types[n]);
	    Types.op_types[n] = NULL;
	}
    }

    pthread_mutex_destroy (&Types.mutex);
}

/* Format a double in a locale independent way and trim trailing
 * zeros.  Based on code from Alex Larson <alexl@redhat.com>.
 * https://mail.gnome.org/archives/gtk-devel-list/2001-October/msg00087.html
 *
 * The code in the patch is copyright Red Hat, Inc under the LGPL.
 */
#define SIGNIFICANT_DIGITS_AFTER_DECIMAL 6
static void
_trace_dtostr (char *buffer, size_t size, double d)
{
    struct lconv *locale_data;
    const char *decimal_point;
    int decimal_point_len;
    char *p;
    int decimal_len;
    int num_zeros, decimal_digits;

    /* Omit the minus sign from negative zero. */
    if (d == 0.0)
	d = 0.0;

    locale_data = localeconv ();
    decimal_point = locale_data->decimal_point;
    decimal_point_len = strlen (decimal_point);

    /* Using "%f" to print numbers less than 0.1 will result in
     * reduced precision due to the default 6 digits after the
     * decimal point.
     *
     * For numbers is < 0.1, we print with maximum precision and count
     * the number of zeros between the decimal point and the first
     * significant digit. We then print the number again with the
     * number of decimal places that gives us the required number of
     * significant digits. This ensures the number is correctly
     * rounded.
     */
    if (fabs (d) >= 0.1) {
	snprintf (buffer, size, "%f", d);
    } else {
	snprintf (buffer, size, "%.18f", d);
	p = buffer;

	if (*p == '+' || *p == '-')
	    p++;

	while (_cairo_isdigit (*p))
	    p++;

	if (strncmp (p, decimal_point, decimal_point_len) == 0)
	    p += decimal_point_len;

	num_zeros = 0;
	while (*p++ == '0')
	    num_zeros++;

	decimal_digits = num_zeros + SIGNIFICANT_DIGITS_AFTER_DECIMAL;

	if (decimal_digits < 18)
	    snprintf (buffer, size, "%.*f", decimal_digits, d);
    }
    p = buffer;

    if (*p == '+' || *p == '-')
	p++;

    while (_cairo_isdigit (*p))
	p++;

    if (strncmp (p, decimal_point, decimal_point_len) == 0) {
	*p = '.';
	decimal_len = strlen (p + decimal_point_len);
	memmove (p + 1, p + decimal_point_len, decimal_len);
	p[1 + decimal_len] = 0;

	/* Remove trailing zeros and decimal point if possible. */
	for (p = p + decimal_len; *p == '0'; p--)
	    *p = 0;

	if (*p == '.') {
	    *p = 0;
	    p--;
	}
    }
}

enum {
    LENGTH_MODIFIER_LONG = 0x100
};

/* Here's a limited reimplementation of printf.  The reason for doing
 * this is primarily to special case handling of doubles.  We want
 * locale independent formatting of doubles and we want to trim
 * trailing zeros.  This is handled by dtostr() above, and the code
 * below handles everything else by calling snprintf() to do the
 * formatting.  This functionality is only for internal use and we
 * only implement the formats we actually use.
 */
static void CAIRO_PRINTF_FORMAT(1, 0)
_trace_vprintf (const char *fmt, va_list ap)
{
#define SINGLE_FMT_BUFFER_SIZE 32
    char buffer[512], single_fmt[SINGLE_FMT_BUFFER_SIZE];
    int single_fmt_length;
    char *p;
    const char *f, *start;
    int length_modifier, width;
    cairo_bool_t var_width;
    int ret_ignored;

    assert (_should_trace ());

    f = fmt;
    p = buffer;
    while (*f != '\0') {
	if (*f != '%') {
	    *p++ = *f++;
	    continue;
	}

	start = f;
	f++;

	if (*f == '0')
	    f++;

        var_width = 0;
        if (*f == '*') {
            var_width = 1;
	    f++;
        }

	while (_cairo_isdigit (*f))
	    f++;

	length_modifier = 0;
	if (*f == 'l') {
	    length_modifier = LENGTH_MODIFIER_LONG;
	    f++;
	}

	/* The only format strings exist in the cairo implementation
	 * itself. So there's an internal consistency problem if any
	 * of them is larger than our format buffer size. */
	single_fmt_length = f - start + 1;

	/* Reuse the format string for this conversion. */
	memcpy (single_fmt, start, single_fmt_length);
	single_fmt[single_fmt_length] = '\0';

	/* Flush contents of buffer before snprintf()'ing into it. */
	ret_ignored = fwrite (buffer, 1, p-buffer, logfile);

	/* We group signed and unsigned together in this switch, the
	 * only thing that matters here is the size of the arguments,
	 * since we're just passing the data through to sprintf(). */
	switch (*f | length_modifier) {
	case '%':
	    buffer[0] = *f;
	    buffer[1] = 0;
	    break;
	case 'd':
	case 'u':
	case 'o':
	case 'x':
	case 'X':
            if (var_width) {
                width = va_arg (ap, int);
                snprintf (buffer, sizeof buffer,
                          single_fmt, width, va_arg (ap, int));
            } else {
                snprintf (buffer, sizeof buffer, single_fmt, va_arg (ap, int));
            }
	    break;
	case 'd' | LENGTH_MODIFIER_LONG:
	case 'u' | LENGTH_MODIFIER_LONG:
	case 'o' | LENGTH_MODIFIER_LONG:
	case 'x' | LENGTH_MODIFIER_LONG:
	case 'X' | LENGTH_MODIFIER_LONG:
            if (var_width) {
                width = va_arg (ap, int);
                snprintf (buffer, sizeof buffer,
                          single_fmt, width, va_arg (ap, long int));
            } else {
                snprintf (buffer, sizeof buffer,
                          single_fmt, va_arg (ap, long int));
            }
	    break;
	case 's':
	    snprintf (buffer, sizeof buffer,
		      single_fmt, va_arg (ap, const char *));
	    break;
	case 'f':
	case 'g':
	    _trace_dtostr (buffer, sizeof buffer, va_arg (ap, double));
	    break;
	case 'c':
	    buffer[0] = va_arg (ap, int);
	    buffer[1] = 0;
	    break;
	default:
	    break;
	}
	p = buffer + strlen (buffer);
	f++;
    }

    ret_ignored = fwrite (buffer, 1, p-buffer, logfile);
    (void)ret_ignored;
}

static void CAIRO_PRINTF_FORMAT(1, 2)
_trace_printf (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    _trace_vprintf (fmt, ap);
    va_end (ap);
}

static void
get_prog_name (char *buf, int length)
{
    char *slash;
    FILE *file;

    memset (buf, 0, length);
    if (length == 0)
	return;

    file = fopen ("/proc/self/cmdline", "rb");
    if (file != NULL) {
	slash = fgets (buf, length, file);
	fclose (file);

	if (slash == NULL)
	    return;
    } else {
	char const *name = getenv ("CAIRO_TRACE_PROG_NAME");
	if (name != NULL) {
	    strncpy (buf, name, length-1);
	}
    }

    slash = strrchr (buf, '/');
    if (slash != NULL) {
	size_t len = strlen (slash+1);
	memmove (buf, slash+1, len+1);
    }
}

static void
_emit_header (void)
{
    char name[4096] = "";

    get_prog_name (name, sizeof (name));

    _trace_printf ("%%!CairoScript - %s\n", name);
}

static cairo_bool_t
_init_logfile (void)
{
    static cairo_bool_t initialized;
    char buf[4096];
    const char *filename;
    const char *env;

    if (initialized)
	return logfile != NULL;

    initialized = TRUE;

    env = getenv ("CAIRO_TRACE_FLUSH");
    if (env != NULL)
	_flush = atoi (env);

    _line_info = TRUE;
    env = getenv ("CAIRO_TRACE_LINE_INFO");
    if (env != NULL)
	_line_info = atoi (env);

    _mark_dirty = TRUE;
    env = getenv ("CAIRO_TRACE_MARK_DIRTY");
    if (env != NULL)
	_mark_dirty = atoi (env);

    filename = getenv ("CAIRO_TRACE_FD");
    if (filename != NULL) {
	int fd = atoi (filename);
	if (fd == -1)
	    return FALSE;

	logfile = fdopen (fd, "w");
	if (logfile == NULL) {
	    fprintf (stderr, "Failed to open trace file descriptor '%s': %s\n",
		       filename, strerror (errno));
	    return FALSE;
	}

	setenv ("CAIRO_TRACE_FD", "-1", 1);
	goto done;
    }

    filename = getenv ("CAIRO_TRACE_OUTFILE_EXACT");
    if (filename == NULL) {
	char name[4096] = "";

	filename = getenv ("CAIRO_TRACE_OUTDIR");
	if (filename == NULL)
	    filename = CAIRO_TRACE_OUTDIR;

	get_prog_name (name, sizeof (name));
	if (*name == '\0')
	    strcpy (name, "cairo-trace.dat");

	if (snprintf (buf, sizeof (buf), "%s/%s.%d.trace",
		      filename, name, getpid()) >= (int) sizeof (buf))
	{
	    fprintf (stderr, "cairo-trace: Trace file name too long\n");
	    return FALSE;
	}

	filename = buf;
    } else {
	setenv ("CAIRO_TRACE_FD", "-1", 1);
    }

    logfile = fopen (filename, "wb");
    if (logfile == NULL) {
	fprintf (stderr, "Failed to open trace file '%s': %s\n",
		   filename, strerror (errno));
	return FALSE;
    }

    fprintf (stderr, "cairo-trace: Recording cairo trace data to %s\n",
	     filename);

done:
    atexit (_close_trace);
    _emit_header ();
    return TRUE;
}

static cairo_bool_t
_write_lock (void)
{
    if (_error)
	return FALSE;

    if (! _should_trace ())
	return FALSE;

    if (! _init_logfile ())
	return FALSE;

#if HAVE_FLOCKFILE && HAVE_FUNLOCKFILE
    flockfile (logfile);
#endif
    return TRUE;
}

static void
_write_unlock (void)
{
    if (logfile == NULL)
	return;

#if HAVE_FLOCKFILE && HAVE_FUNLOCKFILE
    funlockfile (logfile);
#endif

    if (_flush)
	fflush (logfile);
}


static Object *
_type_object_create (enum operand_type op_type, const void *ptr)
{
    Type *type;
    Object *obj;

    type = _get_type (op_type);

    pthread_mutex_lock (&type->mutex);
    obj = _object_create (type, ptr);
    pthread_mutex_unlock (&type->mutex);

    return obj;
}

static Object *
_get_object (enum operand_type op_type, const void *ptr)
{
    Type *type;
    Object *obj;

    type = _get_type (op_type);
    pthread_mutex_lock (&type->mutex);
    obj = _type_get_object (type, ptr);
    pthread_mutex_unlock (&type->mutex);

    return obj;
}

static Object *current_object[2048]; /* XXX limit operand stack */
static int current_stack_depth;

static void
dump_stack(const char *func)
{
#if DEBUG_STACK
	int n;

	_trace_printf ("%% %s: stack[%d] = [", func, current_stack_depth);
	fflush (logfile);
	for (n = 0; n < current_stack_depth; n++) {
		Object *obj = current_object[n];
		assert(obj && obj->type);
		_trace_printf (" %s%s%ld",
			       obj->defined ? "" : "*",
			       obj->type->op_code, obj->token);
	fflush (logfile);
	}
	_trace_printf (" ]\n");
	fflush (logfile);
#endif
}

static void
ensure_operands (int num_operands)
{
    if (current_stack_depth < num_operands) {
	int n;

	fprintf (stderr, "Operand stack underflow!\n");
	for (n = 0; n < current_stack_depth; n++) {
	    Object *obj = current_object[n];

	    fprintf (stderr, "  [%3d] = %s%ld\n",
		     n, obj->type->op_code, obj->token);
	}

	abort ();
    }
}

static void
_consume_operand (bool discard)
{
    Object *obj;

    ensure_operands (1);
    obj = current_object[--current_stack_depth];
    if (!discard && ! obj->defined) {
	_trace_printf ("dup /%s%ld exch def\n",
		       obj->type->op_code,
		       obj->token);
	obj->defined = TRUE;
    }
    obj->operand = -1;
}

static void
_exch_operands (void)
{
    Object *tmp;

    ensure_operands (2);
    tmp = current_object[current_stack_depth-1];
    tmp->operand--;
    current_object[current_stack_depth-1] = current_object[current_stack_depth-2];
    current_object[current_stack_depth-2] = tmp;
    tmp = current_object[current_stack_depth-1];
    tmp->operand++;
}

static cairo_bool_t
_pop_operands_to_depth (int depth)
{
    if (depth < 0)
	return FALSE;

    assert(current_stack_depth >= depth);
    if (current_stack_depth == depth)
	return TRUE;

    while (current_stack_depth > depth + 1) {
	Object *c_obj;

	ensure_operands (1);
	c_obj = current_object[--current_stack_depth];

	assert(c_obj);
	assert(c_obj->type);

	if (! c_obj->defined) {
	    current_stack_depth++;
	    return FALSE;
	}

	_trace_printf ("pop %% %s%ld\n",
		       c_obj->type->op_code, c_obj->token);
	c_obj->operand = -1;
    }

    _exch_operands ();
    _trace_printf ("exch\n");

    dump_stack(__func__);
    return TRUE;
}

static cairo_bool_t
_pop_operands_to_object (Object *obj)
{
    if (!obj)
	return FALSE;

    if (obj->operand == -1)
	return FALSE;

    if (obj->operand == current_stack_depth - 1)
	return TRUE;

    if (obj->operand == current_stack_depth - 2) {
	_exch_operands ();
	_trace_printf ("exch ");
	return TRUE;
    }

    return _pop_operands_to_depth (obj->operand + 1);
}

static cairo_bool_t
_pop_operands_to (enum operand_type t, const void *ptr)
{
    return _pop_operands_to_object (_get_object (t, ptr));
}

static cairo_bool_t
_is_current_object (Object *obj, int depth)
{
    if (current_stack_depth <= depth)
	return FALSE;
    return current_object[current_stack_depth-depth-1] == obj;
}

static cairo_bool_t
_is_current (enum operand_type type, const void *ptr, int depth)
{
    return _is_current_object (_get_object (type, ptr), depth);
}

static void
_push_object(Object *obj)
{
    assert(obj->operand == -1);

    if (current_stack_depth == ARRAY_LENGTH (current_object)) {
	int n;

	fprintf (stderr, "Operand stack overflow!\n");
	for (n = 0; n < current_stack_depth; n++) {
	    obj = current_object[n];

	    fprintf (stderr, "  [%3d] = %s%ld\n",
		     n, obj->type->op_code, obj->token);
	}

	abort ();
    }

    obj->operand = current_stack_depth;
    current_object[current_stack_depth++] = obj;
}

static void
_push_operand (enum operand_type t, const void *ptr)
{
    _push_object(_get_object(t, ptr));
}

static void
_object_remove (Object *obj)
{
    if (obj->operand != -1) {
	ensure_operands (1);
	if (obj->operand == current_stack_depth - 1) {
	    _trace_printf ("pop %% %s%ld destroyed\n",
			   obj->type->op_code, obj->token);
	} else if (obj->operand == current_stack_depth - 2) {
	    _exch_operands ();
	    _trace_printf ("exch pop %% %s%ld destroyed\n",
			   obj->type->op_code, obj->token);
	} else {
	    int n;

	    _trace_printf ("%d -1 roll pop %% %s%ld destroyed\n",
			   current_stack_depth - obj->operand,
			   obj->type->op_code, obj->token);

	    for (n = obj->operand; n < current_stack_depth - 1; n++) {
		current_object[n] = current_object[n+1];
		current_object[n]->operand = n;
	    }
	}
	obj->operand = -1;

	current_stack_depth--;
	dump_stack(__func__);
    }
}

static void
_object_undef (void *ptr)
{
    Object *obj = ptr;

    if (_write_lock ()) {
	_object_remove (obj);

	if (obj->defined) {
	    _trace_printf ("/%s%ld undef\n",
			   obj->type->op_code, obj->token);
	}

	_write_unlock ();
    }

    _object_destroy (obj);
}

static long
_create_context_id (cairo_t *cr)
{
    Object *obj;

    obj = _get_object (CONTEXT, cr);
    if (obj == NULL) {
	obj = _type_object_create (CONTEXT, cr);
	DLCALL (cairo_set_user_data,
		cr, &destroy_key, obj, _object_undef);
    }

    return obj->token;
}

static long
_get_id (enum operand_type op_type, const void *ptr)
{
    Object *obj;

    obj = _get_object (op_type, ptr);
    if (obj == NULL) {
	if (logfile != NULL) {
	    _trace_printf ("%% Unknown object of type %s, trace is incomplete.",
			   _get_type (op_type)->name);
	}
	_error = TRUE;
	return -1;
    }

    return obj->token;
}

static cairo_bool_t
_has_id (enum operand_type op_type, const void *ptr)
{
    return _get_object (op_type, ptr) != NULL;
}

static long
_create_font_face_id (cairo_font_face_t *font_face)
{
    Object *obj;

    obj = _get_object (FONT_FACE, font_face);
    if (obj == NULL) {
	obj = _type_object_create (FONT_FACE, font_face);
	DLCALL (cairo_font_face_set_user_data,
		font_face, &destroy_key, obj, _object_undef);
    }

    return obj->token;
}

static long
_get_font_face_id (cairo_font_face_t *font_face)
{
    return _get_id (FONT_FACE, font_face);
}

static void
_emit_font_face_id (cairo_font_face_t *font_face)
{
    Object *obj = _get_object (FONT_FACE, font_face);
    if (obj == NULL) {
	_trace_printf ("null ");
    } else {
	if (obj->defined) {
	    _trace_printf ("f%ld ", obj->token);
	} else {
	    _trace_printf ("%d index ", current_stack_depth - obj->operand - 1);
	}
    }
}

static cairo_bool_t
_has_pattern_id (cairo_pattern_t *pattern)
{
    return _has_id (PATTERN, pattern);
}

static long
_create_pattern_id (cairo_pattern_t *pattern)
{
    Object *obj;

    obj = _get_object (PATTERN, pattern);
    if (obj == NULL) {
	obj = _type_object_create (PATTERN, pattern);
	DLCALL (cairo_pattern_set_user_data,
		pattern, &destroy_key, obj, _object_undef);
    }

    return obj->token;
}

static void
_emit_pattern_id (cairo_pattern_t *pattern)
{
    Object *obj = _get_object (PATTERN, pattern);
    if (obj == NULL) {
	_trace_printf ("null ");
    } else {
	if (obj->defined) {
	    _trace_printf ("p%ld ", obj->token);
	} else {
	    _trace_printf ("%d index ",
			   current_stack_depth - obj->operand - 1);
	}
    }
}

static void
_emit_scaled_font_id (const cairo_scaled_font_t *scaled_font)
{
    Object *obj = _get_object (SCALED_FONT, scaled_font);
    if (obj == NULL) {
	_trace_printf ("null ");
    } else {
	if (obj->defined) {
	    _trace_printf ("sf%ld ", obj->token);
	} else {
	    _trace_printf ("%d index ",
		     current_stack_depth - obj->operand - 1);
	}
    }
}

static long
_create_scaled_font_id (cairo_scaled_font_t *font)
{
    Object *obj;

    assert(_get_object (SCALED_FONT, font) == NULL);
    obj = _type_object_create (SCALED_FONT, font);
    DLCALL (cairo_scaled_font_set_user_data,
	    font, &destroy_key, obj, _object_undef);

    return obj->token;
}

static cairo_bool_t
_has_scaled_font_id (const cairo_scaled_font_t *font)
{
    return _has_id (SCALED_FONT, font);
}

static Object *
_create_surface (cairo_surface_t *surface)
{
    Object *obj;

    obj = _get_object (SURFACE, surface);
    if (obj == NULL) {
	obj = _type_object_create (SURFACE, surface);
	DLCALL (cairo_surface_set_user_data,
		surface, &destroy_key, obj, _object_undef);
    }

    return obj;
}

static long
_get_surface_id (cairo_surface_t *surface)
{
    return _get_id (SURFACE, surface);
}

static cairo_bool_t
_matrix_is_identity (const cairo_matrix_t *m)
{
    return m->xx == 1. && m->yx == 0. &&
	   m->xy == 0. && m->yy == 1. &&
	   m->x0 == 0. && m->y0 == 0.;
}

#define BUFFER_SIZE 16384
struct _data_stream {
    z_stream zlib_stream;
    unsigned char zin_buf[BUFFER_SIZE];
    unsigned char zout_buf[BUFFER_SIZE];
    unsigned char four_tuple[4];
    int base85_pending;
};

static void
_write_zlib_data_start (struct _data_stream *stream)
{
    stream->zlib_stream.zalloc = Z_NULL;
    stream->zlib_stream.zfree  = Z_NULL;
    stream->zlib_stream.opaque  = Z_NULL;

    deflateInit (&stream->zlib_stream, Z_DEFAULT_COMPRESSION);

    stream->zlib_stream.next_in = stream->zin_buf;
    stream->zlib_stream.avail_in = 0;
    stream->zlib_stream.next_out = stream->zout_buf;
    stream->zlib_stream.avail_out = BUFFER_SIZE;
}

static void
_write_base85_data_start (struct _data_stream *stream)
{
    stream->base85_pending = 0;
}

static cairo_bool_t
_expand_four_tuple_to_five (unsigned char four_tuple[4],
			    unsigned char five_tuple[5])
{
    uint32_t value;
    int digit, i;
    cairo_bool_t all_zero = TRUE;

    value = four_tuple[0] << 24 |
	    four_tuple[1] << 16 |
	    four_tuple[2] << 8  |
	    four_tuple[3] << 0;
    for (i = 0; i < 5; i++) {
	digit = value % 85;
	if (digit != 0 && all_zero)
	    all_zero = FALSE;
	five_tuple[4-i] = digit + 33;
	value = value / 85;
    }

    return all_zero;
}

static void
_write_base85_data (struct _data_stream *stream,
		    const unsigned char	  *data,
		    unsigned long	   length)
{
    unsigned char five_tuple[5];
    int ret;

    assert (_should_trace ());

    while (length--) {
	stream->four_tuple[stream->base85_pending++] = *data++;
	if (stream->base85_pending == 4) {
	    if (_expand_four_tuple_to_five (stream->four_tuple, five_tuple))
		ret = fwrite ("z", 1, 1, logfile);
	    else
		ret = fwrite (five_tuple, 5, 1, logfile);
	    (void)ret;
	    stream->base85_pending = 0;
	}
    }
}

static void
_write_zlib_data (struct _data_stream *stream, cairo_bool_t flush)
{
    cairo_bool_t finished;

    do {
	int ret = deflate (&stream->zlib_stream, flush ? Z_FINISH : Z_NO_FLUSH);
	if (flush || stream->zlib_stream.avail_out == 0) {
	    _write_base85_data (stream,
				stream->zout_buf,
				BUFFER_SIZE - stream->zlib_stream.avail_out);
	    stream->zlib_stream.next_out = stream->zout_buf;
	    stream->zlib_stream.avail_out = BUFFER_SIZE;
	}

	finished = TRUE;
	if (stream->zlib_stream.avail_in != 0)
	    finished = FALSE;
	if (flush && ret != Z_STREAM_END)
	    finished = FALSE;
    } while (! finished);

    stream->zlib_stream.next_in = stream->zin_buf;
}

static void
_write_data_start (struct _data_stream *stream, uint32_t len)
{
    _write_zlib_data_start (stream);
    _write_base85_data_start (stream);

    _trace_printf ("<|");
    len = to_be32 (len);
    _write_base85_data (stream, (unsigned char *) &len, sizeof (len));
}

static void
_write_data (struct _data_stream *stream,
	     const void *data,
	     unsigned int length)
{
    unsigned int count;
    const unsigned char *p = data;

    while (length) {
	count = length;
	if (count > BUFFER_SIZE - stream->zlib_stream.avail_in)
	    count = BUFFER_SIZE - stream->zlib_stream.avail_in;
	memcpy (stream->zin_buf + stream->zlib_stream.avail_in, p, count);
	p += count;
	stream->zlib_stream.avail_in += count;
	length -= count;

	if (stream->zlib_stream.avail_in == BUFFER_SIZE)
	    _write_zlib_data (stream, FALSE);
    }
}

static void
_write_zlib_data_end (struct _data_stream *stream)
{
    _write_zlib_data (stream, TRUE);
    deflateEnd (&stream->zlib_stream);

}

static void
_write_base85_data_end (struct _data_stream *stream)
{
    unsigned char five_tuple[5];
    int ret;

    assert (_should_trace ());

    if (stream->base85_pending) {
	memset (stream->four_tuple + stream->base85_pending,
		0, 4 - stream->base85_pending);
	_expand_four_tuple_to_five (stream->four_tuple, five_tuple);
	ret = fwrite (five_tuple, stream->base85_pending+1, 1, logfile);
	(void) ret;
    }
}

static void
_write_data_end (struct _data_stream *stream)
{
    _write_zlib_data_end (stream);
    _write_base85_data_end (stream);

    _trace_printf ("~>");
}

static void
_emit_data (const void *data, unsigned int length)
{
    struct _data_stream stream;

    _write_data_start (&stream, length);
    _write_data (&stream, data, length);
    _write_data_end (&stream);
}

static const char *
_format_to_string (cairo_format_t format)
{
#define f(name) case CAIRO_FORMAT_ ## name: return #name
    switch (format) {
	f(INVALID);
	f(RGBA128F);
	f(RGB96F);
	f(ARGB32);
	f(RGB30);
	f(RGB24);
	f(RGB16_565);
	f(A8);
	f(A1);
    }
#undef f
    return "UNKNOWN_FORMAT";
}

static const char *
_format_to_content_string (cairo_format_t format)
{
    switch (format) {
    case CAIRO_FORMAT_INVALID:
	return "INVALID";
    case CAIRO_FORMAT_RGBA128F:
    case CAIRO_FORMAT_ARGB32:
	return "COLOR_ALPHA";
    case CAIRO_FORMAT_RGB96F:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_RGB24:
    case CAIRO_FORMAT_RGB16_565:
	return "COLOR";
    case CAIRO_FORMAT_A8:
    case CAIRO_FORMAT_A1:
	return "ALPHA";
    }
    return "UNKNOWN";
}

static const char *
_status_to_string (cairo_status_t status)
{
#define f(name) case CAIRO_STATUS_ ## name: return "STATUS_" #name
    switch (status) {
	f(SUCCESS);
	f(NO_MEMORY);
	f(INVALID_RESTORE);
	f(INVALID_POP_GROUP);
	f(NO_CURRENT_POINT);
	f(INVALID_MATRIX);
	f(INVALID_STATUS);
	f(NULL_POINTER);
	f(INVALID_STRING);
	f(INVALID_PATH_DATA);
	f(READ_ERROR);
	f(WRITE_ERROR);
	f(SURFACE_FINISHED);
	f(SURFACE_TYPE_MISMATCH);
	f(PATTERN_TYPE_MISMATCH);
	f(INVALID_CONTENT);
	f(INVALID_FORMAT);
	f(INVALID_VISUAL);
	f(FILE_NOT_FOUND);
	f(INVALID_DASH);
	f(INVALID_DSC_COMMENT);
	f(INVALID_INDEX);
	f(CLIP_NOT_REPRESENTABLE);
	f(TEMP_FILE_ERROR);
	f(INVALID_STRIDE);
	f(FONT_TYPE_MISMATCH);
	f(USER_FONT_IMMUTABLE);
	f(USER_FONT_ERROR);
	f(NEGATIVE_COUNT);
	f(INVALID_CLUSTERS);
	f(INVALID_SLANT);
	f(INVALID_WEIGHT);
	f(INVALID_SIZE);
	f(USER_FONT_NOT_IMPLEMENTED);
	f(DEVICE_TYPE_MISMATCH);
	f(DEVICE_ERROR);
	f(INVALID_MESH_CONSTRUCTION);
	f(DEVICE_FINISHED);
	f(JBIG2_GLOBAL_MISSING);
	f(PNG_ERROR);
	f(FREETYPE_ERROR);
	f(WIN32_GDI_ERROR);
	f(TAG_ERROR);
	f(DWRITE_ERROR);
	f(SVG_FONT_ERROR);
    case CAIRO_STATUS_LAST_STATUS:
	break;
    }
    return "UNKNOWN_STATUS";
#undef f
}

static void CAIRO_PRINTF_FORMAT(2, 3)
_emit_image (cairo_surface_t *image,
	     const char *info,
	     ...)
{
    int stride, row, width, height;
    uint32_t len;
    cairo_format_t format;
    uint8_t row_stack[BUFFER_SIZE];
    uint8_t *rowdata;
    uint8_t *data;
    struct _data_stream stream;
    cairo_status_t status;

    status = DLCALL (cairo_surface_status, image);
    if (status) {
	_trace_printf ("<< /status //%s >> image",
		       _status_to_string (status));
	return;
    }

    width = DLCALL (cairo_image_surface_get_width, image);
    height = DLCALL (cairo_image_surface_get_height, image);
    stride = DLCALL (cairo_image_surface_get_stride, image);
    format = DLCALL (cairo_image_surface_get_format, image);
    data = DLCALL (cairo_image_surface_get_data, image);

    _trace_printf ("dict\n"
		   "  /width %d set\n"
		   "  /height %d set\n"
		   "  /format //%s set\n",
		   width, height,
		   _format_to_string (format));
    if (info != NULL) {
	va_list ap;

	va_start (ap, info);
	_trace_vprintf (info, ap);
	va_end (ap);
    }

    if (DLCALL (cairo_version) >= CAIRO_VERSION_ENCODE (1, 9, 0)) {
	const char *mime_types[] = {
	    CAIRO_MIME_TYPE_JPEG,
	    CAIRO_MIME_TYPE_JP2,
	    CAIRO_MIME_TYPE_PNG,
	    NULL
	}, **mime_type;

	for (mime_type = mime_types; *mime_type; mime_type++) {
	    const unsigned char *mime_data;
	    unsigned long mime_length;

	    DLCALL (cairo_surface_get_mime_data,
		    image, *mime_type, &mime_data, &mime_length);
	    if (mime_data != NULL) {
		_trace_printf ("  /mime-type (%s) set\n"
			       "  /source <~",
			       *mime_type);
		_write_base85_data_start (&stream);
		_write_base85_data (&stream, mime_data, mime_length);
		_write_base85_data_end (&stream);
		_trace_printf ("~> set\n"
			       "  image");
		return;
	    }
	}
    }

    switch (format) {
    case CAIRO_FORMAT_A1:        len = (width + 7)/8; break;
    case CAIRO_FORMAT_A8:        len =  width; break;
    case CAIRO_FORMAT_RGB16_565: len = 2*width; break;
    case CAIRO_FORMAT_RGB24:     len = 3*width; break;
    default:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_INVALID:
    case CAIRO_FORMAT_ARGB32: len = 4*width; break;
    case CAIRO_FORMAT_RGB96F: len = 12*width; break;
    case CAIRO_FORMAT_RGBA128F: len = 16*width; break;
    }

    _trace_printf ("  /source ");
    _write_data_start (&stream, len * height);

#ifdef WORDS_BIGENDIAN
    switch (format) {
    case CAIRO_FORMAT_RGB24:
	for (row = height; row--; ) {
	    int col;
	    rowdata = data;
	    for (col = width; col--; ) {
		_write_data (&stream, rowdata, 3);
		rowdata+=4;
	    }
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_A1:
    case CAIRO_FORMAT_A8:
    case CAIRO_FORMAT_RGB16_565:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_ARGB32:
    case CAIRO_FORMAT_RGB96F:
    case CAIRO_FORMAT_RGBA128F:
	for (row = height; row--; ) {
	    _write_data (&stream, data, len);
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_INVALID:
    default:
	break;
    }
#else
    if (stride > ARRAY_LENGTH (row_stack)) {
	rowdata = malloc (stride);
	if (rowdata == NULL)
	    goto BAIL;
    } else
	rowdata = row_stack;

    switch (format) {
    case CAIRO_FORMAT_A1:
	for (row = height; row--; ) {
	    int col;
	    for (col = 0; col < (width + 7)/8; col++)
		rowdata[col] = CAIRO_BITSWAP8 (data[col]);
	    _write_data (&stream, rowdata, (width+7)/8);
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_A8:
	for (row = height; row--; ) {
	    _write_data (&stream, rowdata, width);
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_RGB16_565: /* XXX endianness */
	for (row = height; row--; ) {
	    uint16_t *src = (uint16_t *) data;
	    uint16_t *dst = (uint16_t *)rowdata;
	    int col;
	    for (col = 0; col < width; col++)
		dst[col] = bswap_16 (src[col]);
	    _write_data (&stream, rowdata, 2*width);
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_RGB24:
	for (row = height; row--; ) {
	    uint8_t *src = data;
	    int col;
	    for (col = 0; col < width; col++) {
		rowdata[3*col+2] = *src++;
		rowdata[3*col+1] = *src++;
		rowdata[3*col+0] = *src++;
		src++;
	    }
	    _write_data (&stream, rowdata, 3*width);
	    data += stride;
	}
	break;
    case CAIRO_FORMAT_RGB96F:
    case CAIRO_FORMAT_RGBA128F:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_ARGB32:
	for (row = height; row--; ) {
	    uint32_t *src = (uint32_t *) data;
	    uint32_t *dst = (uint32_t *) rowdata;
	    int col;
	    for (col = 0; col < width; col++)
		dst[col] = bswap_32 (src[col]);
	    _write_data (&stream, rowdata, len);
	    data += stride;
	}
	break;
   case CAIRO_FORMAT_INVALID:
    default:
	break;
    }
    if (rowdata != row_stack)
	free (rowdata);

BAIL:
    _write_data_end (&stream);
#endif
    _trace_printf (" set\n  image");
}

static void
_encode_string_literal (char *out, int max,
			const char *utf8, int len)
{
    char c;
    const char *end;

    *out++ = '(';
    max--;

    if (utf8 == NULL)
	goto DONE;

    if (len < 0)
	len = strlen (utf8);
    end = utf8 + len;

    while (utf8 < end) {
	if (max < 5)
	    break;

	switch ((c = *utf8++)) {
	case '\n':
	    *out++ = '\\';
	    *out++ = 'n';
	    max -= 2;
	    break;
	case '\r':
	    *out++ = '\\';
	    *out++ = 'r';
	    max -= 2;
	    break;
	case '\t':
	    *out++ = '\\';
	    *out++ = 't';
	    max -= 2;
	    break;
	case '\b':
	    *out++ = '\\';
	    *out++ = 'b';
	    max -= 2;
	    break;
	case '\f':
	    *out++ = '\\';
	    *out++ = 'f';
	    max -= 2;
	    break;
	case '\\':
	case '(':
	case ')':
	    *out++ = '\\';
	    *out++ = c;
	    max -= 2;
	    break;
	default:
	    if (_cairo_isprint (c)) {
		*out++ = c;
	    } else {
		int octal = 0;
		while (c) {
		    octal *= 10;
		    octal += c&7;
		    c /= 8;
		}
		octal = snprintf (out, max, "\\%03d", octal);
		out += octal;
		max -= octal;
	    }
	    break;
	}
    }
DONE:
    *out++ = ')';
    *out = '\0';
}

static void
to_octal (int value, char *buf, size_t size)
{
    do {
	buf[--size] = '0' + (value & 7);
	value >>= 3;
    } while (size);
}

static void
_emit_string_literal (const char *utf8, int len)
{
    char c;
    const char *end;

    if (utf8 == NULL) {
	_trace_printf ("()");
	return;
    }

    if (len < 0)
	len = strlen (utf8);
    end = utf8 + len;

    _trace_printf ("(");
    while (utf8 < end) {
	switch ((c = *utf8++)) {
	case '\n':
	    c = 'n';
	    goto ESCAPED_CHAR;
	case '\r':
	    c = 'r';
	    goto ESCAPED_CHAR;
	case '\t':
	    c = 't';
	    goto ESCAPED_CHAR;
	case '\b':
	    c = 'b';
	    goto ESCAPED_CHAR;
	case '\f':
	    c = 'f';
	    goto ESCAPED_CHAR;
	case '\\':
	case '(':
	case ')':
ESCAPED_CHAR:
	    _trace_printf ("\\%c", c);
	    break;
	default:
	    if (_cairo_isprint (c)) {
		_trace_printf ("%c", c);
	    } else {
		char buf[4] = { '\\' };
		int ret_ignored;

		to_octal (c, buf+1, 3);
		ret_ignored = fwrite (buf, 4, 1, logfile);
		(void)ret_ignored;
	    }
	    break;
	}
    }
    _trace_printf (")");
}

static void
_emit_current (Object *obj)
{
    if (obj != NULL && ! _pop_operands_to_object (obj)) {
	if (obj->operand != -1) {
	    int n;

	    _trace_printf ("%d -1 roll %% %s%ld\n",
			   current_stack_depth - obj->operand,
			   obj->type->op_code, obj->token);

	    for (n = obj->operand; n < current_stack_depth - 1; n++) {
		current_object[n] = current_object[n+1];
		current_object[n]->operand = n;
	    }
	    obj->operand = -1;
	    current_stack_depth--;
	} else {
	    assert(obj->defined);
	    _trace_printf ("%s%ld\n", obj->type->op_code, obj->token);
	}

	_push_object (obj);
	dump_stack(__func__);
    }
}

static void
_emit_context (cairo_t *cr)
{
    _emit_current (_get_object (CONTEXT, cr));
}

static void
_emit_pattern (cairo_pattern_t *pattern)
{
    _emit_current (_get_object (PATTERN, pattern));
}

static void
_emit_surface (cairo_surface_t *surface)
{
    _emit_current (_get_object (SURFACE, surface));
}

static void CAIRO_PRINTF_FORMAT(2, 3)
_emit_cairo_op (cairo_t *cr, const char *fmt, ...)
{
    va_list ap;

    if (cr == NULL || ! _write_lock ())
	return;

    _emit_context (cr);

    va_start (ap, fmt);
    _trace_vprintf ( fmt, ap);
    va_end (ap);

    _write_unlock ();
}

cairo_t *
cairo_create (cairo_surface_t *target)
{
    cairo_t *ret;
    long surface_id;
    long context_id;

    _enter_trace ();

    ret = DLCALL (cairo_create, target);
    context_id = _create_context_id (ret);

    _emit_line_info ();
    if (target != NULL && _write_lock ()) {
	surface_id = _get_surface_id (target);
	if (surface_id != -1) {
	    _get_object (SURFACE, target)->foreign = FALSE;

	    /* we presume that we will continue to use the context */
	    if (_pop_operands_to (SURFACE, target)){
		_consume_operand (false);
	    } else {
		_trace_printf ("s%ld ", surface_id);
	    }
	    _trace_printf ("context %% c%ld\n", context_id);
	    _push_operand (CONTEXT, ret);
	    dump_stack(__func__);
	}
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

void
cairo_save (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "save\n");
    DLCALL (cairo_save, cr);
    _exit_trace ();
}

void
cairo_restore (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "restore\n");
    DLCALL (cairo_restore, cr);
    _exit_trace ();
}

void
cairo_push_group (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "//COLOR_ALPHA push-group\n");
    DLCALL (cairo_push_group, cr);
    _exit_trace ();
}

static const char *
_content_to_string (cairo_content_t content)
{
    switch (content) {
    case CAIRO_CONTENT_ALPHA: return "ALPHA";
    case CAIRO_CONTENT_COLOR: return "COLOR";
    default:
    case CAIRO_CONTENT_COLOR_ALPHA: return "COLOR_ALPHA";
    }
}

void
cairo_push_group_with_content (cairo_t *cr, cairo_content_t content)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "//%s push-group\n", _content_to_string (content));
    DLCALL (cairo_push_group_with_content, cr, content);
    _exit_trace ();
}

cairo_pattern_t *
cairo_pop_group (cairo_t *cr)
{
    cairo_pattern_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_pop_group, cr);

    _emit_line_info ();
    _emit_cairo_op (cr, "pop-group %% p%ld\n", _create_pattern_id (ret));
    _push_operand (PATTERN, ret);
    dump_stack(__func__);

    _exit_trace ();
    return ret;
}

void
cairo_pop_group_to_source (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "pop-group set-source\n");
    DLCALL (cairo_pop_group_to_source, cr);
    _exit_trace ();
}

static const char *
_operator_to_string (cairo_operator_t op)
{
#define f(name) case CAIRO_OPERATOR_ ## name: return #name
    switch (op) {
	f(OVER);
	f(SOURCE);
	f(CLEAR);
	f(IN);
	f(OUT);
	f(ATOP);
	f(DEST);
	f(DEST_OVER);
	f(DEST_IN);
	f(DEST_OUT);
	f(DEST_ATOP);
	f(XOR);
	f(ADD);
	f(SATURATE);
	f(MULTIPLY);
	f(SCREEN);
	f(OVERLAY);
	f(DARKEN);
	f(LIGHTEN);
        case CAIRO_OPERATOR_COLOR_DODGE: return "DODGE";
        case CAIRO_OPERATOR_COLOR_BURN: return "BURN";
	f(HARD_LIGHT);
	f(SOFT_LIGHT);
	f(DIFFERENCE);
	f(EXCLUSION);
	f(HSL_HUE);
	f(HSL_SATURATION);
	f(HSL_COLOR);
	f(HSL_LUMINOSITY);
    }
#undef f
    return "UNKNOWN_OPERATOR";
}

void
cairo_set_operator (cairo_t *cr, cairo_operator_t op)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "//%s set-operator\n", _operator_to_string (op));
    DLCALL (cairo_set_operator, cr, op);
    _exit_trace ();
}

void
cairo_set_source_rgb (cairo_t *cr, double red, double green, double blue)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g %g set-source-rgb\n", red, green, blue);
    DLCALL (cairo_set_source_rgb, cr, red, green, blue);
    _exit_trace ();
}

void
cairo_set_source_rgba (cairo_t *cr, double red, double green, double blue, double alpha)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g %g %g set-source-rgba\n",
		    red, green, blue, alpha);
    DLCALL (cairo_set_source_rgba, cr, red, green, blue, alpha);
    _exit_trace ();
}

static void
_emit_source_image (cairo_surface_t *surface)
{
    Object *obj;
    cairo_surface_t *image;
    cairo_t *cr;

    obj = _get_object (SURFACE, surface);
    if (obj == NULL)
	return;

    image = DLCALL (cairo_image_surface_create,
		    CAIRO_FORMAT_ARGB32,
		    obj->width,
		    obj->height);
    cr = DLCALL (cairo_create, image);
    DLCALL (cairo_set_source_surface, cr, surface, 0, 0);
    DLCALL (cairo_paint, cr);
    DLCALL (cairo_destroy, cr);

    _emit_image (image, NULL);
    _trace_printf (" set-source-image ");
    DLCALL (cairo_surface_destroy, image);

    obj->foreign = FALSE;
}

static void
_emit_source_image_rectangle (cairo_surface_t *surface,
			      int x, int y,
			      int width, int height)
{
    Object *obj;
    cairo_surface_t *image;
    cairo_t *cr;

    obj = _get_object (SURFACE, surface);
    if (obj == NULL)
	return;

    if (obj->foreign) {
	_emit_source_image (surface);
	return;
    }

    image = DLCALL (cairo_image_surface_create,
		    CAIRO_FORMAT_ARGB32,
		    width,
		    height);
    cr = DLCALL (cairo_create, image);
    DLCALL (cairo_set_source_surface, cr, surface, x, y);
    DLCALL (cairo_paint, cr);
    DLCALL (cairo_destroy, cr);

    _emit_image (image, NULL);
    _trace_printf (" %d %d set-device-offset set-source-image ",
	     x, y);
    DLCALL (cairo_surface_destroy, image);
}

void
cairo_set_source_surface (cairo_t *cr, cairo_surface_t *surface, double x, double y)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && surface != NULL && _write_lock ()) {
	Object *obj = _get_object (SURFACE, surface);

	if (_is_current (SURFACE, surface, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    _consume_operand (false);
	}
	else if (_is_current (SURFACE, surface, 1) &&
		 _is_current (CONTEXT, cr, 0) &&
		 obj->defined)
	{
	    _trace_printf ("exch ");
	    _exch_operands ();
	    _consume_operand (false);
	} else if (obj->defined) {
	    _emit_context (cr);
	    _trace_printf ("s%ld ", obj->token);
	} else {
	    _emit_context (cr);
	    _trace_printf ("%d index ",
			   current_stack_depth - obj->operand - 1);
	}

	if (obj->foreign)
	    _emit_source_image (surface);

	_trace_printf ("pattern");
	if (x != 0. || y != 0.)
	    _trace_printf (" %g %g translate", -x, -y);

	_trace_printf (" set-source\n");
	_write_unlock ();
    }

    DLCALL (cairo_set_source_surface, cr, surface, x, y);
    _exit_trace ();
}

void
cairo_set_source (cairo_t *cr, cairo_pattern_t *source)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && source != NULL && _write_lock ()) {
	Object *obj = _get_object (PATTERN, source);
	cairo_bool_t need_context_and_pattern = TRUE;

	if (_is_current (PATTERN, source, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    if (obj->defined) {
		_consume_operand (false);
	    } else {
		_trace_printf ("exch 1 index ");
		_exch_operands ();
	    }
	    need_context_and_pattern = FALSE;
	}
	else if (_is_current (PATTERN, source, 1) &&
		 _is_current (CONTEXT, cr, 0))
	{
	    if (obj->defined) {
		_trace_printf ("exch ");
		_exch_operands ();
		_consume_operand (false);
		need_context_and_pattern = FALSE;
	    }
	}

	if (need_context_and_pattern) {
	    _emit_context (cr);
	    _emit_pattern_id (source);
	}

	_trace_printf ("set-source %% p%ld\n", obj->token);
	_write_unlock ();
    }

    DLCALL (cairo_set_source, cr, source);
    _exit_trace ();
}

cairo_pattern_t *
cairo_get_source (cairo_t *cr)
{
    cairo_pattern_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_get_source, cr);

    if (! _has_pattern_id (ret)) {
	_emit_cairo_op (cr, "/source get /p%ld exch def\n",
			_create_pattern_id (ret));
	_get_object (PATTERN, ret)->defined = TRUE;
    }

    _exit_trace ();
    return ret;
}

void
cairo_set_tolerance (cairo_t *cr, double tolerance)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g set-tolerance\n", tolerance);
    DLCALL (cairo_set_tolerance, cr, tolerance);
    _exit_trace ();
}

static const char *
_antialias_to_string (cairo_antialias_t antialias)
{
#define f(name) case CAIRO_ANTIALIAS_ ## name: return "ANTIALIAS_" #name
    switch (antialias) {
	f(DEFAULT);

	f(NONE);
	f(GRAY);
	f(SUBPIXEL);

	f(FAST);
	f(GOOD);
	f(BEST);
    };
#undef f
    return "UNKNOWN_ANTIALIAS";
}

void
cairo_set_antialias (cairo_t *cr, cairo_antialias_t antialias)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr,
		    "//%s set-antialias\n", _antialias_to_string (antialias));
    DLCALL (cairo_set_antialias, cr, antialias);
    _exit_trace ();
}

static const char *
_fill_rule_to_string (cairo_fill_rule_t rule)
{
#define f(name) case CAIRO_FILL_RULE_ ## name: return #name
    switch (rule) {
	f(WINDING);
	f(EVEN_ODD);
    };
#undef f
    return "UNKNOWN_FILL_RULE";
}

void
cairo_set_fill_rule (cairo_t *cr, cairo_fill_rule_t fill_rule)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr,
		    "//%s set-fill-rule\n", _fill_rule_to_string (fill_rule));
    DLCALL (cairo_set_fill_rule, cr, fill_rule);
    _exit_trace ();
}

void
cairo_set_line_width (cairo_t *cr, double width)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g set-line-width\n", width);
    DLCALL (cairo_set_line_width, cr, width);
    _exit_trace ();
}

static const char *
_line_cap_to_string (cairo_line_cap_t line_cap)
{
#define f(name) case CAIRO_LINE_CAP_ ## name: return "LINE_CAP_" #name
    switch (line_cap) {
	f(BUTT);
	f(ROUND);
	f(SQUARE);
    };
#undef f
    return "UNKNOWN_LINE_CAP";
}

void
cairo_set_line_cap (cairo_t *cr, cairo_line_cap_t line_cap)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "//%s set-line-cap\n", _line_cap_to_string (line_cap));
    DLCALL (cairo_set_line_cap, cr, line_cap);
    _exit_trace ();
}

static const char *
_line_join_to_string (cairo_line_join_t line_join)
{
#define f(name) case CAIRO_LINE_JOIN_ ## name: return "LINE_JOIN_" #name
    switch (line_join) {
	f(MITER);
	f(ROUND);
	f(BEVEL);
    };
#undef f
    return "UNKNOWN_LINE_JOIN";
}

void
cairo_set_line_join (cairo_t *cr, cairo_line_join_t line_join)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr,
		    "//%s set-line-join\n", _line_join_to_string (line_join));
    DLCALL (cairo_set_line_join, cr, line_join);
    _exit_trace ();
}

void
cairo_set_dash (cairo_t *cr, const double *dashes, int num_dashes, double offset)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && _write_lock ()) {
	int n;

	_emit_context (cr);

	_trace_printf ("[");
	for (n = 0; n <  num_dashes; n++) {
	    if (n != 0)
		_trace_printf (" ");
	    _trace_printf ("%g", dashes[n]);
	}
	_trace_printf ("] %g set-dash\n", offset);

	_write_unlock ();
    }

    DLCALL (cairo_set_dash, cr, dashes, num_dashes, offset);
    _exit_trace ();
}

void
cairo_set_miter_limit (cairo_t *cr, double limit)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g set-miter-limit\n", limit);
    DLCALL (cairo_set_miter_limit, cr, limit);
    _exit_trace ();
}

void
cairo_translate (cairo_t *cr, double tx, double ty)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g translate\n", tx, ty);
    DLCALL (cairo_translate, cr, tx, ty);
    _exit_trace ();
}

void
cairo_scale (cairo_t *cr, double sx, double sy)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g scale\n", sx, sy);
    DLCALL (cairo_scale, cr, sx, sy);
    _exit_trace ();
}

void
cairo_rotate (cairo_t *cr, double angle)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g rotate\n", angle);
    DLCALL (cairo_rotate, cr, angle);
    _exit_trace ();
}

void
cairo_transform (cairo_t *cr, const cairo_matrix_t *matrix)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g %g %g %g %g matrix transform\n",
		    matrix->xx, matrix->yx,
		    matrix->xy, matrix->yy,
		    matrix->x0, matrix->y0);
    DLCALL (cairo_transform, cr, matrix);
    _exit_trace ();
}

void
cairo_set_matrix (cairo_t *cr, const cairo_matrix_t *matrix)
{
    _enter_trace ();
    _emit_line_info ();
    if (_matrix_is_identity (matrix)) {
	_emit_cairo_op (cr, "identity set-matrix\n");
    } else {
	_emit_cairo_op (cr, "%g %g %g %g %g %g matrix set-matrix\n",
			matrix->xx, matrix->yx,
			matrix->xy, matrix->yy,
			matrix->x0, matrix->y0);
    }
    DLCALL (cairo_set_matrix, cr, matrix);
    _exit_trace ();
}

cairo_surface_t *
cairo_get_target (cairo_t *cr)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_get_target, cr);
    if (cr != NULL) {
	Object *obj = _create_surface (ret);

	if (! obj->defined) {
	    _emit_cairo_op (cr,
			    "/target get /s%ld exch def\n",
			    obj->token);
	    obj->defined = TRUE;
	}
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_get_group_target (cairo_t *cr)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_get_group_target, cr);
    if (cr != NULL) {
	Object *obj = _create_surface (ret);

	if (! obj->defined) {
	    _emit_cairo_op (cr,
			    "/group-target get /s%ld exch def\n",
			    obj->token);
	    obj->defined = TRUE;
	}
    }

    _exit_trace ();
    return ret;
}

void
cairo_identity_matrix (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "identity set-matrix\n");
    DLCALL (cairo_identity_matrix, cr);
    _exit_trace ();
}

void
cairo_new_path (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "n ");
    DLCALL (cairo_new_path, cr);
    _exit_trace ();
}

void
cairo_move_to (cairo_t *cr, double x, double y)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g m ", x, y);
    DLCALL (cairo_move_to, cr, x, y);
    _exit_trace ();
}

void
cairo_new_sub_path (cairo_t *cr)
{
    _enter_trace ();
    _emit_cairo_op (cr, "N ");
    DLCALL (cairo_new_sub_path, cr);
    _exit_trace ();
}

void
cairo_line_to (cairo_t *cr, double x, double y)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g l ", x, y);
    DLCALL (cairo_line_to, cr, x, y);
    _exit_trace ();
}

void
cairo_curve_to (cairo_t *cr, double x1, double y1, double x2, double y2, double x3, double y3)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g %g %g %g %g c ", x1, y1, x2, y2, x3, y3);
    DLCALL (cairo_curve_to, cr, x1, y1, x2, y2, x3, y3);
    _exit_trace ();
}

void
cairo_arc (cairo_t *cr, double xc, double yc, double radius, double angle1, double angle2)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g %g %g %g arc\n", xc, yc, radius, angle1, angle2);
    DLCALL (cairo_arc, cr, xc, yc, radius, angle1, angle2);
    _exit_trace ();
}

void
cairo_arc_negative (cairo_t *cr, double xc, double yc, double radius, double angle1, double angle2)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g %g %g %g arc-\n",
		    xc, yc, radius, angle1, angle2);
    DLCALL (cairo_arc_negative, cr, xc, yc, radius, angle1, angle2);
    _exit_trace ();
}

void
cairo_rel_move_to (cairo_t *cr, double dx, double dy)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g M ", dx, dy);
    DLCALL (cairo_rel_move_to, cr, dx, dy);
    _exit_trace ();
}

void
cairo_rel_line_to (cairo_t *cr, double dx, double dy)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g L ", dx, dy);
    DLCALL (cairo_rel_line_to, cr, dx, dy);
    _exit_trace ();
}

void
cairo_rel_curve_to (cairo_t *cr, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g %g %g %g %g C ",
		    dx1, dy1, dx2, dy2, dx3, dy3);
    DLCALL (cairo_rel_curve_to, cr, dx1, dy1, dx2, dy2, dx3, dy3);
    _exit_trace ();
}

void
cairo_rectangle (cairo_t *cr, double x, double y, double width, double height)
{
    _enter_trace ();
    _emit_cairo_op (cr, "%g %g %g %g rectangle\n", x, y, width, height);
    DLCALL (cairo_rectangle, cr, x, y, width, height);
    _exit_trace ();
}

void
cairo_close_path (cairo_t *cr)
{
    _enter_trace ();
    _emit_cairo_op (cr, "h\n");
    DLCALL (cairo_close_path, cr);
    _exit_trace ();
}

void
cairo_paint (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "paint\n");
    DLCALL (cairo_paint, cr);
    _exit_trace ();
}

void
cairo_paint_with_alpha (cairo_t *cr, double alpha)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g paint-with-alpha\n", alpha);
    DLCALL (cairo_paint_with_alpha, cr, alpha);
    _exit_trace ();
}

void
cairo_mask (cairo_t *cr, cairo_pattern_t *pattern)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && pattern != NULL && _write_lock ()) {
	Object *obj = _get_object (PATTERN, pattern);
	cairo_bool_t need_context_and_pattern = TRUE;

	if (_is_current (PATTERN, pattern, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    if (obj->defined) {
		_consume_operand (false);
		need_context_and_pattern = FALSE;
	    }
	}
	else if (_is_current (PATTERN, pattern, 1) &&
		 _is_current (CONTEXT, cr, 0))
	{
	    if (obj->defined) {
		_trace_printf ("exch ");
		_exch_operands ();
		_consume_operand (false);
		need_context_and_pattern = FALSE;
	    }
	}

	if (need_context_and_pattern) {
	    _emit_context (cr);
	    _emit_pattern_id (pattern);
	}

	_trace_printf (" mask\n");
	_write_unlock ();
    }
    DLCALL (cairo_mask, cr, pattern);
    _exit_trace ();
}

void
cairo_mask_surface (cairo_t *cr, cairo_surface_t *surface, double x, double y)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && surface != NULL && _write_lock ()) {
	Object *obj = _get_object (SURFACE, surface);
	if (_is_current (SURFACE, surface, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    _consume_operand (false);
	}
	else if (_is_current (SURFACE, surface, 1) &&
		 _is_current (CONTEXT, cr, 0))
	{
	    _trace_printf ("exch ");
	    _exch_operands ();
	    _consume_operand (false);
	} else if (obj->defined){
	    _emit_context (cr);
	    _trace_printf ("s%ld ", obj->token);
	} else {
	    _emit_context (cr);
	    _trace_printf ("%d index ",
			   current_stack_depth - obj->operand - 1);
	}
	_trace_printf ("pattern");

	if (x != 0. || y != 0.)
	    _trace_printf (" %g %g translate", -x, -y);

	_trace_printf (" mask\n");
	_write_unlock ();
    }

    DLCALL (cairo_mask_surface, cr, surface, x, y);
    _exit_trace ();
}

void
cairo_stroke (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "stroke\n");
    DLCALL (cairo_stroke, cr);
    _exit_trace ();
}

void
cairo_stroke_preserve (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "stroke+\n");
    DLCALL (cairo_stroke_preserve, cr);
    _exit_trace ();
}

void
cairo_fill (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "fill\n");
    DLCALL (cairo_fill, cr);
    _exit_trace ();
}

void
cairo_fill_preserve (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "fill+\n");
    DLCALL (cairo_fill_preserve, cr);
    _exit_trace ();
}

void
cairo_copy_page (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "copy-page\n");
    DLCALL (cairo_copy_page, cr);
    _exit_trace ();
}

void
cairo_show_page (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "show-page\n");
    DLCALL (cairo_show_page, cr);
    _exit_trace ();
}

void
cairo_clip (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "clip\n");
    DLCALL (cairo_clip, cr);
    _exit_trace ();
}

void
cairo_clip_preserve (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "clip+\n");
    DLCALL (cairo_clip_preserve, cr);
    _exit_trace ();
}

void
cairo_reset_clip (cairo_t *cr)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "reset-clip\n");
    DLCALL (cairo_reset_clip, cr);
    _exit_trace ();
}


static const char *
_slant_to_string (cairo_font_slant_t font_slant)
{
#define f(name) case CAIRO_FONT_SLANT_ ## name: return "SLANT_" #name
    switch (font_slant) {
	f(NORMAL);
	f(ITALIC);
	f(OBLIQUE);
    };
#undef f
    return "UNKNOWN_SLANT";
}

static const char *
_weight_to_string (cairo_font_weight_t font_weight)
{
#define f(name) case CAIRO_FONT_WEIGHT_ ## name: return "WEIGHT_" #name
    switch (font_weight) {
	f(NORMAL);
	f(BOLD);
    };
#undef f
    return "UNKNOWN_WEIGHT";
}

void
cairo_select_font_face (cairo_t *cr, const char *family, cairo_font_slant_t slant, cairo_font_weight_t weight)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && _write_lock ()) {
	_emit_context (cr);
	_emit_string_literal (family, -1);
	_trace_printf (" //%s //%s select-font-face\n",
		       _slant_to_string (slant),
		       _weight_to_string (weight));
	_write_unlock ();
    }
    DLCALL (cairo_select_font_face, cr, family, slant, weight);
    _exit_trace ();
}

cairo_font_face_t *
cairo_get_font_face (cairo_t *cr)
{
    cairo_font_face_t *ret;
    long font_face_id;

    _enter_trace ();

    ret = DLCALL (cairo_get_font_face, cr);
    font_face_id = _create_font_face_id (ret);

    _emit_cairo_op (cr, "/font-face get %% f%ld\n", font_face_id);
    _push_operand (FONT_FACE, ret);
    dump_stack(__func__);

    _exit_trace ();
    return ret;
}

void
cairo_set_font_face (cairo_t *cr, cairo_font_face_t *font_face)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && font_face != NULL && _write_lock ()) {
	if (_is_current (FONT_FACE, font_face, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    _consume_operand (false);
	}
	else if (_is_current (FONT_FACE, font_face, 1) &&
		 _is_current (CONTEXT, cr, 0))
	{
	    _trace_printf ("exch ");
	    _exch_operands ();
	    _consume_operand (false);
	}
	else
	{
	    _emit_context (cr);
	    _emit_font_face_id (font_face);
	}

	_trace_printf ("set-font-face\n");
	_write_unlock ();
    }

    DLCALL (cairo_set_font_face, cr, font_face);
    _exit_trace ();
}

void
cairo_set_font_size (cairo_t *cr, double size)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g set-font-size\n", size);
    DLCALL (cairo_set_font_size, cr, size);
    _exit_trace ();
}

void
cairo_set_font_matrix (cairo_t *cr, const cairo_matrix_t *matrix)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_cairo_op (cr, "%g %g %g %g %g %g matrix set-font-matrix\n",
		    matrix->xx, matrix->yx,
		    matrix->xy, matrix->yy,
		    matrix->x0, matrix->y0);
    DLCALL (cairo_set_font_matrix, cr, matrix);
    _exit_trace ();
}

static const char *
_subpixel_order_to_string (cairo_subpixel_order_t subpixel_order)
{
#define f(name) case CAIRO_SUBPIXEL_ORDER_ ## name: return "SUBPIXEL_ORDER_" #name
    switch (subpixel_order) {
	f(DEFAULT);
	f(RGB);
	f(BGR);
	f(VRGB);
	f(VBGR);
    };
#undef f
    return "UNKNOWN_SUBPIXEL_ORDER";
}

static const char *
_hint_style_to_string (cairo_hint_style_t hint_style)
{
#define f(name) case CAIRO_HINT_STYLE_ ## name: return "HINT_STYLE_" #name
    switch (hint_style) {
	f(DEFAULT);
	f(NONE);
	f(SLIGHT);
	f(MEDIUM);
	f(FULL);
    };
#undef f
    return "UNKNOWN_HINT_STYLE";
}

static const char *
_hint_metrics_to_string (cairo_hint_metrics_t hint_metrics)
{
#define f(name) case CAIRO_HINT_METRICS_ ## name: return "HINT_METRICS_" #name
    switch (hint_metrics) {
	f(DEFAULT);
	f(OFF);
	f(ON);
    };
#undef f
    return "UNKNOWN_HINT_METRICS";
}

static void
_emit_font_options (const cairo_font_options_t *options)
{
    cairo_antialias_t antialias;
    cairo_subpixel_order_t subpixel_order;
    cairo_hint_style_t hint_style;
    cairo_hint_metrics_t hint_metrics;

    _trace_printf ("<<");

    antialias = DLCALL (cairo_font_options_get_antialias, options);
    if (antialias != CAIRO_ANTIALIAS_DEFAULT) {
	_trace_printf (" /antialias //%s",
		       _antialias_to_string (antialias));
    }

    subpixel_order = DLCALL (cairo_font_options_get_subpixel_order, options);
    if (subpixel_order != CAIRO_SUBPIXEL_ORDER_DEFAULT) {
	_trace_printf (" /subpixel-order //%s",
		       _subpixel_order_to_string (subpixel_order));
    }

    hint_style = DLCALL (cairo_font_options_get_hint_style, options);
    if (hint_style != CAIRO_HINT_STYLE_DEFAULT) {
	_trace_printf (" /hint-style //%s",
		       _hint_style_to_string (hint_style));
    }

    hint_metrics = DLCALL (cairo_font_options_get_hint_metrics, options);
    if (hint_metrics != CAIRO_HINT_METRICS_DEFAULT) {
	_trace_printf (" /hint-metrics //%s",
		       _hint_metrics_to_string (hint_metrics));
    }

    _trace_printf (" >>");
}

void
cairo_set_font_options (cairo_t *cr, const cairo_font_options_t *options)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && options != NULL && _write_lock ()) {
	_emit_context (cr);
	_emit_font_options (options);
	_trace_printf (" set-font-options\n");
	_write_unlock ();
    }

    DLCALL (cairo_set_font_options, cr, options);
    _exit_trace ();
}

cairo_scaled_font_t *
cairo_get_scaled_font (cairo_t *cr)
{
    cairo_scaled_font_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_get_scaled_font, cr);

    if (cr != NULL && ! _has_scaled_font_id (ret)) {
	_emit_cairo_op (cr, "/scaled-font get /sf%ld exch def\n",
			_create_scaled_font_id (ret));
	_get_object (SCALED_FONT, ret)->defined = TRUE;
    }

    _exit_trace ();
    return ret;
}

void
cairo_set_scaled_font (cairo_t *cr, const cairo_scaled_font_t *scaled_font)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && scaled_font != NULL && _write_lock ()) {
	Object *obj = _get_object (SCALED_FONT, scaled_font);
	cairo_bool_t need_context_and_font = TRUE;

	if (_is_current (SCALED_FONT, scaled_font, 0) &&
	    _is_current (CONTEXT, cr, 1))
	{
	    if (obj->defined) {
		_consume_operand (false);
	    } else {
		_trace_printf ("exch 1 index ");
		_exch_operands ();
	    }
	    need_context_and_font = FALSE;
	}
	else if (_is_current (SCALED_FONT, scaled_font, 1) &&
		 _is_current (CONTEXT, cr, 0))
	{
	    if (obj->defined) {
		_trace_printf ("exch ");
		_exch_operands ();
		_consume_operand (false);
		need_context_and_font = FALSE;
	    }
	}

	if (need_context_and_font) {
	    _emit_context (cr);
	    _emit_scaled_font_id (scaled_font);
	}

	_trace_printf ("set-scaled-font\n");

	_write_unlock ();
    }
    DLCALL (cairo_set_scaled_font, cr, scaled_font);
    _exit_trace ();
}

static void
_emit_matrix (const cairo_matrix_t *m)
{
    if (_matrix_is_identity(m))
    {
	_trace_printf ("identity");
    }
    else
    {
	_trace_printf ("%g %g %g %g %g %g matrix",
		       m->xx, m->yx,
		       m->xy, m->yy,
		       m->x0, m->y0);
    }
}

cairo_scaled_font_t *
cairo_scaled_font_create (cairo_font_face_t *font_face,
			  const cairo_matrix_t *font_matrix,
			  const cairo_matrix_t *ctm,
			  const cairo_font_options_t *options)
{
    cairo_scaled_font_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_scaled_font_create, font_face, font_matrix, ctm, options);
    if (_has_scaled_font_id (ret))
	    goto out;

    _emit_line_info ();
    if (font_face != NULL &&
	font_matrix != NULL &&
	ctm != NULL &&
	options != NULL
	&& _write_lock ())
    {
	Object *obj;

	obj = _type_object_create (SCALED_FONT, ret);
	DLCALL (cairo_scaled_font_set_user_data,
		ret, &destroy_key, obj, _object_undef);

	if (_pop_operands_to (FONT_FACE, font_face))
	    _consume_operand (false);
	else
	    _trace_printf ("f%ld ", _get_font_face_id (font_face));

	_emit_matrix (font_matrix);
	_trace_printf (" ");

	_emit_matrix (ctm);
	_trace_printf (" ");

	_emit_font_options (options);

	_trace_printf (" scaled-font /sf%ld exch def\n",
		       obj->token);
	obj->defined = TRUE;

	_write_unlock ();
    }

out:
    _exit_trace ();
    return ret;
}

void
cairo_show_text (cairo_t *cr, const char *utf8)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && _write_lock ()) {
	_emit_context (cr);
	_emit_string_literal (utf8, -1);
	_trace_printf (" show-text\n");
	_write_unlock ();
    }
    DLCALL (cairo_show_text, cr, utf8);
    _exit_trace ();
}

static void
_glyph_advance (cairo_scaled_font_t *font,
		const cairo_glyph_t *glyph,
		double *x, double *y)
{
    cairo_text_extents_t extents;

    DLCALL (cairo_scaled_font_glyph_extents, font, glyph, 1, &extents);
    *x += extents.x_advance;
    *y += extents.y_advance;
}

#define TOLERANCE 1e-5
static void
_emit_glyphs (cairo_scaled_font_t *font,
	      const cairo_glyph_t *glyphs,
	      int num_glyphs)
{
    double x,y;
    int n;

    if (num_glyphs == 0) {
	_trace_printf ("[]");
	return;
    }

    for (n = 0; n < num_glyphs; n++) {
	if (glyphs[n].index > 255)
	    break;
    }

    x = glyphs->x;
    y = glyphs->y;
    if (n < num_glyphs) { /* need full glyph range */
	cairo_bool_t first;

	_trace_printf ("[%g %g [", x, y);
	first = TRUE;
	while (num_glyphs--) {
	    if (fabs (glyphs->x - x) > TOLERANCE ||
		fabs (glyphs->y - y) > TOLERANCE)
	    {
		x = glyphs->x;
		y = glyphs->y;
		_trace_printf ("] %g %g [", x, y);
		first = TRUE;
	    }

	    if (! first)
		_trace_printf (" ");
	    _trace_printf ("%lu", glyphs->index);
	    first = FALSE;

	    _glyph_advance (font, glyphs, &x, &y);
	    glyphs++;
	}
	_trace_printf ("]]");
    } else {
	struct _data_stream stream;

	if (num_glyphs == 1) {
	    _trace_printf ("[%g %g <%02lx>]", x, y,  glyphs->index);
	} else {
	    _trace_printf ("[%g %g <~", x, y);
	    _write_base85_data_start (&stream);
	    while (num_glyphs--) {
		unsigned char c;

		if (fabs (glyphs->x - x) > TOLERANCE ||
		    fabs (glyphs->y - y) > TOLERANCE)
		{
		    x = glyphs->x;
		    y = glyphs->y;
		    _write_base85_data_end (&stream);
		    _trace_printf ("~> %g %g <~", x, y);
		    _write_base85_data_start (&stream);
		}

		c = glyphs->index;
		_write_base85_data (&stream, &c, 1);

		_glyph_advance (font, glyphs, &x, &y);
		glyphs++;
	    }
	    _write_base85_data_end (&stream);
	    _trace_printf ("~>]");
	}
    }
}

void
cairo_show_glyphs (cairo_t *cr, const cairo_glyph_t *glyphs, int num_glyphs)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && glyphs != NULL && _write_lock ()) {
	cairo_scaled_font_t *font;

	_emit_context (cr);
	font = DLCALL (cairo_get_scaled_font, cr);

	_emit_glyphs (font, glyphs, num_glyphs);
	_trace_printf (" show-glyphs\n");
	_write_unlock ();
    }

    DLCALL (cairo_show_glyphs, cr, glyphs, num_glyphs);
    _exit_trace ();
}

static const char *
_direction_to_string (cairo_bool_t backward)
{
    const char *names[] = {
	"FORWARD",
	"BACKWARD"
    };
    return names[!!backward];
}

void
cairo_show_text_glyphs (cairo_t			   *cr,
			const char		   *utf8,
			int			    utf8_len,
			const cairo_glyph_t	   *glyphs,
			int			    num_glyphs,
			const cairo_text_cluster_t *clusters,
			int			    num_clusters,
			cairo_text_cluster_flags_t  backward)
{
    cairo_scaled_font_t *font;

    _enter_trace ();

    font = DLCALL (cairo_get_scaled_font, cr);

    _emit_line_info ();
    if (cr != NULL && glyphs != NULL && clusters != NULL && _write_lock ()) {
	int n;

	_emit_context (cr);

	_emit_string_literal (utf8, utf8_len);

	_emit_glyphs (font, glyphs, num_glyphs);
	_trace_printf ("  [");
	for (n = 0; n < num_clusters; n++) {
	    _trace_printf (" %d %d",
			   clusters[n].num_bytes,
			   clusters[n].num_glyphs);
	}
	_trace_printf (" ] //%s show-text-glyphs\n",
		       _direction_to_string (backward));

	_write_unlock ();
    }

    DLCALL (cairo_show_text_glyphs, cr,
	                            utf8, utf8_len,
				    glyphs, num_glyphs,
				    clusters, num_clusters,
				    backward);
    _exit_trace ();
}

void
cairo_text_path (cairo_t *cr, const char *utf8)
{
    _enter_trace ();
    _emit_line_info ();
    if (cr != NULL && _write_lock ()) {
	_emit_context (cr);
	_emit_string_literal (utf8, -1);
	_trace_printf (" text-path\n");
	_write_unlock ();
    }
    DLCALL (cairo_text_path, cr, utf8);
    _exit_trace ();
}

void
cairo_glyph_path (cairo_t *cr, const cairo_glyph_t *glyphs, int num_glyphs)
{
    cairo_scaled_font_t *font;

    _enter_trace ();

    font = DLCALL (cairo_get_scaled_font, cr);

    _emit_line_info ();
    if (cr != NULL && glyphs != NULL && _write_lock ()) {
	_emit_context (cr);
	_emit_glyphs (font, glyphs, num_glyphs);
	_trace_printf (" glyph-path\n");

	_write_unlock ();
    }

    DLCALL (cairo_glyph_path, cr, glyphs, num_glyphs);
    _exit_trace ();
}

void
cairo_append_path (cairo_t *cr, const cairo_path_t *path)
{
    /* XXX no support for named paths, so manually reconstruct */
    int i;
    cairo_path_data_t *p;

    _enter_trace ();

    _emit_line_info ();
    if (cr == NULL || path == NULL) {
	DLCALL (cairo_append_path, cr, path);
	_exit_trace ();
	return;
    }

    for (i=0; i < path->num_data; i += path->data[i].header.length) {
	p = &path->data[i];
	switch (p->header.type) {
	case CAIRO_PATH_MOVE_TO:
	    if (p->header.length >= 2)
		cairo_move_to (cr, p[1].point.x, p[1].point.y);
	    break;
	case CAIRO_PATH_LINE_TO:
	    if (p->header.length >= 2)
		cairo_line_to (cr, p[1].point.x, p[1].point.y);
	    break;
	case CAIRO_PATH_CURVE_TO:
	    if (p->header.length >= 4)
		cairo_curve_to (cr,
				p[1].point.x, p[1].point.y,
				p[2].point.x, p[2].point.y,
				p[3].point.x, p[3].point.y);
	    break;
	case CAIRO_PATH_CLOSE_PATH:
	    if (p->header.length >= 1)
		cairo_close_path (cr);
	    break;
	default:
	    break;
	}
    }
    _exit_trace ();
}

cairo_surface_t *
cairo_image_surface_create (cairo_format_t format, int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_image_surface_create, format, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);
	const char *format_str = _format_to_string (format);
	const char *content_str = _format_to_content_string (format);

	_trace_printf ("dict\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  /format //%s set\n"
		       "  /content //%s set\n"
		       "  image dup /s%ld exch def\n",
		       width, height, format_str, content_str, obj->token);
	obj->width = width;
	obj->height = height;
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_image_surface_create_for_data (unsigned char *data, cairo_format_t format, int width, int height, int stride)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_image_surface_create_for_data, data, format, width, height, stride);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	/* cairo_image_surface_create_for_data() is both used to supply
	 * foreign pixel data to cairo and in order to read pixels back.
	 * Defer grabbing the pixel contents until we have to, but only for
	 * "large" images, for small images the overhead of embedding pixels
	 * is negligible.
	 *
	 * Choose 32x32 as that captures most icons which thanks to GdkPixbuf
	 * are frequently reloaded.
	 */
	if (width * height < 32*32) {
	    _emit_image (ret, NULL);
	    _trace_printf (" dup /s%ld exch def\n",
			   obj->token);
	} else {
	    _trace_printf ("dict\n"
			   "  /width %d set\n"
			   "  /height %d set\n"
			   "  /format //%s set\n"
			   "  image dup /s%ld exch def\n",
			   width, height,
			   _format_to_string (format),
			   obj->token);

	    obj->foreign = TRUE;
	}

	obj->width  = width;
	obj->height = height;
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

unsigned char *
cairo_image_surface_get_data (cairo_surface_t *surface)
{
    unsigned char *ptr;

    /* Just leave some breadcrumbs */
    _enter_trace ();
    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	_trace_printf ("%% s%ld get-data\n", _get_surface_id (surface));
	_write_unlock ();
    }
    ptr = DLCALL (cairo_image_surface_get_data, surface);
    _exit_trace ();

    return ptr;
}

cairo_pattern_t *
cairo_pattern_create_raster_source (void *data, cairo_content_t content, int width, int height)
{
    cairo_pattern_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_raster_source, data, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	long pattern_id = _create_pattern_id (ret);
	cairo_format_t format;
	cairo_surface_t *image;
	cairo_t *cr;

	/* Impossible to accurately record the interaction with this custom
	 * pattern so just suck all the data into an image upfront */
	switch (content) {
	case CAIRO_CONTENT_ALPHA: format = CAIRO_FORMAT_A8; break;
	case CAIRO_CONTENT_COLOR: format = CAIRO_FORMAT_RGB24; break;
	default:
	case CAIRO_CONTENT_COLOR_ALPHA: format = CAIRO_FORMAT_ARGB32; break;
	}

	_trace_printf ("%% raster-source\n");

	image = DLCALL (cairo_image_surface_create, format, width, height);
	cr = DLCALL (cairo_create, image);
	DLCALL (cairo_set_source, cr, ret);
	DLCALL (cairo_paint, cr);
	DLCALL (cairo_destroy, cr);

	_emit_image (image, NULL);
	DLCALL (cairo_surface_destroy, image);
	_trace_printf (" pattern dup /s%ld exch def\n",
		       pattern_id);

	_push_operand (PATTERN, ret);
	_get_object (PATTERN, ret)->defined = TRUE;
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_surface_create_similar (cairo_surface_t *other,
			      cairo_content_t content,
			      int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_surface_create_similar, other, content, width, height);

    _emit_line_info ();
    if (other != NULL && _write_lock ()) {
	Object *other_obj = _get_object(SURFACE, other);
	Object *new_obj = _create_surface (ret);

	if (other_obj->operand != -1) {
	    if (current_stack_depth == other_obj->operand + 1)
		_trace_printf ("dup ");
	    else
		_trace_printf ("%d index ",
			       current_stack_depth - other_obj->operand - 1);
	} else {
	    assert(other_obj->defined);
	    _trace_printf ("s%ld ", other_obj->token);
	}

	_trace_printf ("%d %d //%s similar dup /s%ld exch def\n",
		       width, height,
		       _content_to_string (content),
		       new_obj->token);

	new_obj->width = width;
	new_obj->height = height;
	new_obj->defined = TRUE;

	_push_object (new_obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_surface_create_similar_image (cairo_surface_t *other,
				    cairo_format_t format,
				    int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_surface_create_similar_image,
		  other, format, width, height);

    _emit_line_info ();
    if (other != NULL && _write_lock ()) {
	Object *other_obj = _get_object(SURFACE, other);
	Object *new_obj = _create_surface (ret);

	if (other_obj->defined)
	    _trace_printf ("s%ld ", other_obj->token);
	else if (current_stack_depth == other_obj->operand + 1)
	    _trace_printf ("dup ");
	else
	    _trace_printf ("%d index ",
			   current_stack_depth - other_obj->operand - 1);
	_trace_printf ("//%s %d %d similar-image %% s%ld\n",
		       _format_to_string (format),
		       width, height,
		       new_obj->token);
	new_obj->width = width;
	new_obj->height = height;

	_push_object (new_obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_surface_map_to_image (cairo_surface_t *surface,
			    const cairo_rectangle_int_t *extents)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_surface_map_to_image, surface, extents);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_emit_surface (surface);
	if (extents) {
	    _trace_printf ("[%d %d %d %d] map-to-image %% s%ld\n",
			   extents->x, extents->y,
			   extents->width, extents->height,
			   obj->token);
	    obj->width  = extents->width;
	    obj->height = extents->height;
	} else {
	    _trace_printf ("[ ] map-to-image %% s%ld\n", obj->token);
	}

	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

void
cairo_surface_unmap_image (cairo_surface_t *surface,
			   cairo_surface_t *image)
{
    _enter_trace ();

    _emit_line_info ();
    if (_write_lock ()) {
	Object *s = _get_object (SURFACE, surface);
	Object *i = _get_object (SURFACE, image);
	if (!(s->operand == current_stack_depth - 2 &&
	      i->operand == current_stack_depth - 1)) {
	    if (i->operand != s->operand + 1 || ! _pop_operands_to_depth (i->operand + 1)) {
		_emit_surface (surface);
		_emit_surface (image);
	    }
	}
	_trace_printf ("unmap-image\n");
	_consume_operand (true);
	_write_unlock ();
    }

    DLCALL (cairo_surface_unmap_image, surface, image);

    _exit_trace ();
}

cairo_surface_t *
cairo_surface_create_for_rectangle (cairo_surface_t *target,
                                    double x, double y,
                                    double width, double height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_surface_create_for_rectangle, target, x, y, width, height);

    _emit_line_info ();
    if (target != NULL && _write_lock ()) {
	Object *target_obj = _get_object (SURFACE, target);
	Object *child_obj = _create_surface (ret);

	if (target_obj->defined)
	    _trace_printf ("s%ld ", target_obj->token);
	else if (current_stack_depth == target_obj->operand + 1)
	    _trace_printf ("dup ");
	else
	    _trace_printf ("%d index ", current_stack_depth - target_obj->operand - 1);
	_trace_printf ("%f %f %f %f subsurface %% s%ld\n",
		       x, y, width, height,
		       child_obj->token);

	_push_object (child_obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

static void CAIRO_PRINTF_FORMAT(2, 3)
_emit_surface_op (cairo_surface_t *surface, const char *fmt, ...)
{
    va_list ap;

    if (surface == NULL || ! _write_lock ())
	return;

    _emit_surface (surface);

    va_start (ap, fmt);
    _trace_vprintf ( fmt, ap);
    va_end (ap);

    _write_unlock ();
}

void
cairo_surface_finish (cairo_surface_t *surface)
{
    _enter_trace ();
    _emit_line_info ();
    DLCALL (cairo_surface_finish, surface);
    _exit_trace ();
}

void
cairo_surface_flush (cairo_surface_t *surface)
{
    _enter_trace ();
    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	_trace_printf ("%% s%ld flush\n", _get_surface_id (surface));
	_write_unlock ();
    }
    DLCALL (cairo_surface_flush, surface);
    _exit_trace ();
}

void
cairo_surface_mark_dirty (cairo_surface_t *surface)
{
    _enter_trace ();
    _emit_line_info ();

    /* Call cairo before emitting the trace since _emit_surface() might cause
     * snapshots to be creates while mark_dirty assert()s that there are none.
     */
    DLCALL (cairo_surface_mark_dirty, surface);

    if (surface != NULL && _write_lock ()) {
	if (_mark_dirty) {
	    _emit_surface (surface);
	    _trace_printf ("%% mark-dirty\n");
	    _emit_source_image (surface);
	} else
	    _trace_printf ("%% s%ld mark-dirty\n", _get_surface_id (surface));
	_write_unlock ();
    }
    _exit_trace ();
}

void
cairo_surface_mark_dirty_rectangle (cairo_surface_t *surface,
				    int x, int y, int width, int height)
{
    _enter_trace ();

    /* Call cairo before emitting the trace since _emit_surface() might cause
     * snapshots to be creates while mark_dirty assert()s that there are none.
     */
    DLCALL (cairo_surface_mark_dirty_rectangle, surface, x, y, width, height);

    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	if (_mark_dirty) {
	    _emit_surface (surface);
	    _trace_printf ("%% %d %d %d %d mark-dirty-rectangle\n",
		            x, y, width, height);
	    _emit_source_image_rectangle (surface, x,y, width, height);
	} else
	    _trace_printf ("%% s%ld %d %d %d %d mark-dirty-rectangle\n",
		           _get_surface_id (surface), x, y, width, height);
	_write_unlock ();
    }
    _exit_trace ();
}

void
cairo_surface_set_device_offset (cairo_surface_t *surface, double x_offset, double y_offset)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_surface_op (surface, "%g %g set-device-offset\n",
		      x_offset, y_offset);
    DLCALL (cairo_surface_set_device_offset, surface, x_offset, y_offset);
    _exit_trace ();
}

void
cairo_surface_set_device_scale (cairo_surface_t *surface, double x_offset, double y_offset)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_surface_op (surface, "%g %g set-device-scale\n",
		      x_offset, y_offset);
    DLCALL (cairo_surface_set_device_scale, surface, x_offset, y_offset);
    _exit_trace ();
}


void
cairo_surface_set_fallback_resolution (cairo_surface_t *surface, double x_pixels_per_inch, double y_pixels_per_inch)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_surface_op (surface, "%g %g set-fallback-resolution\n",
		      x_pixels_per_inch, y_pixels_per_inch);
    DLCALL (cairo_surface_set_fallback_resolution, surface, x_pixels_per_inch, y_pixels_per_inch);
    _exit_trace ();
}

void
cairo_surface_copy_page (cairo_surface_t *surface)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_surface_op (surface, "copy-page\n");
    DLCALL (cairo_surface_copy_page, surface);
    _exit_trace ();
}

void
cairo_surface_show_page (cairo_surface_t *surface)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_surface_op (surface, "show-page\n");
    DLCALL (cairo_surface_show_page, surface);
    _exit_trace ();
}

cairo_status_t
cairo_surface_set_mime_data (cairo_surface_t		*surface,
                             const char			*mime_type,
                             const unsigned char	*data,
                             unsigned long		 length,
			     cairo_destroy_func_t	 destroy,
			     void			*closure)
{
    cairo_status_t ret;
    _enter_trace ();
    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	_emit_surface (surface);
	_emit_string_literal (mime_type, -1);
	_trace_printf (" ");
	_emit_data (data, length);
	_trace_printf (" /deflate filter set-mime-data\n");

	_write_unlock ();
    }

    ret = DLCALL (cairo_surface_set_mime_data,
		  surface,
		  mime_type,
		  data, length,
		  destroy,
		  closure);
    _exit_trace ();
    return ret;
}

#if CAIRO_HAS_PNG_FUNCTIONS
cairo_status_t
cairo_surface_write_to_png (cairo_surface_t *surface, const char *filename)
{
    cairo_status_t ret;
    _enter_trace ();
    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	_trace_printf ("%% s%ld ", _get_surface_id (surface));
	_emit_string_literal (filename, -1);
	_trace_printf (" write-to-png pop\n");
	_write_unlock ();
    }
    ret = DLCALL (cairo_surface_write_to_png, surface, filename);
    _exit_trace ();
    return ret;
}

cairo_status_t
cairo_surface_write_to_png_stream (cairo_surface_t *surface,
				   cairo_write_func_t write_func,
				   void *data)
{
    cairo_status_t ret;
    _enter_trace ();
    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	char symbol[1024];

	_trace_printf ("%% s%ld ", _get_surface_id (surface));
#if CAIRO_HAS_SYMBOL_LOOKUP
	lookup_symbol (symbol, sizeof (symbol), write_func);
#else
	symbol[0] = '\0';
#endif
	_emit_string_literal (symbol, -1);
	_trace_printf (" write-to-png-stream pop\n");
	_write_unlock ();
    }
    ret = DLCALL (cairo_surface_write_to_png_stream,
		  surface, write_func, data);
    _exit_trace ();
    return ret;
}
#endif

static void CAIRO_PRINTF_FORMAT(2, 3)
_emit_pattern_op (cairo_pattern_t *pattern, const char *fmt, ...)
{
    va_list ap;

    if (pattern == NULL || ! _write_lock ())
	return;

    _emit_pattern (pattern);

    va_start (ap, fmt);
    _trace_vprintf (fmt, ap);
    va_end (ap);

    _write_unlock ();
}

cairo_pattern_t *
cairo_pattern_create_rgb (double red, double green, double blue)
{
    cairo_pattern_t *ret;
    long pattern_id;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_rgb, red, green, blue);
    pattern_id = _create_pattern_id (ret);

    _emit_line_info ();
    if (_write_lock ()) {
	_trace_printf ("/p%ld %g %g %g rgb def\n",
		       pattern_id, red, green, blue);
	_get_object (PATTERN, ret)->defined = TRUE;
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_pattern_t *
cairo_pattern_create_rgba (double red, double green, double blue, double alpha)
{
    cairo_pattern_t *ret;
    long pattern_id;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_rgba, red, green, blue, alpha);
    pattern_id = _create_pattern_id (ret);

    _emit_line_info ();
    if (_write_lock ()) {
	_trace_printf ("/p%ld %g %g %g %g rgba def\n",
		       pattern_id, red, green, blue, alpha);
	_get_object (PATTERN, ret)->defined = TRUE;
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_pattern_t *
cairo_pattern_create_for_surface (cairo_surface_t *surface)
{
    cairo_pattern_t *ret;
    long pattern_id;
    long surface_id;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_for_surface, surface);
    pattern_id = _create_pattern_id (ret);

    _emit_line_info ();
    if (surface != NULL && _write_lock ()) {
	surface_id = _get_surface_id (surface);

	if (_pop_operands_to (SURFACE, surface)) {
	    _consume_operand (false);
	} else {
	    _trace_printf ("s%ld ", surface_id);
	}

	if (_get_object (SURFACE, surface)->foreign)
	    _emit_source_image (surface);

	_trace_printf ("pattern %% p%ld\n", pattern_id);
	_push_operand (PATTERN, ret);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_pattern_t *
cairo_pattern_create_linear (double x0, double y0, double x1, double y1)
{
    cairo_pattern_t *ret;
    long pattern_id;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_linear, x0, y0, x1, y1);
    pattern_id = _create_pattern_id (ret);

    _emit_line_info ();
    if (_write_lock ()) {
	_trace_printf ("%g %g %g %g linear %% p%ld\n",
		       x0, y0, x1, y1, pattern_id);
	_push_operand (PATTERN, ret);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_pattern_t *
cairo_pattern_create_radial (double cx0, double cy0, double radius0, double cx1, double cy1, double radius1)
{
    cairo_pattern_t *ret;
    long pattern_id;

    _enter_trace ();

    ret = DLCALL (cairo_pattern_create_radial,
		  cx0, cy0, radius0,
		  cx1, cy1, radius1);
    pattern_id = _create_pattern_id (ret);

    _emit_line_info ();
    if (_write_lock ()) {
	_trace_printf ("%g %g %g %g %g %g radial %% p%ld\n",
		       cx0, cy0, radius0, cx1, cy1, radius1,
		       pattern_id);
	_push_operand (PATTERN, ret);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

void
cairo_pattern_add_color_stop_rgb (cairo_pattern_t *pattern, double offset, double red, double green, double blue)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_pattern_op (pattern,
		      "%g %g %g %g 1 add-color-stop\n",
		      offset, red, green, blue);
    DLCALL (cairo_pattern_add_color_stop_rgb, pattern, offset, red, green, blue);
    _exit_trace ();
}

void
cairo_pattern_add_color_stop_rgba (cairo_pattern_t *pattern, double offset, double red, double green, double blue, double alpha)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_pattern_op (pattern,
		      "%g %g %g %g %g add-color-stop\n",
		      offset, red, green, blue, alpha);
    DLCALL (cairo_pattern_add_color_stop_rgba, pattern, offset, red, green, blue, alpha);
    _exit_trace ();
}

void
cairo_pattern_set_matrix (cairo_pattern_t *pattern, const cairo_matrix_t *matrix)
{
    _enter_trace ();
    _emit_line_info ();
    if (_matrix_is_identity (matrix)) {
	_emit_pattern_op (pattern, "identity set-matrix\n");
    } else {
	_emit_pattern_op (pattern,
			  "%g %g %g %g %g %g matrix set-matrix\n",
			  matrix->xx, matrix->yx,
			  matrix->xy, matrix->yy,
			  matrix->x0, matrix->y0);
    }
    DLCALL (cairo_pattern_set_matrix, pattern, matrix);
    _exit_trace ();
}

static const char *
_filter_to_string (cairo_filter_t filter)
{
#define f(name) case CAIRO_FILTER_ ## name: return "FILTER_" #name
    switch (filter) {
	f(FAST);
	f(GOOD);
	f(BEST);
	f(NEAREST);
	f(BILINEAR);
	f(GAUSSIAN);
    };
#undef f
    return "UNKNOWN_FILTER";
}

void
cairo_pattern_set_filter (cairo_pattern_t *pattern, cairo_filter_t filter)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_pattern_op (pattern, "//%s set-filter\n", _filter_to_string (filter));
    DLCALL (cairo_pattern_set_filter, pattern, filter);
    _exit_trace ();
}

static const char *
_extend_to_string (cairo_extend_t extend)
{
#define f(name) case CAIRO_EXTEND_ ## name: return "EXTEND_" #name
    switch (extend) {
	f(NONE);
	f(REPEAT);
	f(REFLECT);
	f(PAD);
    };
#undef f
    return "UNKNOWN_EXTEND";
}

void
cairo_pattern_set_extend (cairo_pattern_t *pattern, cairo_extend_t extend)
{
    _enter_trace ();
    _emit_line_info ();
    _emit_pattern_op (pattern, "//%s set-extend\n", _extend_to_string (extend));
    DLCALL (cairo_pattern_set_extend, pattern, extend);
    _exit_trace ();
}

#if CAIRO_HAS_FT_FONT
#if CAIRO_HAS_FC_FONT
cairo_font_face_t *
cairo_ft_font_face_create_for_pattern (FcPattern *pattern)
{
    cairo_font_face_t *ret;
    long font_face_id;

    _enter_trace ();

    ret = DLCALL (cairo_ft_font_face_create_for_pattern, pattern);
    font_face_id = _create_font_face_id (ret);

    _emit_line_info ();
    if (pattern != NULL && _write_lock ()) {
	Object *obj;

	obj = _get_object (FONT_FACE, ret);
	if (obj->unknown) {
		FcPattern *copy;
		FcChar8 *unparsed;

		copy = DLCALL (FcPatternDuplicate, pattern);
		if (copy)
		{
			DLCALL (FcPatternDel, copy, FC_LANG);
			DLCALL (FcPatternDel, copy, FC_CHARSET);
			DLCALL (FcPatternDel, copy, FC_CAPABILITY);
		}
		else
			copy = pattern;

		unparsed = DLCALL (FcNameUnparse, copy);
		_trace_printf ("dict\n"
			       "  /type 42 set\n"
			       "  /pattern ");
		_emit_string_literal ((char *) unparsed, -1);
		_trace_printf (" set\n"
			       "  font %% f%ld\n",
			       font_face_id);
		obj->unknown = FALSE;
		_push_operand (FONT_FACE, ret);
		dump_stack(__func__);

		if (copy != pattern)
			DLCALL (FcPatternDestroy, copy);
		free (unparsed);
	}
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}
#endif /* CAIRO_HAS_FC_FONT*/

typedef struct _ft_face_data {
    unsigned long index;
    unsigned long size;
    void *data;
} FtFaceData;

static void
_ft_face_data_destroy (void *arg)
{
    FtFaceData *data = arg;
    free (data->data);
    free (data);
}

cairo_font_face_t *
cairo_ft_font_face_create_for_ft_face (FT_Face face, int load_flags)
{
    cairo_font_face_t *ret;
    Object *obj;
    FtFaceData *data;
    long font_face_id;

    _enter_trace ();

    ret = DLCALL (cairo_ft_font_face_create_for_ft_face, face, load_flags);
    font_face_id = _create_font_face_id (ret);

    if (face == NULL) {
	_exit_trace ();
	return ret;
    }

    obj = _get_object (NONE, face);
    data = obj->data;
    if (data == NULL) {
	_exit_trace ();
	return ret;
    }

    _emit_line_info ();
    if (_write_lock ()) {
	obj = _get_object (FONT_FACE, ret);
	if (obj->operand != -1)
	    _object_remove (obj);

	_trace_printf ("<< /type 42 /source ");
	_emit_data (data->data, data->size);
	_trace_printf (" /index %lu /flags %d >> font %% f%ld\n",
		       data->index, load_flags, font_face_id);
	_push_operand (FONT_FACE, ret);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

static cairo_bool_t
_ft_read_file (FtFaceData *data, const char *path)
{
    char buf[8192];
    FILE *file;

    file = fopen (path, "rb");
    if (file != NULL) {
	size_t ret;
	unsigned long int allocated = sizeof (buf);
	data->data = malloc (allocated);
	do {
	    ret = fread (buf, 1, sizeof (buf), file);
	    if (ret == 0)
		break;
	    memcpy ((char *) data->data + data->size, buf, ret);
	    data->size += ret;
	    if (ret != sizeof (buf))
		break;

	    if (data->size == allocated) {
		allocated *= 2;
		data->data = realloc (data->data, allocated);
	    }
	} while (TRUE);
	fclose (file);
    }

    return file != NULL;
}

FT_Error
FT_New_Face (FT_Library library, const char *pathname, FT_Long index, FT_Face *face)
{
    FT_Error ret;

    _enter_trace ();

    ret = DLCALL (FT_New_Face, library, pathname, index, face);
    if (ret == 0) {
	Object *obj = _type_object_create (NONE, *face);
	FtFaceData *data = malloc (sizeof (FtFaceData));
	data->index = index;
	data->size = 0;
	data->data = NULL;
	_ft_read_file (data, pathname);
	obj->data = data;
	obj->destroy = _ft_face_data_destroy;
    }

    _exit_trace ();
    return ret;
}

FT_Error
FT_New_Memory_Face (FT_Library library, const FT_Byte *mem, FT_Long size, FT_Long index, FT_Face *face)
{
    FT_Error ret;

    _enter_trace ();

    ret = DLCALL (FT_New_Memory_Face, library, mem, size, index, face);
    if (ret == 0) {
	Object *obj = _type_object_create (NONE, *face);
	FtFaceData *data = malloc (sizeof (FtFaceData));
	data->index = index;
	data->size = size;
	data->data = malloc (size);
	memcpy (data->data, mem, size);
	obj->data = data;
	obj->destroy = _ft_face_data_destroy;
    }

    _exit_trace ();
    return ret;
}

/* XXX
 * FT_New_Memory_Face() and FT_New_Face() appear to wrap FT_Open_Face() so we
 * get a redundant call to FT_Open_Face() from those paths (no PLT hiding
 * within FT, naughty library!) but we do not intercept a direct call to
 * FT_Open_Face(). So far this has not caused any issues, but it will one
 * day...
 */
FT_Error
FT_Open_Face (FT_Library library, const FT_Open_Args *args, FT_Long index, FT_Face *face)
{
    FT_Error ret;

    _enter_trace ();

    ret = DLCALL (FT_Open_Face, library, args, index, face);
    if (ret == 0) {
	Object *obj = _get_object (NONE, *face);
	if (obj == NULL) {
	    FtFaceData *data;

	    data = malloc (sizeof (FtFaceData));
	    data->index = index;
	    if (args->flags & FT_OPEN_MEMORY) {
		data->size = args->memory_size;
		data->data = malloc (args->memory_size);
		memcpy (data->data, args->memory_base, args->memory_size);
	    } else if (args->flags & FT_OPEN_STREAM) {
		fprintf (stderr, "FT_Open_Face (stream, %ld) = %p\n",
			 index, *face);
		abort ();
	    } else if (args->flags & FT_OPEN_PATHNAME) {
		data->size = 0;
		data->data = NULL;
		_ft_read_file (data, args->pathname);
	    }

	    obj = _type_object_create (NONE, *face);
	    obj->data = data;
	    obj->destroy = _ft_face_data_destroy;
	}
    }

    _exit_trace ();
    return ret;
}

FT_Error
FT_Done_Face (FT_Face face)
{
    FT_Error ret;
    _enter_trace ();

    _object_destroy (_get_object (NONE, face));

    ret = DLCALL (FT_Done_Face, face);
    _exit_trace ();
    return ret;
}
#endif

static void
_surface_object_set_size (cairo_surface_t *surface, int width, int height)
{
    Object *obj;

    obj = _get_object (SURFACE, surface);
    obj->width = width;
    obj->height = height;
}

static void
_surface_object_set_size_from_surface (cairo_surface_t *surface)
{
    _surface_object_set_size (surface,
			      DLCALL (cairo_image_surface_get_width, surface),
			      DLCALL (cairo_image_surface_get_height, surface));
}

#if CAIRO_HAS_PS_SURFACE
#include<cairo-ps.h>

cairo_surface_t *
cairo_ps_surface_create (const char *filename, double width_in_points, double height_in_points)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_ps_surface_create, filename, width_in_points, height_in_points);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /PS set\n"
		       "  /filename ");
	_emit_string_literal (filename, -1);
	_trace_printf (" set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width_in_points,
		       height_in_points,
		       obj->token);
	obj->width = width_in_points;
	obj->height = height_in_points;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_ps_surface_create_for_stream (cairo_write_func_t write_func, void *closure, double width_in_points, double height_in_points)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_ps_surface_create_for_stream, write_func, closure, width_in_points, height_in_points);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /PS set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width_in_points,
		       height_in_points,
		       obj->token);
	obj->width = width_in_points;
	obj->height = height_in_points;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

void
cairo_ps_surface_set_size (cairo_surface_t *surface, double width_in_points, double height_in_points)
{
    _enter_trace ();
    _emit_line_info ();
    DLCALL (cairo_ps_surface_set_size, surface, width_in_points, height_in_points);
    _exit_trace ();
}

#endif

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>

cairo_surface_t *
cairo_pdf_surface_create (const char *filename, double width_in_points, double height_in_points)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_pdf_surface_create, filename, width_in_points, height_in_points);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /PDF set\n"
		       "  /filename ");
	_emit_string_literal (filename, -1);
	_trace_printf (" set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width_in_points,
		       height_in_points,
		       obj->token);
	obj->width = width_in_points;
	obj->height = height_in_points;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_pdf_surface_create_for_stream (cairo_write_func_t write_func, void *closure, double width_in_points, double height_in_points)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_pdf_surface_create_for_stream, write_func, closure, width_in_points, height_in_points);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /PDF set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width_in_points,
		       height_in_points,
		       obj->token);
	obj->width = width_in_points;
	obj->height = height_in_points;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }
    _exit_trace ();
    return ret;
}

void
cairo_pdf_surface_set_size (cairo_surface_t *surface, double width_in_points, double height_in_points)
{
    _enter_trace ();
    _emit_line_info ();
    DLCALL (cairo_pdf_surface_set_size, surface, width_in_points, height_in_points);
    _exit_trace ();
}
#endif

#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>

cairo_surface_t *
cairo_svg_surface_create (const char *filename, double width, double height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_svg_surface_create, filename, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /SVG set\n"
		       "  /filename ");
	_emit_string_literal (filename, -1);
	_trace_printf (" set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width,
		       height,
		       obj->token);
	obj->width = width;
	obj->height = height;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_svg_surface_create_for_stream (cairo_write_func_t write_func, void *closure, double width, double height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_svg_surface_create_for_stream, write_func, closure, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /SVG set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface %% s%ld\n",
		       width,
		       height,
		       obj->token);
	obj->width = width;
	obj->height = height;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

#endif

#if CAIRO_HAS_PNG_FUNCTIONS
cairo_surface_t *
cairo_image_surface_create_from_png (const char *filename)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_image_surface_create_from_png, filename);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);
	char filename_string[4096];

	_encode_string_literal (filename_string, sizeof (filename_string),
				filename, -1);
	_emit_image (ret, "  /filename %s set\n", filename_string);
	_trace_printf (" dup /s%ld exch def\n", obj->token);
	_surface_object_set_size_from_surface (ret);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_image_surface_create_from_png_stream (cairo_read_func_t read_func, void *closure)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_image_surface_create_from_png_stream, read_func, closure);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_emit_image (ret, NULL);
	_trace_printf (" dup /s%ld exch def\n",
		       obj->token);

	_surface_object_set_size_from_surface (ret);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}
#endif

#if CAIRO_HAS_TEE_SURFACE
#include <cairo-tee.h>

cairo_surface_t *
cairo_tee_surface_create (cairo_surface_t *master)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_tee_surface_create, master);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /tee set\n"
		       "  /master s%ld set\n"
		       "  surface dup /s%ld exch def\n",
		       _get_object (SURFACE, master)->token,
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

#endif

#if CAIRO_HAS_XLIB_SURFACE
#include <cairo-xlib.h>

static const char *
_content_from_surface (cairo_surface_t *surface)
{
    return _content_to_string (DLCALL (cairo_surface_get_content, surface));
}

cairo_surface_t *
cairo_xlib_surface_create (Display *dpy,
			   Drawable drawable,
			   Visual *visual,
			   int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_xlib_surface_create,
	          dpy, drawable, visual, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /xlib set\n"
		       "  /drawable 16!%lx set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  surface dup /s%ld exch def\n",
		       drawable,
		       _content_from_surface (ret),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	obj->width = width;
	obj->height = height;
	obj->foreign = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_xlib_surface_create_for_bitmap (Display *dpy,
				      Pixmap bitmap,
				      Screen *screen,
				      int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_xlib_surface_create_for_bitmap,
	          dpy, bitmap, screen, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /xlib set\n"
		       "  /drawable 16!%lx set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  /depth 1 set\n"
		       "  surface dup /s%ld exch def\n",
		       bitmap,
		       _content_from_surface (ret),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	obj->width = width;
	obj->height = height;
	obj->foreign = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

#if CAIRO_HAS_XLIB_XRENDER_SURFACE
#include <cairo-xlib-xrender.h>
cairo_surface_t *
cairo_xlib_surface_create_with_xrender_format (Display *dpy,
					       Drawable drawable,
					       Screen *screen,
					       XRenderPictFormat *format,
					       int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_xlib_surface_create_with_xrender_format,
	          dpy, drawable, screen, format, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /xrender set\n"
		       "  /drawable 16!%lx set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  /depth %d set\n"
		       "  surface dup /s%ld exch def\n",
		       drawable,
		       _content_from_surface (ret),
		       width, height,
		       format->depth,
		       obj->token);
	obj->defined = TRUE;
	obj->width = width;
	obj->height = height;
	obj->foreign = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}
#endif
#endif

#if CAIRO_HAS_SCRIPT_SURFACE
#include <cairo-script.h>
cairo_surface_t *
cairo_script_surface_create (cairo_device_t *device,
			     cairo_content_t content,
			     double width,
			     double height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_script_surface_create, device, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /script set\n"
		       "  /content %s set\n"
		       "  /width %g set\n"
		       "  /height %g set\n"
		       "  surface dup /s%ld exch def\n",
		       _content_to_string (content),
		       width, height,
		       obj->token);
	obj->width = width;
	obj->height = height;
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
cairo_script_surface_create_for_target (cairo_device_t *device,
					cairo_surface_t *target)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_script_surface_create_for_target, device, target);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /script set\n"
		       "  surface dup /s%ld exch def\n",
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}
#endif

#if CAIRO_HAS_TEST_SURFACES
#include <test-paginated-surface.h>
cairo_surface_t *
_cairo_test_paginated_surface_create (cairo_surface_t *surface)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (_cairo_test_paginated_surface_create, surface);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	/* XXX store initial data? */
	_trace_printf ("dict\n"
		       "  /type /test-paginated set\n"
		       "  /target s%ld set\n"
		       "  surface dup /s%ld exch def\n",
		       _get_surface_id (surface),
		       obj->token);
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

#include <test-compositor-surface.h>

cairo_surface_t *
_cairo_test_fallback_compositor_surface_create (cairo_content_t content, int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (_cairo_test_fallback_compositor_surface_create, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /test-fallback-compositor set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  surface dup /s%ld exch def\n",
		       _content_to_string (content),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
_cairo_test_mask_compositor_surface_create (cairo_content_t content, int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (_cairo_test_mask_compositor_surface_create, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /test-mask-compositor set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  surface dup /s%ld exch def\n",
		       _content_to_string (content),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
_cairo_test_spans_compositor_surface_create (cairo_content_t content, int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (_cairo_test_spans_compositor_surface_create, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /test-spans-compositor set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  surface dup /s%ld exch def\n",
		       _content_to_string (content),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

cairo_surface_t *
_cairo_test_traps_compositor_surface_create (cairo_content_t content, int width, int height)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (_cairo_test_traps_compositor_surface_create, content, width, height);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	_trace_printf ("dict\n"
		       "  /type /test-traps-compositor set\n"
		       "  /content //%s set\n"
		       "  /width %d set\n"
		       "  /height %d set\n"
		       "  surface dup /s%ld exch def\n",
		       _content_to_string (content),
		       width, height,
		       obj->token);
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}

#endif

cairo_surface_t *
cairo_recording_surface_create (cairo_content_t content,
				const cairo_rectangle_t *extents)
{
    cairo_surface_t *ret;

    _enter_trace ();

    ret = DLCALL (cairo_recording_surface_create, content, extents);

    _emit_line_info ();
    if (_write_lock ()) {
	Object *obj = _create_surface (ret);

	if (extents) {
	    _trace_printf ("//%s [ %f %f %f %f ] record dup /s%ld exch def\n",
			   _content_to_string (content),
			   extents->x, extents->y,
			   extents->width, extents->height,
			   obj->token);
	    obj->width = extents->width;
	    obj->height = extents->height;
	} else {
	    _trace_printf ("//%s [ ] record dup /s%ld exch def\n",
			   _content_to_string (content),
			   obj->token);
	}
	obj->defined = TRUE;
	_push_object (obj);
	dump_stack(__func__);
	_write_unlock ();
    }

    _exit_trace ();
    return ret;
}
