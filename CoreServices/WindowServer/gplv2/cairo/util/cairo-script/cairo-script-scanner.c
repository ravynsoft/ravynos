/*
 * Copyright © 2008 Chris Wilson <chris@chris-wilson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson.
 *
 * Contributor(s):
 *	Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include "cairo-script-private.h"

#include <limits.h> /* INT_MAX */
#include <math.h> /* pow */
#include <stdio.h> /* EOF */
#include <stdint.h> /* for {INT,UINT}*_{MIN,MAX} */
#include <stdlib.h> /* malloc/free */
#include <string.h> /* memset */
#include <assert.h>
#include <zlib.h>

#if HAVE_LZO
#include <lzo/lzo2a.h>
#endif

#define DEBUG_SCAN 0

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

/*
 * whitespace:
 * 0 - nul
 * 9 - tab
 * A - LF
 * C - FF
 * D - CR
 *
 * syntax delimiters
 * ( = 28, ) = 29 - literal strings
 * < = 3C, > = 3E - hex/base85 strings, dictionary name
 * [ = 5B, ] = 5D - array
 * { = 7B, } = 7C - procedure
 * / = 5C - literal marker
 * % = 25 - comment
 */

static void
fprintf_obj (FILE *stream, csi_t *ctx, const csi_object_t *obj)
{
    switch (csi_object_get_type (obj)) {
	case CSI_OBJECT_TYPE_NULL:
	    fprintf (stream, "NULL\n");
	    break;

	    /* atomics */
	case CSI_OBJECT_TYPE_BOOLEAN:
	    fprintf (stream, "boolean: %s\n",
		    obj->datum.boolean ? "true" : "false");
	    break;
	case CSI_OBJECT_TYPE_INTEGER:
	    fprintf (stream, "integer: %ld\n", obj->datum.integer);
	    break;
	case CSI_OBJECT_TYPE_MARK:
	    fprintf (stream, "mark\n");
	    break;
	case CSI_OBJECT_TYPE_NAME:
	    fprintf (stream, "name: %s\n", (char *) obj->datum.name);
	    break;
	case CSI_OBJECT_TYPE_OPERATOR:
	    fprintf (stream, "operator: %p\n", obj->datum.ptr);
	    break;
	case CSI_OBJECT_TYPE_REAL:
	    fprintf (stream, "real: %g\n", obj->datum.real);
	    break;

	    /* compound */
	case CSI_OBJECT_TYPE_ARRAY:
	    fprintf (stream, "array\n");
	    break;
	case CSI_OBJECT_TYPE_DICTIONARY:
	    fprintf (stream, "dictionary\n");
	    break;
	case CSI_OBJECT_TYPE_FILE:
	    fprintf (stream, "file\n");
	    break;
	case CSI_OBJECT_TYPE_MATRIX:
	    fprintf (stream, "matrix: [%g %g %g %g %g %g]\n",
		    obj->datum.matrix->matrix.xx,
		    obj->datum.matrix->matrix.yx,
		    obj->datum.matrix->matrix.xy,
		    obj->datum.matrix->matrix.yy,
		    obj->datum.matrix->matrix.x0,
		    obj->datum.matrix->matrix.y0);
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    fprintf (stream, "string: len=%ld, defate=%ld, method=%d\n",
		     obj->datum.string->len, obj->datum.string->deflate, obj->datum.string->method);
	    break;

	    /* cairo */
	case CSI_OBJECT_TYPE_CONTEXT:
	    fprintf (stream, "context\n");
	    break;
	case CSI_OBJECT_TYPE_FONT:
	    fprintf (stream, "font\n");
	    break;
	case CSI_OBJECT_TYPE_PATTERN:
	    fprintf (stream, "pattern\n");
	    break;
	case CSI_OBJECT_TYPE_SCALED_FONT:
	    fprintf (stream, "scaled-font\n");
	    break;
	case CSI_OBJECT_TYPE_SURFACE:
	    fprintf (stream, "surface\n");
	    break;
    }
}

/* takes ownership of obj */
static inline csi_status_t
scan_push (csi_t *ctx, csi_object_t *obj)
{
    return ctx->scanner.push (ctx, obj);
}

static inline csi_status_t
scan_execute (csi_t *ctx, csi_object_t *obj)
{
    return ctx->scanner.execute (ctx, obj);
}

static cairo_status_t
buffer_init (csi_t *ctx, csi_buffer_t *buffer)
{
    cairo_status_t status = CSI_STATUS_SUCCESS;

    buffer->size = 16384;
    buffer->base = _csi_alloc (ctx, buffer->size);
    if (_csi_unlikely (buffer->base == NULL)) {
	status = _csi_error (CSI_STATUS_NO_MEMORY);
	buffer->size = 0;
    }

    buffer->ptr = buffer->base;
    buffer->end = buffer->base + buffer->size;

    return status;
}

static void
buffer_fini (csi_t *ctx, csi_buffer_t *buffer)
{
    _csi_free (ctx, buffer->base);
}

static void
_buffer_grow (csi_t *ctx, csi_scanner_t *scan)
{
    int newsize;
    int offset;
    char *base;

    if (_csi_unlikely (scan->buffer.size > INT_MAX / 2))
	longjmp (scan->jump_buffer,  _csi_error (CSI_STATUS_NO_MEMORY));

    offset = scan->buffer.ptr - scan->buffer.base;
    newsize = scan->buffer.size * 2;
    base = _csi_realloc (ctx, scan->buffer.base, newsize);
    if (_csi_unlikely (base == NULL))
	longjmp (scan->jump_buffer,  _csi_error (CSI_STATUS_NO_MEMORY));

    scan->buffer.base = base;
    scan->buffer.ptr  = base + offset;
    scan->buffer.end  = base + newsize;
    scan->buffer.size = newsize;
}

static inline void
buffer_check (csi_t *ctx, csi_scanner_t *scan, int count)
{
    if (_csi_unlikely (scan->buffer.ptr + count > scan->buffer.end))
	_buffer_grow (ctx, scan);
}

static inline void
buffer_add (csi_buffer_t *buffer, int c)
{
    *buffer->ptr++ = c;
}

static inline void
buffer_reset (csi_buffer_t *buffer)
{
    buffer->ptr = buffer->base;
}

static void
token_start (csi_scanner_t *scan)
{
    buffer_reset (&scan->buffer);
}

static void
token_add (csi_t *ctx, csi_scanner_t *scan, int c)
{
    buffer_check (ctx, scan, 1);
    buffer_add (&scan->buffer, c);
}

static void
token_add_unchecked (csi_scanner_t *scan, int c)
{
    buffer_add (&scan->buffer, c);
}

csi_boolean_t
_csi_parse_number (csi_object_t *obj, const char *s, int len)
{
    int radix = 0;
    long long mantissa = 0;
    int exponent = 0;
    int sign = 1;
    int decimal = -1;
    int exponent_sign = 0;
    const char * const end = s + len;

    switch (*s) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	mantissa = *s - '0';
    case '+':
	break;
    case '-':
	sign = -1;
	break;
    case '.':
	decimal = 0;
	break;
    default:
	return FALSE;
    }

    while (++s < end) {
	if (*s < '0') {
	    if (*s == '.') {
		if (_csi_unlikely (radix))
		    return FALSE;
		if (_csi_unlikely (decimal != -1))
		    return FALSE;
		if (_csi_unlikely (exponent_sign))
		    return FALSE;

		decimal = 0;
	    } else if (*s == '!') {
		if (_csi_unlikely (radix))
		    return FALSE;
		if (_csi_unlikely (decimal != -1))
		    return FALSE;
		if (_csi_unlikely (exponent_sign))
		    return FALSE;

		radix = mantissa;
		mantissa = 0;

		if (_csi_unlikely (radix < 2 || radix > 36))
		    return FALSE;
	    } else
		return FALSE;
	} else if (*s <= '9') {
	    int v = *s - '0';
	    if (_csi_unlikely (radix && v >= radix))
		return FALSE;

	    if (exponent_sign) {
		exponent = 10 * exponent + v;
	    } else {
		if (radix)
		    mantissa = radix * mantissa + v;
		else
		    mantissa = 10 * mantissa + v;
		if (decimal != -1)
		    decimal++;
	    }
	} else if (*s == 'E' || * s== 'e') {
	    if (radix == 0) {
		if (_csi_unlikely (s + 1 == end))
		    return FALSE;

		exponent_sign = 1;
		if (s[1] == '-') {
		    exponent_sign = -1;
		    s++;
		} else if (s[1] == '+')
		    s++;
	    } else {
		int v = 0xe;

		if (_csi_unlikely (v >= radix))
		    return FALSE;

		mantissa = radix * mantissa + v;
	    }
	} else if (*s < 'A') {
	    return FALSE;
	} else if (*s <= 'Z') {
	    int v = *s - 'A' + 0xA;

	    if (_csi_unlikely (v >= radix))
		return FALSE;

	    mantissa = radix * mantissa + v;
	} else if (*s < 'a') {
	    return FALSE;
	} else if (*s <= 'z') {
	    int v = *s - 'a' + 0xa;

	    if (_csi_unlikely (v >= radix))
		return FALSE;

	    mantissa = radix * mantissa + v;
	} else
	    return FALSE;
    }

    if (exponent_sign || decimal != -1) {
	if (mantissa == 0) {
	    obj->type = CSI_OBJECT_TYPE_REAL;
	    obj->datum.real = 0.;
	    return TRUE;
	} else {
	    int e;
	    double v;

	    v = mantissa;
	    e = exponent * exponent_sign;
	    if (decimal != -1)
		e -= decimal;
	    switch (e) {
	    case -7: v *= 0.0000001; break;
	    case -6: v *= 0.000001; break;
	    case -5: v *= 0.00001; break;
	    case -4: v *= 0.0001; break;
	    case -3: v *= 0.001; break;
	    case -2: v *= 0.01; break;
	    case -1: v *= 0.1; break;
	    case  0: break;
	    case  1: v *= 10; break;
	    case  2: v *= 100; break;
	    case  3: v *= 1000; break;
	    case  4: v *= 10000; break;
	    case  5: v *= 100000; break;
	    case  6: v *= 1000000; break;
	    default:
		    v *= pow (10, e); /* XXX */
		    break;
	    }

	    obj->type = CSI_OBJECT_TYPE_REAL;
	    obj->datum.real = sign * v;
	    return TRUE;
	}
    } else {
	obj->type = CSI_OBJECT_TYPE_INTEGER;
	obj->datum.integer = sign * mantissa;
	return TRUE;
    }
}

static void
token_end (csi_t *ctx, csi_scanner_t *scan, csi_file_t *src)
{
    cairo_status_t status;
    char *s;
    csi_object_t obj;
    int len;

    /*
     * Any token that consists entirely of regular characters and
     * cannot be interpreted as a number is treated as a name object
     * (more precisely, an executable name). All characters except
     * delimiters and white-space characters can appear in names,
     * including characters ordinarily considered to be punctuation.
     */

    if (_csi_unlikely (scan->buffer.ptr == scan->buffer.base))
	return;

    s = scan->buffer.base;
    len = scan->buffer.ptr - scan->buffer.base;

    if (_csi_likely (! scan->bind)) {
	if (s[0] == '{') { /* special case procedures */
	    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL) {
		status = _csi_stack_push (ctx,
					  &scan->procedure_stack,
					  &scan->build_procedure);
		if (_csi_unlikely (status))
		    longjmp (scan->jump_buffer, status);
	    }

	    status = csi_array_new (ctx, 0, &scan->build_procedure);
	    if (_csi_unlikely (status))
		longjmp (scan->jump_buffer, status);

	    scan->build_procedure.type |= CSI_OBJECT_ATTR_EXECUTABLE;
	    return;
	} else if (s[0] == '}') {
	    if (_csi_unlikely
		(scan->build_procedure.type == CSI_OBJECT_TYPE_NULL))
	    {
		longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));
	    }

	    if (scan->procedure_stack.len) {
		csi_object_t *next;

		next = _csi_stack_peek (&scan->procedure_stack, 0);
		status = csi_array_append (ctx, next->datum.array,
					   &scan->build_procedure);
		scan->build_procedure = *next;
		scan->procedure_stack.len--;
	    } else {
		status = scan_push (ctx, &scan->build_procedure);
		scan->build_procedure.type = CSI_OBJECT_TYPE_NULL;
	    }
	    if (_csi_unlikely (status))
		longjmp (scan->jump_buffer, status);

	    return;
	}
    }

    if (s[0] == '/') {
	if (len >= 2 && s[1] == '/') { /* substituted name */
	    status = csi_name_new (ctx, &obj, s + 2, len - 2);
	    if (_csi_unlikely (status))
		longjmp (scan->jump_buffer, status);

	    status = _csi_name_lookup (ctx, obj.datum.name, &obj);
	} else { /* literal name */
	    status = csi_name_new (ctx, &obj, s + 1, len - 1);
	}
	if (_csi_unlikely (status))
	    longjmp (scan->jump_buffer, status);
    } else {
	if (! _csi_parse_number (&obj, s, len)) {
	    status = csi_name_new (ctx, &obj, s, len);
	    if (_csi_unlikely (status))
		longjmp (scan->jump_buffer, status);

	    obj.type |= CSI_OBJECT_ATTR_EXECUTABLE;
	}
    }

    /* consume whitespace after token, before calling the interpreter */
    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL) {
	status = csi_array_append (ctx,
				   scan->build_procedure.datum.array,
				   &obj);
    } else if (obj.type & CSI_OBJECT_ATTR_EXECUTABLE) {
	status = scan_execute (ctx, &obj);
	csi_object_free (ctx, &obj);
    } else {
	status = scan_push (ctx, &obj);
    }
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);
}

static void
string_add (csi_t *ctx, csi_scanner_t *scan, int c)
{
    buffer_check (ctx, scan, 1);
    buffer_add (&scan->buffer, c);
}

static void
string_end (csi_t *ctx, csi_scanner_t *scan)
{
    csi_object_t obj;
    cairo_status_t status;

    status = csi_string_new (ctx,
			     &obj,
			     scan->buffer.base,
			     scan->buffer.ptr - scan->buffer.base);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);

    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL)
	status = csi_array_append (ctx,
				   scan->build_procedure.datum.array,
				   &obj);
    else
	status = scan_push (ctx, &obj);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);
}

static int
hex_value (int c)
{
    if (c < '0')
	return EOF;
    if (c <= '9')
	return c - '0';
    c |= 32;
    if (c < 'a')
	return EOF;
    if (c <= 'f')
	return c - 'a' + 0xa;
    return EOF;
}

static void
hex_add (csi_t *ctx, csi_scanner_t *scan, int c)
{
    if (scan->accumulator_count == 0) {
	scan->accumulator |= hex_value (c) << 4;
	scan->accumulator_count = 1;
    } else {
	scan->accumulator |= hex_value (c) << 0;
	buffer_check (ctx, scan, 1);
	buffer_add (&scan->buffer, scan->accumulator);

	scan->accumulator = 0;
	scan->accumulator_count = 0;
    }
}

static void
hex_end (csi_t *ctx, csi_scanner_t *scan)
{
    csi_object_t obj;
    cairo_status_t status;

    if (scan->accumulator_count)
	hex_add (ctx, scan, '0');

    status = csi_string_new (ctx,
			     &obj,
			     scan->buffer.base,
			     scan->buffer.ptr - scan->buffer.base);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);

    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL)
	status = csi_array_append (ctx,
				   scan->build_procedure.datum.array,
				   &obj);
    else
	status = scan_push (ctx, &obj);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);
}

static void
base85_add (csi_t *ctx, csi_scanner_t *scan, int c)
{
    if (c == 'z') {
	if (_csi_unlikely (scan->accumulator_count != 0))
	    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

	buffer_check (ctx, scan, 4);
	buffer_add (&scan->buffer, 0);
	buffer_add (&scan->buffer, 0);
	buffer_add (&scan->buffer, 0);
	buffer_add (&scan->buffer, 0);
    } else if (_csi_unlikely (c < '!' || c > 'u')) {
	longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));
    } else {
	scan->accumulator = scan->accumulator*85 + c - '!';
	if (++scan->accumulator_count == 5) {
	    buffer_check (ctx, scan, 4);
	    buffer_add (&scan->buffer, (scan->accumulator >> 24) & 0xff);
	    buffer_add (&scan->buffer, (scan->accumulator >> 16) & 0xff);
	    buffer_add (&scan->buffer, (scan->accumulator >>  8) & 0xff);
	    buffer_add (&scan->buffer, (scan->accumulator >>  0) & 0xff);

	    scan->accumulator = 0;
	    scan->accumulator_count = 0;
	}
    }
}

static void
base85_end (csi_t *ctx, csi_scanner_t *scan, cairo_bool_t deflate)
{
    csi_object_t obj;
    cairo_status_t status;

    buffer_check (ctx, scan, 4);

    switch (scan->accumulator_count) {
    case 0:
	break;
    case 1:
	longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));
	break;

    case 2:
	scan->accumulator = scan->accumulator * (85*85*85) + 85*85*85 -1;
	buffer_add (&scan->buffer, (scan->accumulator >> 24) & 0xff);
	break;
    case 3:
	scan->accumulator = scan->accumulator * (85*85) + 85*85 -1;
	buffer_add (&scan->buffer, (scan->accumulator >> 24) & 0xff);
	buffer_add (&scan->buffer, (scan->accumulator >> 16) & 0xff);
	break;
    case 4:
	scan->accumulator = scan->accumulator * 85 + 84;
	buffer_add (&scan->buffer, (scan->accumulator >> 24) & 0xff);
	buffer_add (&scan->buffer, (scan->accumulator >> 16) & 0xff);
	buffer_add (&scan->buffer, (scan->accumulator >>  8) & 0xff);
	break;
    }

    if (deflate) {
        uLongf len = be32 (*(uint32_t *) scan->buffer.base);
	Bytef *source = (Bytef *) (scan->buffer.base + sizeof (uint32_t));

	status = csi_string_deflate_new (ctx, &obj,
					 source,
					 (Bytef *) scan->buffer.ptr - source,
					 len);
	if (_csi_unlikely (status))
	    longjmp (scan->jump_buffer, status);
    } else {
	status = csi_string_new (ctx,
				 &obj,
				 scan->buffer.base,
				 scan->buffer.ptr - scan->buffer.base);
	if (_csi_unlikely (status))
	    longjmp (scan->jump_buffer, status);
    }

    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL)
	status = csi_array_append (ctx,
				   scan->build_procedure.datum.array,
				   &obj);
    else
	status = scan_push (ctx, &obj);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);
}

static void
base64_add (csi_t *ctx, csi_scanner_t *scan, int c)
{
    int val;

    /* Convert Base64 character to its 6 bit nibble */
    val = scan->accumulator;
    if (c =='/') {
	val = (val << 6) | 63;
    } else if (c =='+') {
	val = (val << 6) | 62;
    } else if (c >='A' && c <='Z') {
	val = (val << 6) | (c -'A');
    } else if (c >='a' && c <='z') {
	val = (val << 6) | (c -'a' + 26);
    } else if (c >='0' && c <='9') {
	val = (val << 6) | (c -'0' + 52);
    }

    buffer_check (ctx, scan, 1);
    switch (scan->accumulator_count++) {
    case 0:
	break;

    case 1:
	buffer_add (&scan->buffer, (val >> 4) & 0xFF);
	val &= 0xF;
	break;

    case 2:
	buffer_add (&scan->buffer, (val >> 2) & 0xFF);
	val &= 0x3;
	break;

    case 3:
	buffer_add (&scan->buffer, (val >> 0) & 0xFF);
	scan->accumulator_count = 0;
	val = 0;
	break;
    }

     if (c == '=') {
	scan->accumulator_count = 0;
	scan->accumulator = 0;
     } else {
	 scan->accumulator = val;
     }
}

static void
base64_end (csi_t *ctx, csi_scanner_t *scan)
{
    csi_object_t obj;
    cairo_status_t status;

    switch (scan->accumulator_count) {
    case 0:
	break;
    case 2:
	base64_add (ctx, scan, (scan->accumulator << 2) & 0x3f);
	base64_add (ctx, scan, '=');
	break;
    case 1:
	base64_add (ctx, scan, (scan->accumulator << 4) & 0x3f);
	base64_add (ctx, scan, '=');
	base64_add (ctx, scan, '=');
	break;
    }

    status = csi_string_new (ctx,
			     &obj,
			     scan->buffer.base,
			     scan->buffer.ptr - scan->buffer.base);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);

    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL)
	status = csi_array_append (ctx,
				   scan->build_procedure.datum.array,
				   &obj);
    else
	status = scan_push (ctx, &obj);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);
}

static void
scan_read (csi_scanner_t *scan, csi_file_t *src, void *ptr, int len)
{
    uint8_t *data = ptr;
    do {
	int ret = csi_file_read (src, data, len);
	if (_csi_unlikely (ret == 0))
	    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_READ_ERROR));
	data += ret;
	len -= ret;
    } while (_csi_unlikely (len));
}

static void
string_read (csi_t *ctx,
	     csi_scanner_t *scan,
	     csi_file_t *src,
	     int len,
	     int compressed,
	     csi_object_t *obj)
{
    csi_status_t status;

    status = csi_string_new (ctx, obj, NULL, len);
    if (_csi_unlikely (status))
	longjmp (scan->jump_buffer, status);

    if (compressed) {
	uint32_t u32;
	scan_read (scan, src, &u32, 4);
	obj->datum.string->deflate = be32 (u32);
	obj->datum.string->method = compressed;
    }

    if (_csi_likely (len))
	scan_read (scan, src, obj->datum.string->string, len);
    obj->datum.string->string[len] = '\0';
}

static void
_scan_file (csi_t *ctx, csi_file_t *src)
{
    csi_scanner_t *scan = &ctx->scanner;
    int c, next;
    union {
	int8_t i8;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	float f;
    } u;
    int deflate = 0;
    int string_p;

scan_none:
    while ((c = csi_file_getc (src)) != EOF) {
	csi_object_t obj = { CSI_OBJECT_TYPE_NULL };

	switch (c) {
	case 0xa:
	    scan->line_number++;
	case 0x0:
	case 0x9:
	case 0xc:
	case 0xd:
	case 0x20: /* ignore whitespace */
	    break;

	case '%':
	    goto scan_comment;

	case '(':
	    goto scan_string;

	case '[': /* needs special case */
	case ']':
	case '{':
	case '}':
	    token_start (scan);
	    token_add_unchecked (scan, c);
	    token_end (ctx, scan, src);
	    goto scan_none;

	case '<':
	    next = csi_file_getc (src);
	    switch (next) {
	    case EOF:
		csi_file_putc (src, '<');
		return;
	    case '<':
		/* dictionary name */
		token_start (scan);
		token_add_unchecked (scan, '<');
		token_add_unchecked (scan, '<');
		token_end (ctx, scan, src);
		goto scan_none;
	    case '|':
		deflate = 1;
	    case '~':
		goto scan_base85;
	    case '{':
		goto scan_base64;
	    default:
		csi_file_putc (src, next);
		goto scan_hex;
	    }
	    break;

	    /* binary token */
#define MSB_INT8 128
#define MSB_UINT8 129
#define MSB_INT16 130
#define MSB_UINT16 131
#define MSB_INT32 132
#define LSB_INT8 MSB_INT8
#define LSB_UINT8 MSB_UINT8
#define LSB_INT16 133
#define LSB_UINT16 134
#define LSB_INT32 135
#define MSB_FLOAT32 140
#define LSB_FLOAT32 141
	case MSB_INT8:
	    scan_read (scan, src, &u.i8, 1);
	    csi_integer_new (&obj, u.i8);
	    break;
	case MSB_UINT8:
	    scan_read (scan, src, &u.u8, 1);
	    csi_integer_new (&obj, u.u8);
	    break;
	case MSB_INT16:
	    scan_read (scan, src, &u.i16, 2);
	    csi_integer_new (&obj, be16 (u.i16));
	    break;
	case LSB_INT16:
	    scan_read (scan, src, &u.i16, 2);
	    csi_integer_new (&obj, le16 (u.i16));
	    break;
	case MSB_UINT16:
	    scan_read (scan, src, &u.u16, 2);
	    csi_integer_new (&obj, be16 (u.u16));
	    break;
	case LSB_UINT16:
	    scan_read (scan, src, &u.u16, 2);
	    csi_integer_new (&obj, le16 (u.u16));
	    break;
	case MSB_INT32:
	    scan_read (scan, src, &u.i32, 4);
	    csi_integer_new (&obj, be32 (u.i32));
	    break;
	case LSB_INT32:
	    scan_read (scan, src, &u.i32, 4);
	    csi_integer_new (&obj, le32 (u.i32));
	    break;

	case 136: /* 16.16 msb */
	    scan_read (scan, src, &u.i32, 4);
	    csi_real_new (&obj, be32 (u.i32) / 65536.);
	    break;
	case 137: /* 16.16 lsb */
	    scan_read (scan, src, &u.i32, 4);
	    csi_real_new (&obj, le32 (u.i32) / 65536.);
	    break;
	case 138: /* 24.8 msb */
	    scan_read (scan, src, &u.i32, 4);
	    csi_real_new (&obj, be32 (u.i32) / 256.);
	    break;
	case 139: /* 24.8 lsb */
	    scan_read (scan, src, &u.i32, 4);
	    csi_real_new (&obj, le32 (u.i32) / 256.);
	    break;
	case MSB_FLOAT32:
	    scan_read (scan, src, &u.i32, 4);
	    u.i32 = be32 (u.i32);
	    csi_real_new (&obj, u.f);
	    break;
	case LSB_FLOAT32:
	    scan_read (scan, src, &u.i32, 4);
	    u.i32 = le32 (u.i32);
	    csi_real_new (&obj, u.f);
	    break;

#define STRING_1 142
#define STRING_2_MSB 144
#define STRING_2_LSB 146
#define STRING_4_MSB 148
#define STRING_4_LSB 150
#define STRING_DEFLATE 1
	case STRING_1:
	case STRING_1 | STRING_DEFLATE:
	    scan_read (scan, src, &u.u8, 1);
	    string_read (ctx, scan, src, u.u8, c & STRING_DEFLATE, &obj);
	    break;
	case STRING_2_MSB:
	case STRING_2_MSB | STRING_DEFLATE:
	    scan_read (scan, src, &u.u16, 2);
	    string_read (ctx, scan, src, be16 (u.u16),  c & STRING_DEFLATE, &obj);
	    break;
	case STRING_2_LSB:
	case STRING_2_LSB | STRING_DEFLATE:
	    scan_read (scan, src, &u.u16, 2);
	    string_read (ctx, scan, src, le16 (u.u16), c & STRING_DEFLATE, &obj);
	    break;
	case STRING_4_MSB:
	case STRING_4_MSB | STRING_DEFLATE:
	    scan_read (scan, src, &u.u32, 4);
	    string_read (ctx, scan, src, be32 (u.u32), c & STRING_DEFLATE, &obj);
	    break;
	case STRING_4_LSB:
	case STRING_4_LSB | STRING_DEFLATE:
	    scan_read (scan, src, &u.u32, 4);
	    string_read (ctx, scan, src, le32 (u.u32), c & STRING_DEFLATE, &obj);
	    break;

#define OPCODE 152
	case OPCODE:
	    scan_read (scan, src, &u.u8, 1);
	    csi_operator_new (&obj, ctx->opcode[u.u8]);
	    break;

	case OPCODE | 1:
	    scan_read (scan, src, &u.u8, 1);
	    csi_operator_new (&obj, ctx->opcode[u.u8]);
	    obj.type &= ~CSI_OBJECT_ATTR_EXECUTABLE;
	    break;

#define STRING_LZO 154
	case STRING_LZO:
	    scan_read (scan, src, &u.u32, 4);
	    string_read (ctx, scan, src, be32 (u.u32), LZO, &obj);
	    break;

	    /* unassigned */
	case 155:
	case 156:
	case 157:
	case 158:
	case 159:
	    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

	case '#': /* PDF 1.2 escape code */
	    {
		int c_hi = csi_file_getc (src);
		int c_lo = csi_file_getc (src);
		c = (hex_value (c_hi) << 4) | hex_value (c_lo);
	    }
	    /* fall-through */
	default:
	    token_start (scan);
	    token_add_unchecked (scan, c);
	    goto scan_token;
	}

	if (obj.type != CSI_OBJECT_TYPE_NULL) {
	    cairo_status_t status;

	    if (scan->build_procedure.type != CSI_OBJECT_TYPE_NULL) {
		status = csi_array_append (ctx,
					   scan->build_procedure.datum.array,
					   &obj);
	    } else if (obj.type & CSI_OBJECT_ATTR_EXECUTABLE) {
		status = scan_execute (ctx, &obj);
		csi_object_free (ctx, &obj);
	    } else {
		status = scan_push (ctx, &obj);
	    }
	    if (_csi_unlikely (status))
		longjmp (scan->jump_buffer, status);
	}
    }
    return;

scan_token:
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case 0xa:
	    scan->line_number++;
	    /* fall through */
	case 0x0:
	case 0x9:
	case 0xc:
	case 0xd:
	case 0x20:
	    token_end (ctx, scan, src);
	    goto scan_none;

	    /* syntax delimiters */
	case '%':
	    token_end (ctx, scan, src);
	    goto scan_comment;
	    /* syntax error? */
	case '(':
	    token_end (ctx, scan, src);
	    goto scan_string;
	    /* XXX syntax error? */
	case ')':
	    token_end (ctx, scan, src);
	    goto scan_none;
	case '/':
	    /* need to special case '^//?' */
	    if (scan->buffer.ptr > scan->buffer.base+1 ||
		scan->buffer.base[0] != '/')
	    {
		token_end (ctx, scan, src);
		token_start (scan);
	    }
	    token_add_unchecked (scan, '/');
	    goto scan_token;

	case '{':
	case '}':
	case ']':
	    token_end (ctx, scan, src);
	    token_start (scan);
	    token_add_unchecked (scan, c);
	    token_end (ctx, scan, src);
	    goto scan_none;

	case '<':
	    csi_file_putc (src, '<');
	    token_end (ctx, scan, src);
	    goto scan_none;

	case '#': /* PDF 1.2 escape code */
	    {
		int c_hi = csi_file_getc (src);
		int c_lo = csi_file_getc (src);
		c = (hex_value (c_hi) << 4) | hex_value (c_lo);
	    }
	    /* fall-through */
	default:
	    token_add (ctx, scan, c);
	    break;
	}
    }
    token_end (ctx, scan, src);
    return;

scan_comment:
    /* discard until newline */
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case 0xa:
	    scan->line_number++;
	case 0xc:
	    goto scan_none;
	}
    }
    return;

scan_string:
    buffer_reset (&scan->buffer);
    string_p = 1;
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case '\\': /* escape */
	    next = csi_file_getc (src);
	    switch (next) {
	    case EOF:
		longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

	    case 'n':
		string_add (ctx, scan, '\n');
		break;
	    case 'r':
		string_add (ctx, scan, '\r');
		break;
	    case 't':
		string_add (ctx, scan, '\t');
		break;
	    case 'b':
		string_add (ctx, scan, '\b');
		break;
	    case 'f':
		string_add (ctx, scan, '\f');
		break;
	    case '\\':
		string_add (ctx, scan, '\\');
		break;
	    case '(':
		string_add (ctx, scan, '(');
		break;
	    case ')':
		string_add (ctx, scan, ')');
		break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		{ /* octal code: \d{1,3} */
		    int i;

		    c = next - '0';

		    for (i = 0; i < 2; i++) {
			next = csi_file_getc (src);
			switch (next) {
			case EOF:
			    return;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			    c = 8*c + next-'0';
			    break;

			default:
			    csi_file_putc (src, next);
			    goto octal_code_done;
			}
		    }
  octal_code_done:
		    string_add (ctx, scan, c);
		}
		break;

	    case 0xa:
		/* skip the newline */
		next = csi_file_getc (src); /* might be compound LFCR */
		switch (next) {
		case EOF:
		    return;
		case 0xc:
		    break;
		default:
		    csi_file_putc (src, next);
		    break;
		}
		scan->line_number++;
		break;
	    case 0xc:
		break;

	    default:
		/* ignore the '\' */
		break;
	    }
	    break;

	case '(':
	    string_p++;
	    string_add (ctx, scan, c);
	    break;

	case ')':
	    if (--string_p == 0) {
		string_end (ctx, scan);
		goto scan_none;
	    }
	    /* fall through */
	default:
	    string_add (ctx, scan, c);
	    break;
	}
    }
    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

scan_hex:
    buffer_reset (&scan->buffer);
    scan->accumulator_count = 0;
    scan->accumulator = 0;
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case 0xa:
	    scan->line_number++;
	case 0x0:
	case 0x9:
	case 0xc:
	case 0xd:
	case 0x20: /* ignore whitespace */
	    break;

	case '>':
	    hex_end (ctx, scan); /* fixup odd digit with '0' */
	    goto scan_none;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	    hex_add (ctx, scan, c);
	    break;

	default:
	    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));
	}
    }
    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

scan_base85:
    buffer_reset (&scan->buffer);
    scan->accumulator = 0;
    scan->accumulator_count = 0;
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case '~':
	    next = csi_file_getc (src);
	    switch (next) {
	    case EOF:
		return;

	    case '>':
		base85_end (ctx, scan, deflate);
		deflate = 0;
		goto scan_none;
	    }
	    csi_file_putc (src, next);

	    /* fall-through */
	default:
	    base85_add (ctx, scan, c);
	    break;
	}
    }
    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

scan_base64:
    buffer_reset (&scan->buffer);
    scan->accumulator = 0;
    scan->accumulator_count = 0;
    while ((c = csi_file_getc (src)) != EOF) {
	switch (c) {
	case '}':
	    next = csi_file_getc (src);
	    switch (next) {
	    case EOF:
		return;

	    case '>':
		base64_end (ctx, scan);
		goto scan_none;
	    }
	    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));

	default:
	    base64_add (ctx, scan, c);
	    break;
	}
    }
    longjmp (scan->jump_buffer, _csi_error (CSI_STATUS_INVALID_SCRIPT));
}

static csi_status_t
_scan_push (csi_t *ctx, csi_object_t *obj)
{
    if (DEBUG_SCAN) {
	fprintf (stderr, "push ");
	fprintf_obj (stderr, ctx, obj);
    }
    return _csi_push_ostack (ctx, obj);
}

static csi_status_t
_scan_execute (csi_t *ctx, csi_object_t *obj)
{
    if (DEBUG_SCAN) {
	fprintf (stderr, "exec ");
	fprintf_obj (stderr, ctx, obj);
    }
    return csi_object_execute (ctx, obj);
}

csi_status_t
_csi_scanner_init (csi_t *ctx, csi_scanner_t *scanner)
{
    csi_status_t status;

    memset (scanner, 0, sizeof (csi_scanner_t));

    status = buffer_init (ctx, &scanner->buffer);
    if (status)
	return status;

    status = _csi_stack_init (ctx, &scanner->procedure_stack, 4);
    if (status)
	return status;

    scanner->bind = 0;
    scanner->push = _scan_push;
    scanner->execute = _scan_execute;

    return CSI_STATUS_SUCCESS;
}

void
_csi_scanner_fini (csi_t *ctx, csi_scanner_t *scanner)
{
    buffer_fini (ctx, &scanner->buffer);
    _csi_stack_fini (ctx, &scanner->procedure_stack);
    if (scanner->build_procedure.type != CSI_OBJECT_TYPE_NULL)
	csi_object_free (ctx, &scanner->build_procedure);
}

csi_status_t
_csi_scan_file (csi_t *ctx, csi_file_t *src)
{
    csi_status_t status;
    int old_line_number;

    /* This function needs to be reentrant to handle recursive scanners.
     * i.e. one script executes a second.
     */

    if (ctx->scanner.depth++ == 0) {
	if ((status = setjmp (ctx->scanner.jump_buffer))) {
	    ctx->scanner.depth = 0;
	    return status;
	}
    }

    old_line_number = ctx->scanner.line_number;
    ctx->scanner.line_number = 0;

    _scan_file (ctx, src);

    ctx->scanner.line_number = old_line_number;

    --ctx->scanner.depth;
    return CSI_STATUS_SUCCESS;
}

struct _translate_closure {
    csi_dictionary_t *opcodes;
    cairo_write_func_t write_func;
    void *closure;
};

static csi_status_t
_translate_name (csi_t *ctx,
	         csi_name_t name,
		 csi_boolean_t executable,
		 struct _translate_closure *closure)
{
    if (executable) {
	csi_dictionary_entry_t *entry;
	uint16_t u16;

	/* Bind executable names.
	 * XXX This may break some scripts that overload system operators.
	 */
	entry = _csi_hash_table_lookup (&closure->opcodes->hash_table,
		                        (csi_hash_entry_t *) &name);
	if (entry == NULL)
	    goto STRING;

	u16 = entry->value.datum.integer;
	u16 = be16 (u16);
	closure->write_func (closure->closure, (unsigned char *) &u16, 2);
    } else {
	closure->write_func (closure->closure, (unsigned char *) "/", 1);
STRING:
	closure->write_func (closure->closure,
		             (unsigned char *) name,
			     strlen ((char *) name));
	closure->write_func (closure->closure, (unsigned char *) "\n", 1);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_operator (csi_t *ctx,
	             csi_operator_t op,
		     csi_boolean_t executable,
		     struct _translate_closure *closure)
{
    csi_dictionary_entry_t *entry;
    uint16_t u16;

    entry = _csi_hash_table_lookup (&closure->opcodes->hash_table,
	                            (csi_hash_entry_t *) &op);
    if (entry == NULL)
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    u16 = entry->value.datum.integer;
    if (! executable)
	u16 += 1 << 8;
    u16 = be16 (u16);
    closure->write_func (closure->closure, (unsigned char *) &u16, 2);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_integer (csi_t *ctx,
	            csi_integer_t i,
		    struct _translate_closure *closure)
{
    uint8_t hdr;
    union {
	int8_t i8;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
    } u;
    int len;

#if WORDS_BIGENDIAN
    if (i < INT16_MIN) {
	hdr = MSB_INT32;
	len = 4;
	u.i32 = i;
    } else if (i < INT8_MIN) {
	hdr = MSB_INT16;
	len = 2;
	u.i16 = i;
    } else if (i < 0) {
	hdr = MSB_INT8;
	len = 1;
	u.i8 = i;
    } else if (i <= UINT8_MAX) {
	hdr = MSB_UINT8;
	len = 1;
	u.u8 = i;
    } else if (i <= UINT16_MAX) {
	hdr = MSB_UINT16;
	len = 2;
	u.u16 = i;
    } else {
	hdr = MSB_INT32;
	len = 4;
	u.u32 = i;
    }
#else
    if (i < INT16_MIN) {
	hdr = LSB_INT32;
	len = 4;
	u.i32 = i;
    } else if (i < INT8_MIN) {
	hdr = LSB_INT16;
	len = 2;
	u.i16 = i;
    } else if (i < 0) {
	hdr = LSB_INT8;
	len = 1;
	u.i8 = i;
    } else if (i <= UINT8_MAX) {
	hdr = LSB_UINT8;
	len = 1;
	u.u8 = i;
    } else if (i <= UINT16_MAX) {
	hdr = LSB_UINT16;
	len = 2;
	u.u16 = i;
    } else {
	hdr = LSB_INT32;
	len = 4;
	u.u32 = i;
    }
#endif

    closure->write_func (closure->closure, (unsigned char *) &hdr, 1);
    closure->write_func (closure->closure, (unsigned char *) &u, len);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_real (csi_t *ctx,
	         csi_real_t real,
		 struct _translate_closure *closure)
{
    uint8_t hdr;

    if (real >= INT32_MIN && real <= INT32_MAX && (int) real == real)
	return _translate_integer (ctx, real, closure);

#if WORDS_BIGENDIAN
    hdr = MSB_FLOAT32;
#else
    hdr = LSB_FLOAT32;
#endif
    closure->write_func (closure->closure, (unsigned char *) &hdr, 1);
    closure->write_func (closure->closure, (unsigned char *) &real, 4);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_string (csi_t *ctx,
	           csi_string_t *string,
		   struct _translate_closure *closure)
{
    uint8_t hdr;
    union {
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
    } u;
    void *buf;
    unsigned long hdr_len, buf_len, deflate;
    int method;

    buf = string->string;
    buf_len = string->len;
    deflate = string->deflate;
    method = string->method;

#if HAVE_LZO
    if (method == NONE && buf_len > 16) {
	unsigned long mem_len = 2*string->len > LZO2A_999_MEM_COMPRESS ? 2*string->len : LZO2A_999_MEM_COMPRESS;
	void *mem = malloc (mem_len);
	void *work = malloc(LZO2A_999_MEM_COMPRESS);

	if (lzo2a_999_compress ((lzo_bytep) buf, buf_len,
				(lzo_bytep) mem, &mem_len,
				work) == 0 &&
	    8+2*mem_len < buf_len)
	{
	    method = LZO;
	    deflate = buf_len;
	    buf_len = mem_len;
	    buf = mem;
	}
	else
	{
	    free (mem);
	}

	free (work);
    }
#if HAVE_ZLIB
    if (method == ZLIB) {
	buf_len = string->deflate;
	buf = malloc (string->deflate);
	if (uncompress ((Bytef *) buf, &buf_len,
			(Bytef *) string->string, string->len) == Z_OK)
	{
	    assert(string->len > 0);
	    if (buf_len <= 8 + 2*((unsigned long)string->len)) {
		method = NONE;
		deflate = 0;
	    } else {
		unsigned long mem_len = 2*string->deflate;
		void *mem = malloc (mem_len);
		void *work = malloc(LZO2A_999_MEM_COMPRESS);

		if (lzo2a_999_compress ((lzo_bytep) buf, buf_len,
					(lzo_bytep) mem, &mem_len,
					work) == 0)
		{
		    if (8 + mem_len > buf_len) {
			method = NONE;
			deflate = 0;
		    } else {
			free (buf);
			method = LZO;
			deflate = buf_len;
			buf_len = mem_len;
			buf = mem;
			assert(deflate);
		    }
		}
		else
		{
		    free (buf);
		    buf = string->string;
		    buf_len = string->len;
		}

		free (work);
	    }
	}
	else
	{
	    free (buf);
	    buf = string->string;
	    buf_len = string->len;
	}
    }
#endif
#endif

    if (method == LZO) {
	hdr = STRING_LZO;
	u.u32 = to_be32 (buf_len);
	hdr_len = 4;
    } else {
#if WORDS_BIGENDIAN
	if (buf_len <= UINT8_MAX) {
	    hdr = STRING_1;
	    u.u8 = buf_len;
	    hdr_len = 1;
	} else if (buf_len <= UINT16_MAX) {
	    hdr = STRING_2_MSB;
	    u.u16 = buf_len;
	    hdr_len = 2;
	} else {
	    hdr = STRING_4_MSB;
	    u.u32 = buf_len;
	    hdr_len = 4;
	}
#else
	if (buf_len <= UINT8_MAX) {
	    hdr = STRING_1;
	    u.u8 = buf_len;
	    hdr_len = 1;
	} else if (buf_len <= UINT16_MAX) {
	    hdr = STRING_2_LSB;
	    u.u16 = buf_len;
	    hdr_len = 2;
	} else {
	    hdr = STRING_4_LSB;
	    u.u32 = buf_len;
	    hdr_len = 4;
	}
#endif
	if (deflate) {
	    assert (method == ZLIB);
	    hdr |= STRING_DEFLATE;
	}
    }

    closure->write_func (closure->closure, (unsigned char *) &hdr, 1);
    closure->write_func (closure->closure, (unsigned char *) &u, hdr_len);
    if (deflate) {
	uint32_t u32 = to_be32 (deflate);
	closure->write_func (closure->closure, (unsigned char *) &u32, 4);
    }
    closure->write_func (closure->closure, (unsigned char *) buf, buf_len);

    if (buf != string->string)
	free (buf);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_push (csi_t *ctx, csi_object_t *obj)
{
    struct _translate_closure *closure = ctx->scanner.closure;

    if (0) {
	fprintf (stderr, "push ");
	fprintf_obj (stderr, ctx, obj);
    }

    switch (csi_object_get_type (obj)) {
    case CSI_OBJECT_TYPE_NAME:
	return _translate_name (ctx, obj->datum.name, FALSE, closure);

    case CSI_OBJECT_TYPE_OPERATOR:
	return _translate_operator (ctx, obj->datum.op, FALSE, closure);

    case CSI_OBJECT_TYPE_INTEGER:
	return _translate_integer (ctx, obj->datum.integer, closure);

    case CSI_OBJECT_TYPE_REAL:
	return _translate_real (ctx, obj->datum.real, closure);

    case CSI_OBJECT_TYPE_STRING:
	return _translate_string (ctx, obj->datum.string, closure);

    case CSI_OBJECT_TYPE_NULL:
    case CSI_OBJECT_TYPE_BOOLEAN:
    case CSI_OBJECT_TYPE_MARK:
    case CSI_OBJECT_TYPE_ARRAY:
    case CSI_OBJECT_TYPE_DICTIONARY:
    case CSI_OBJECT_TYPE_FILE:
    case CSI_OBJECT_TYPE_MATRIX:
    case CSI_OBJECT_TYPE_CONTEXT:
    case CSI_OBJECT_TYPE_FONT:
    case CSI_OBJECT_TYPE_PATTERN:
    case CSI_OBJECT_TYPE_SCALED_FONT:
    case CSI_OBJECT_TYPE_SURFACE:
	longjmp (ctx->scanner.jump_buffer,  _csi_error (CSI_STATUS_INVALID_SCRIPT));
	break;
    }

    csi_object_free (ctx, obj);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate_execute (csi_t *ctx, csi_object_t *obj)
{
    struct _translate_closure *closure = ctx->scanner.closure;

    if (0) {
	fprintf (stderr, "exec ");
	fprintf_obj (stderr, ctx, obj);
    }

    switch (csi_object_get_type (obj)) {
    case CSI_OBJECT_TYPE_NAME:
	return _translate_name (ctx, obj->datum.name, TRUE, closure);

    case CSI_OBJECT_TYPE_OPERATOR:
	return _translate_operator (ctx, obj->datum.op, TRUE, closure);

    case CSI_OBJECT_TYPE_INTEGER:
	return _translate_integer (ctx, obj->datum.integer, closure);

    case CSI_OBJECT_TYPE_REAL:
	return _translate_real (ctx, obj->datum.real, closure);

    case CSI_OBJECT_TYPE_STRING:
	return _translate_string (ctx, obj->datum.string, closure);

    case CSI_OBJECT_TYPE_NULL:
    case CSI_OBJECT_TYPE_BOOLEAN:
    case CSI_OBJECT_TYPE_MARK:
    case CSI_OBJECT_TYPE_ARRAY:
    case CSI_OBJECT_TYPE_DICTIONARY:
    case CSI_OBJECT_TYPE_FILE:
    case CSI_OBJECT_TYPE_MATRIX:
    case CSI_OBJECT_TYPE_CONTEXT:
    case CSI_OBJECT_TYPE_FONT:
    case CSI_OBJECT_TYPE_PATTERN:
    case CSI_OBJECT_TYPE_SCALED_FONT:
    case CSI_OBJECT_TYPE_SURFACE:
	longjmp (ctx->scanner.jump_buffer,  _csi_error (CSI_STATUS_INVALID_SCRIPT));
	break;
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
build_opcodes (csi_t *ctx, csi_dictionary_t **out)
{
    csi_object_t obj;
    csi_dictionary_t *dict;
    const csi_operator_def_t *def;
    csi_status_t status;
    int opcode = OPCODE << 8;

    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    dict = obj.datum.dictionary;

    csi_integer_new (&obj, opcode++);
    status = csi_dictionary_put (ctx, dict, 0, &obj);
    if (_csi_unlikely (status))
	goto FAIL;

    for (def = _csi_operators (); def->name != NULL; def++) {
	csi_object_t name;
	csi_dictionary_entry_t *entry;
	int code;

	entry = _csi_hash_table_lookup (&dict->hash_table,
		                        (csi_hash_entry_t *) &def->op);
	if (entry == NULL) {
	    code = opcode++;
	    csi_integer_new (&obj, code);
	    status = csi_dictionary_put (ctx, dict, (csi_name_t) def->op, &obj);
	    if (_csi_unlikely (status))
		goto FAIL;
	} else {
	    code = entry->value.datum.integer;
	    csi_integer_new (&obj, code);
	}
	assert (ctx->opcode[code & 0xff] == def->op);

	status = csi_name_new_static (ctx, &name, def->name);
	if (_csi_unlikely (status))
	    goto FAIL;

	status = csi_dictionary_put (ctx, dict, name.datum.name, &obj);
	if (_csi_unlikely (status))
	    goto FAIL;
    }

    *out = dict;
    return CSI_STATUS_SUCCESS;

FAIL:
    csi_dictionary_free (ctx, dict);
    return status;
}

csi_status_t
_csi_translate_file (csi_t *ctx,
	             csi_file_t *file,
		     cairo_write_func_t write_func,
		     void *closure)
{
    csi_status_t status;
    struct _translate_closure translator;

    if ((status = setjmp (ctx->scanner.jump_buffer)))
	return status;

    status = build_opcodes (ctx, &translator.opcodes);
    if (_csi_unlikely (status))
	return status;

    translator.write_func = write_func;
    translator.closure = closure;
    ctx->scanner.closure = &translator;

    ctx->scanner.bind = 1;
    ctx->scanner.push = _translate_push;
    ctx->scanner.execute = _translate_execute;

    _scan_file (ctx, file);

    ctx->scanner.bind = 0;
    ctx->scanner.push = _scan_push;
    ctx->scanner.execute = _scan_execute;

    csi_dictionary_free (ctx, translator.opcodes);

    return CSI_STATUS_SUCCESS;
}
