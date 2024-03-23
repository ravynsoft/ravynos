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

#include <stdio.h>
#include <limits.h> /* INT_MAX */
#include <string.h>
#include <zlib.h>

#if HAVE_LZO
#include <lzo/lzo2a.h>
#endif

#define CHUNK_SIZE 32768

#define OWN_STREAM 0x1

csi_status_t
csi_file_new (csi_t *ctx,
	      csi_object_t *obj,
	      const char *path, const char *mode)
{
    csi_file_t *file;

    file = _csi_slab_alloc (ctx, sizeof (csi_file_t));
    if (file == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    file->base.type = CSI_OBJECT_TYPE_FILE;
    file->base.ref = 1;

    file->data = NULL;
    file->type = STDIO;
    file->flags = OWN_STREAM;
    file->src = fopen (path, mode);
    if (file->src == NULL) {
	_csi_slab_free (ctx, file, sizeof (csi_file_t));
	return _csi_error (CAIRO_STATUS_FILE_NOT_FOUND);
    }

    file->data = _csi_alloc (ctx, CHUNK_SIZE);
    if (file->data == NULL) {
	_csi_slab_free (ctx, file, sizeof (csi_file_t));
	return _csi_error (CAIRO_STATUS_NO_MEMORY);
    }
    file->bp = file->data;
    file->rem = 0;

    obj->type = CSI_OBJECT_TYPE_FILE;
    obj->datum.file = file;
    return CAIRO_STATUS_SUCCESS;
}

csi_status_t
csi_file_new_for_stream (csi_t *ctx,
	                 csi_object_t *obj,
			 FILE *stream)
{
    csi_file_t *file;

    file = _csi_slab_alloc (ctx, sizeof (csi_file_t));
    if (file == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    file->base.type = CSI_OBJECT_TYPE_FILE;
    file->base.ref = 1;

    file->data = NULL;
    file->type = STDIO;
    file->flags = 0;
    file->src = stream;
    if (file->src == NULL) {
	_csi_slab_free (ctx, file, sizeof (csi_file_t));
	return _csi_error (CAIRO_STATUS_FILE_NOT_FOUND);
    }

    file->data = _csi_alloc (ctx, CHUNK_SIZE);
    if (file->data == NULL) {
	_csi_slab_free (ctx, file, sizeof (csi_file_t));
	return _csi_error (CAIRO_STATUS_NO_MEMORY);
    }
    file->bp = file->data;
    file->rem = 0;

    obj->type = CSI_OBJECT_TYPE_FILE;
    obj->datum.file = file;
    return CAIRO_STATUS_SUCCESS;
}

csi_status_t
csi_file_new_for_bytes (csi_t *ctx,
			csi_object_t *obj,
			const char *bytes,
			unsigned int length)
{
    csi_file_t *file;

    file = _csi_slab_alloc (ctx, sizeof (csi_file_t));
    if (file == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    file->base.type = CSI_OBJECT_TYPE_FILE;
    file->base.ref = 1;

    file->type = BYTES;
    file->src  = (uint8_t *) bytes;
    file->data = (uint8_t *) bytes;
    file->bp   = (uint8_t *) bytes;
    file->rem  = length;

    obj->type = CSI_OBJECT_TYPE_FILE;
    obj->datum.file = file;
    return CAIRO_STATUS_SUCCESS;
}

csi_status_t
csi_file_new_from_string (csi_t *ctx,
			  csi_object_t *obj,
			  csi_string_t *src)
{
    csi_file_t *file;

    file = _csi_slab_alloc (ctx, sizeof (csi_file_t));
    if (_csi_unlikely (file == NULL))
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    file->base.type = CSI_OBJECT_TYPE_FILE;
    file->base.ref = 1;

    if (src->deflate) {
	uLongf len = src->deflate;
	csi_object_t tmp_obj;
	csi_string_t *tmp_str;
	csi_status_t status;

	status = csi_string_new (ctx, &tmp_obj,  NULL, src->deflate);
	if (_csi_unlikely (status))
	    return status;

	tmp_str = tmp_obj.datum.string;
	switch (src->method) {
	case NONE:
	default:
	    status = _csi_error (CAIRO_STATUS_NO_MEMORY);
	    break;

	case ZLIB:
#if HAVE_ZLIB
	    if (uncompress ((Bytef *) tmp_str->string, &len,
			    (Bytef *) src->string, src->len) != Z_OK)
#endif
		status = _csi_error (CAIRO_STATUS_NO_MEMORY);
	    break;
	case LZO:
#if HAVE_LZO
	    if (lzo2a_decompress ((lzo_bytep) src->string, src->len,
				  (lzo_bytep) tmp_str->string, &len,
				  NULL))
#endif
		status = _csi_error (CAIRO_STATUS_NO_MEMORY);
	    break;
	}
	if (_csi_unlikely (status)) {
	    csi_string_free (ctx, tmp_str);
	    _csi_slab_free (ctx, file, sizeof (csi_file_t));
	    return status;
	}

	file->src  = tmp_str;
	file->data = tmp_str->string;
	file->rem  = tmp_str->len;
    } else {
	file->src  = src; src->base.ref++;
	file->data = src->string;
	file->rem  = src->len;
    }
    file->type = BYTES;
    file->bp   = file->data;

    obj->type = CSI_OBJECT_TYPE_FILE;
    obj->datum.file = file;
    return CAIRO_STATUS_SUCCESS;
}

static csi_status_t
_csi_file_new_filter (csi_t *ctx,
		      csi_object_t *obj,
		      csi_object_t *src,
		      const csi_filter_funcs_t *funcs,
		      void *data)
{
    csi_file_t *file;
    csi_object_t src_file;
    csi_status_t status;

    file = _csi_slab_alloc (ctx, sizeof (csi_file_t));
    if (file == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    obj->type = CSI_OBJECT_TYPE_FILE;
    obj->datum.file = file;

    file->base.type = CSI_OBJECT_TYPE_FILE;
    file->base.ref = 1;

    file->type = FILTER;
    file->data = data;
    file->filter = funcs;
    status = csi_object_as_file (ctx, src, &src_file);
    if (status) {
	csi_object_free (ctx, obj);
	return status;
    }
    file->src = src_file.datum.file;

    return CAIRO_STATUS_SUCCESS;
}


#if 0
csi_status_t
csi_file_new_from_stream (csi_t *ctx,
			  FILE *file,
			  csi_object_t **out)
{
    csi_file_t *obj;

    obj = (csi_file_t *) _csi_object_new (ctx, CSI_OBJECT_TYPE_FILE);
    if (obj == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    obj->type = STDIO;
    obj->src = file;
    obj->data = _csi_alloc (ctx, CHUNK_SIZE);
    if (obj->data == NULL) {
	csi_object_free (&obj->base);
	return _csi_error (CAIRO_STATUS_UNDEFINED_FILENAME_ERROR);
    }
    obj->bp = obj->data;
    obj->rem = 0;

    *out = &obj->base;
    return CAIRO_STATUS_SUCCESS;
}

static csi_object_t *
_csi_file_new_from_procedure (csi_t *ctx, csi_object_t *src)
{
    csi_file_t *obj;

    obj = (csi_file_t *) _csi_object_new (ctx, CSI_OBJECT_TYPE_FILE);
    if (obj == NULL)
	return NULL;

    obj->type = PROCEDURE;
    obj->src = csi_object_reference (src);
    obj->data = NULL;

    return &obj->base;
}
#endif

typedef struct _ascii85_decode_data {
    uint8_t buf[CHUNK_SIZE];
    uint8_t *bp;
    short bytes_available;
    short eod;
} _ascii85_decode_data_t;

static int
_getc_skip_whitespace (csi_file_t *src)
{
    int c;

    do switch ((c = csi_file_getc (src))) {
    case 0x0:
    case 0x9:
    case 0xa:
    case 0xc:
    case 0xd:
    case 0x20:
	continue;

    default:
	return c;
    } while (TRUE);

    return c;
}

static void
_ascii85_decode (csi_file_t *file)
{
    _ascii85_decode_data_t *data = file->data;
    unsigned int n;

    if (data->eod)
	return;

    data->bp = data->buf;

    n = 0;
    do {
	unsigned int v = _getc_skip_whitespace (file->src);
	if (v == 'z') {
	    data->buf[n+0] = 0;
	    data->buf[n+1] = 0;
	    data->buf[n+2] = 0;
	    data->buf[n+3] = 0;
	} else if (v == '~') {
	    _getc_skip_whitespace (file->src); /* == '>' || IO_ERROR */
	    data->eod = TRUE;
	    break;
	} else if (v < '!' || v > 'u') {
	    /* IO_ERROR */
	    data->eod = TRUE;
	    break;
	} else {
	    unsigned int i;

	    v -= '!';
	    for (i = 1; i < 5; i++) {
		int c = _getc_skip_whitespace (file->src);
		if (c == '~') { /* short tuple */
		    _getc_skip_whitespace (file->src); /* == '>' || IO_ERROR */
		    data->eod = TRUE;
		    switch (i) {
		    case 0:
		    case 1:
			/* IO_ERROR */
			break;
		    case 2:
			v = v * (85*85*85) + 85*85*85 -1;
			goto odd1;
		    case 3:
			v = v * (85*85) + 85*85 -1;
			goto odd2;
		    case 4:
			v = v * 85 + 84;
			data->buf[n+2] = v >> 8 & 0xff;
odd2:
			data->buf[n+1] = v >> 16 & 0xff;
odd1:
			data->buf[n+0] = v >> 24 & 0xff;
			data->bytes_available = n + i - 1;
			return;
		    }
		    break;
		}
		v = 85*v + c-'!';
	    }

	    data->buf[n+0] = v >> 24 & 0xff;
	    data->buf[n+1] = v >> 16 & 0xff;
	    data->buf[n+2] = v >> 8 & 0xff;
	    data->buf[n+3] = v >> 0 & 0xff;
	}
	n += 4;
    } while (n < sizeof (data->buf) && data->eod == FALSE);

    data->bytes_available = n;
}

static int
_ascii85_decode_getc (csi_file_t *file)
{
    _ascii85_decode_data_t *data = file->data;

    if (data->bytes_available == 0) {
	_ascii85_decode (file);

	if (data->bytes_available == 0)
	    return EOF;
    }

    data->bytes_available--;
    return *data->bp++;
}

static void
_ascii85_decode_putc (csi_file_t *file, int c)
{
    _ascii85_decode_data_t *data = file->data;
    data->bytes_available++;
    data->bp--;
}

static int
_ascii85_decode_read (csi_file_t *file, uint8_t *buf, int len)
{
    _ascii85_decode_data_t *data = file->data;

    if (data->bytes_available == 0) {
	_ascii85_decode (file);

	if (data->bytes_available == 0)
	    return 0;
    }

    if (len > data->bytes_available)
	len = data->bytes_available;
    memcpy (buf, data->bp, len);
    data->bp += len;
    data->bytes_available -= len;
    return len;
}

csi_status_t
csi_file_new_ascii85_decode (csi_t *ctx,
			     csi_object_t *obj,
			     csi_dictionary_t *dict,
			     csi_object_t *src)
{
    static const csi_filter_funcs_t funcs = {
	_ascii85_decode_getc,
	_ascii85_decode_putc,
	_ascii85_decode_read,
	_csi_free,
    };
    _ascii85_decode_data_t *data;

    data = _csi_alloc0 (ctx, sizeof (_ascii85_decode_data_t));
    if (data == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    return _csi_file_new_filter (ctx, obj, src, &funcs, data);
}

#if HAVE_ZLIB
#include <zlib.h>

typedef struct _deflate_decode_data {
    z_stream zlib_stream;

    uint8_t in[CHUNK_SIZE];
    uint8_t out[CHUNK_SIZE];

    int bytes_available;
    uint8_t *bp;
} _deflate_decode_data_t;

static void
_deflate_decode (csi_file_t *file)
{
    _deflate_decode_data_t *data = file->data;
    uint8_t *bp;
    int len;

    data->zlib_stream.next_out = data->out;
    data->zlib_stream.avail_out = sizeof (data->out);

    bp = data->in;
    len = sizeof (data->in);
    if (data->zlib_stream.avail_in) {
	memmove (data->in,
		 data->zlib_stream.next_in,
		 data->zlib_stream.avail_in);
	len -= data->zlib_stream.avail_in;
	bp += data->zlib_stream.avail_in;
    }

    len = csi_file_read (file->src, bp, len);

    data->zlib_stream.next_in = data->in;
    data->zlib_stream.avail_in += len;

    inflate (&data->zlib_stream, len == 0 ? Z_FINISH : Z_NO_FLUSH);

    data->bytes_available = data->zlib_stream.next_out - data->out;
    data->bp = data->out;
}

static int
_deflate_decode_getc (csi_file_t *file)
{
    _deflate_decode_data_t *data = file->data;

    if (data->bytes_available == 0) {
	_deflate_decode (file);

	if (data->bytes_available == 0)
	    return EOF;
    }

    data->bytes_available--;
    return *data->bp++;
}

static void
_deflate_decode_putc (csi_file_t *file, int c)
{
    _deflate_decode_data_t *data = file->data;
    data->bytes_available++;
    data->bp--;
}

static int
_deflate_decode_read (csi_file_t *file, uint8_t *buf, int len)
{
    _deflate_decode_data_t *data = file->data;

    if (data->bytes_available == 0) {
	_deflate_decode (file);

	if (data->bytes_available == 0)
	    return 0;
    }

    if (len > (int) data->bytes_available)
	len = data->bytes_available;
    memcpy (buf, data->bp, len);
    data->bp += len;
    data->bytes_available -= len;
    return len;
}

static void
_deflate_destroy (csi_t *ctx, void *closure)
{
    _deflate_decode_data_t *data;

    data = closure;

    inflateEnd (&data->zlib_stream);

    _csi_free (ctx, data);
}

csi_status_t
csi_file_new_deflate_decode (csi_t *ctx,
			     csi_object_t *obj,
			     csi_dictionary_t *dict,
			     csi_object_t *src)
{
    static const csi_filter_funcs_t funcs = {
	_deflate_decode_getc,
	_deflate_decode_putc,
	_deflate_decode_read,
	_deflate_destroy,
    };
    _deflate_decode_data_t *data;

    data = _csi_alloc (ctx, sizeof (_deflate_decode_data_t));
    if (data == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    data->zlib_stream.zalloc = Z_NULL;
    data->zlib_stream.zfree = Z_NULL;
    data->zlib_stream.opaque = Z_NULL;
    data->zlib_stream.next_in = data->in;
    data->zlib_stream.avail_in = 0;
    data->zlib_stream.next_out = data->out;
    data->zlib_stream.avail_out = sizeof (data->out);
    data->bytes_available = 0;

    if (inflateInit (&data->zlib_stream) != Z_OK) {
	_csi_free (ctx, data);
	return _csi_error (CAIRO_STATUS_NO_MEMORY);
    }

    return _csi_file_new_filter (ctx, obj, src, &funcs, data);
}
#endif

#if 0
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

/* Adobe Type 1 Font Format book: p63 */
typedef struct _decrypt_data {
    uint8_t putback[32];
    uint8_t nputback;
    csi_bool_t is_hexadecimal;
    unsigned short R;
    int eod;
} _decrypt_data_t;

static uint8_t
_decrypt (unsigned short *R, uint8_t cypher)
{
#define c1 52845
#define c2 22719
    uint8_t plain;

    plain = cypher ^ (*R >> 8);
    *R = (cypher + *R) * c1 + c2;
    return plain;
#undef c1
#undef c2
}

int
csi_decrypt (uint8_t *in, int length,
	     unsigned short salt, int binary,
	     uint8_t *out)
{
    const uint8_t * const end = in + length;
    uint8_t *base = out;

    while (in < end) {
	int c;

	if (! binary) {
	    int c_hi = -1, c_lo = 0;

	    while (in < end && (c_hi = *in++)) {
		switch (c_hi) {
		case 0x0:
		case 0x9:
		case 0xa:
		case 0xc:
		case 0xd:
		case 0x20:
		    continue;

		default:
		    break;
		}
	    }
	    if (c_hi < 0)
		break;

	    while (in < end && (c_lo = *in++)) {
		switch (c_lo) {
		case 0x0:
		case 0x9:
		case 0xa:
		case 0xc:
		case 0xd:
		case 0x20:
		    continue;

		default:
		    break;
		}
	    }

	    c = (hex_value (c_hi) << 4) | hex_value (c_lo);
	} else
	    c = *in++;

	*out++ = _decrypt (&salt, c);
    }

    return out - base;
}

static uint8_t
_encrypt (unsigned short *R, uint8_t plain)
{
#define c1 52845
#define c2 22719
    uint8_t cypher;

    cypher = plain ^ (*R >> 8);
    *R = (cypher + *R) * c1 + c2;
    return cypher;
#undef c1
#undef c2
}

int
csi_encrypt (uint8_t *in, int length,
	     unsigned short salt, int discard, int binary,
	     uint8_t *out)
{
    const char hex[]="0123456789abcdef";
    const uint8_t * const end = in + length;
    uint8_t *base = out;
    int col = 0;

    while (discard--) {
	if (! binary) {
	    int c = _encrypt (&salt, ' ');
	    *out++ = hex[(c >> 4) & 0xf];
	    *out++ = hex[(c >> 0) & 0xf];
	} else
	    *out++ = _encrypt (&salt, 0);
    }

    while (in < end) {
	int c;

	c = _encrypt (&salt, *in++);
	if (! binary) {
	    if (col == 78) {
		*out++ = '\n';
		col = 0;
	    }
	    *out++ = hex[(c >> 4) & 0xf];
	    *out++ = hex[(c >> 0) & 0xf];
	    col += 2;
	} else
	    *out++ = c;
    }

    return out - base;
}

static int
_decrypt_getc (csi_file_t *file)
{
    _decrypt_data_t *data = file->data;
    int c;

    if (data->nputback)
	return data->putback[--data->nputback];

    if (data->is_hexadecimal) {
	int c_hi, c_lo;

	c_hi = _getc_skip_whitespace (file->src);
	c_lo = _getc_skip_whitespace (file->src);
	c = (hex_value (c_hi) << 4) | hex_value (c_lo);
    } else
	c = csi_file_getc (file->src);

    if (c == EOF)
	return EOF;

    return _decrypt (&data->R, c);
}

static void
_decrypt_putc (csi_file_t *file, int c)
{
    _decrypt_data_t *data;

    data = file->data;

    data->putback[data->nputback++] = c;
}

csi_object_t *
csi_file_new_decrypt (csi_t *ctx, csi_object_t *src, int salt, int discard)
{
    csi_object_t *obj;
    _decrypt_data_t *data;
    int n;

    data = _csi_alloc0 (ctx, sizeof (_decrypt_data_t));
    if (data == NULL)
	return NULL;

    data->R = salt;

    obj = _csi_file_new_filter (ctx, src,
				_decrypt_getc,
				_decrypt_putc,
				NULL,
				_csi_free,
				data);
    if (obj == NULL)
	return NULL;

    /* XXX determine encoding, eexec only? */
    data->is_hexadecimal = salt != 4330;
    for (n = 0; n < discard; n++) {
	int c;
	c = csi_file_getc (obj);
	if (c == EOF) {
	    return obj;
	}
    }
    return obj;
}
#endif

csi_status_t
_csi_file_execute (csi_t *ctx, csi_file_t *obj)
{
    return _csi_scan_file (ctx, obj);
}

int
csi_file_getc (csi_file_t *file)
{
    int c;

    if (_csi_unlikely (file->src == NULL))
	return EOF;

    switch (file->type) {
    case STDIO:
	if (_csi_likely (file->rem)) {
	    c = *file->bp++;
	    file->rem--;
	} else {
	    file->rem = fread (file->bp = file->data, 1, CHUNK_SIZE, file->src);
	    /* fall through */
    case BYTES:
	    if (_csi_likely (file->rem)) {
		c = *file->bp++;
		file->rem--;
	    } else
		c = EOF;
	}
	break;

    case PROCEDURE:
#if 0
	if (file->data == NULL) {
	    csi_status_t status;
	    csi_object_t *string;

RERUN_PROCEDURE:
	    status = csi_object_execute (file->src);
	    if (status)
		return EOF;

	    string = csi_pop_operand (file->base.ctx);
	    if (string == NULL)
		return EOF;
	    file->data = csi_object_as_file (file->base.ctx, string);
	    csi_object_free (string);
	    if (file->data == NULL)
		return EOF;
	}
	c = csi_file_getc (file->data);
	if (c == EOF) {
	    csi_object_free (file->data);
	    file->data = NULL;
	    goto RERUN_PROCEDURE;
	}
#else
	c = EOF;
#endif
	break;

    case FILTER:
	c = file->filter->filter_getc (file);
	break;

    default:
	c = EOF;
	break;
    }

    return c;
}

int
csi_file_read (csi_file_t *file, void *buf, int len)
{
    int ret;

    if (file->src == NULL)
	return 0;

    switch (file->type) {
    case STDIO:
	if (file->rem > 0) {
	    ret = len;
	    if (file->rem < ret)
		ret = file->rem;
	    memcpy (buf, file->bp, ret);
	    file->bp  += ret;
	    file->rem -= ret;
	} else
	    ret = fread (buf, 1, len, file->src);
	break;

    case BYTES:
	if (file->rem > 0) {
	    ret = len;
	    if (file->rem < ret)
		ret = file->rem;
	    memcpy (buf, file->bp, ret);
	    file->bp  += ret;
	    file->rem -= ret;
	} else
	    ret = 0;
	break;

    case PROCEDURE:
#if 0
	if (file->data == NULL) {
	    csi_status_t status;
	    csi_object_t *string;

RERUN_PROCEDURE:
	    status = csi_object_execute (file->src);
	    if (status)
		return 0;

	    string = csi_pop_operand (file->base.ctx);
	    if (string == NULL)
		return 0;
	    file->data = csi_object_as_file (file->base.ctx, string);
	    csi_object_free (string);
	    if (file->data == NULL)
		return 0;
	}
	ret = csi_file_read (file->data, buf, len);
	if (ret == 0) {
	    csi_object_free (file->data);
	    file->data = NULL;
	    goto RERUN_PROCEDURE;
	}
#else
	ret = 0;
#endif
	break;

    case FILTER:
	ret = file->filter->filter_read (file, buf, len);
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}

void
csi_file_putc (csi_file_t *file, int c)
{
    if (file->src == NULL)
	return;

    switch ((int) file->type) {
    case STDIO:
    case BYTES:
	file->bp--;
	file->rem++;
	break;
    case FILTER:
	file->filter->filter_putc (file, c);
	break;
    default:
	break;
    }
}

void
csi_file_flush (csi_file_t *file)
{
    if (file->src == NULL)
	return;

    switch ((int) file->type) {
    case FILTER: /* need to eat EOD */
	while (csi_file_getc (file) != EOF)
	    ;
	break;
    default:
	break;
    }
}

void
csi_file_close (csi_t *ctx, csi_file_t *file)
{
    if (file->src == NULL)
	return;

    switch (file->type) {
    case STDIO:
	if (file->flags & OWN_STREAM)
	    fclose (file->src);
	break;
    case BYTES:
	if (file->src != file->data) {
	    csi_string_t *src = file->src;
	    if (src != NULL && --src->base.ref == 0)
		csi_string_free (ctx, src);
	}
	break;
    case FILTER:
	{
	    csi_file_t *src = file->src;
	    if (src != NULL && --src->base.ref == 0)
		_csi_file_free (ctx, src);
	}
	break;
    case PROCEDURE:
    default:
	break;
    }
    file->src = NULL;
}

void
_csi_file_free (csi_t *ctx, csi_file_t *file)
{
    csi_file_flush (file);
    /* XXX putback */
    csi_file_close (ctx, file);

    switch (file->type) {
    case BYTES:
	break;
    case PROCEDURE:
#if 0
	csi_object_free (ctx, file->data);
#endif
	break;
    case STDIO:
	_csi_free (ctx, file->data);
	break;
    case FILTER:
	file->filter->filter_destroy (ctx, file->data);
	break;
    default:
	break;
    }

    _csi_slab_free (ctx, file, sizeof (csi_file_t));
}

csi_status_t
_csi_file_as_string (csi_t *ctx,
		     csi_file_t *file,
		     csi_object_t *obj)
{
    char *bytes;
    unsigned int len;
    unsigned int allocated;
    csi_status_t status;

    allocated = 16384;
    bytes = _csi_alloc (ctx, allocated);
    if (bytes == NULL)
	return _csi_error (CAIRO_STATUS_NO_MEMORY);

    len = 0;
    do {
	int ret;

	ret = csi_file_read (file, bytes + len, allocated - len);
	if (ret == 0)
	    break;

	len += ret;
	if (len + 1 > allocated / 2) {
	    char *newbytes;
	    int newlen;

	    if (_csi_unlikely (allocated > INT_MAX / 2))
		return _csi_error (CAIRO_STATUS_NO_MEMORY);

	    newlen = allocated * 2;
	    newbytes = _csi_realloc (ctx, bytes, newlen);
	    if (_csi_unlikely (newbytes == NULL)) {
		_csi_free (ctx, bytes);
		return _csi_error (CAIRO_STATUS_NO_MEMORY);
	    }
	    bytes = newbytes;
	    allocated = newlen;
	}
    } while (TRUE);

    bytes[len] = '\0'; /* better safe than sorry! */
    status = csi_string_new_from_bytes (ctx, obj, bytes, len);
    if (status) {
	_csi_free (ctx, bytes);
	return status;
    }

    return CAIRO_STATUS_SUCCESS;
}

