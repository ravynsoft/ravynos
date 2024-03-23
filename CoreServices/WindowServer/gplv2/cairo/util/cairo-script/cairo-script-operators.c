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

/* TODO real path type */

#include "cairo-script-private.h"

#if CAIRO_HAS_SCRIPT_SURFACE
#include "cairo-script.h"
#endif

#include <stdio.h> /* snprintf */
#include <stdlib.h> /* mkstemp */
#include <string.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES /* for M_LN2, M_PI and M_SQRT2 on win32 */
#define snprintf _snprintf
#endif

#include <math.h>
#include <limits.h> /* INT_MAX */
#include <assert.h>

#if HAVE_ZLIB
#include <zlib.h>
#endif

#if HAVE_LZO
#include <lzo/lzo2a.h>
#endif

#ifdef HAVE_MMAP
# ifdef HAVE_UNISTD_H
#  include <sys/mman.h>
#  include <unistd.h>
# else
#  undef HAVE_MMAP
# endif
#endif

typedef struct _csi_proxy {
    csi_t *ctx;
    void *ptr;
    csi_dictionary_t *dictionary;
    csi_destroy_func_t destroy_func;
    void *destroy_data;
} csi_proxy_t;

typedef struct _csi_blob {
    csi_list_t list;
    unsigned long hash;
    uint8_t *bytes;
    unsigned int len;
} csi_blob_t;

static const cairo_user_data_key_t _csi_proxy_key;
static const cairo_user_data_key_t _csi_blob_key;

enum mime_type {
    MIME_TYPE_NONE = 0,
    MIME_TYPE_PNG
};

#define check(CNT) do {\
    if (_csi_unlikely (! _csi_check_ostack (ctx, (CNT)))) \
	return _csi_error (CSI_STATUS_INVALID_SCRIPT); \
} while (0)
#define pop(CNT) _csi_pop_ostack (ctx, (CNT))
#define push(OBJ) _csi_push_ostack (ctx, (OBJ))

static csi_proxy_t *
_csi_proxy_create (csi_t *ctx,
		   void *ptr,
		   csi_dictionary_t *dictionary,
		   csi_destroy_func_t destroy_func,
		   void *destroy_data)
{
    csi_proxy_t *proxy;

    proxy = _csi_slab_alloc (ctx, sizeof (csi_proxy_t));
    if (proxy == NULL)
	return NULL;

    proxy->ctx = cairo_script_interpreter_reference (ctx);
    proxy->ptr = ptr;
    proxy->destroy_func = destroy_func;
    proxy->destroy_data = destroy_data;
    proxy->dictionary = dictionary;
    if (dictionary != NULL)
	dictionary->base.ref++;

    return proxy;
}

static void
_csi_proxy_destroy (void *closure)
{
    csi_proxy_t *proxy = closure;
    csi_t *ctx = proxy->ctx;

    /* XXX this doesn't work because user_data_destroy is called too late.
     * Considering another hook into the (cairo internal) object system.
     */
    if (proxy->destroy_func != NULL)
	proxy->destroy_func (proxy->destroy_data, proxy->ptr);

    if (proxy->dictionary != NULL && --proxy->dictionary->base.ref == 0)
	csi_dictionary_free (ctx, proxy->dictionary);

    _csi_slab_free (ctx, proxy, sizeof (csi_proxy_t));
    cairo_script_interpreter_destroy (ctx);
}

static void
_csi_blob_hash (csi_blob_t *blob, const uint32_t *data, int len)
{
    unsigned long hash = blob->hash;
    /* very simple! */
    while (len--) {
	unsigned long c = *data++;
	hash *= 33;
	hash ^= c;
    }
    blob->hash = hash;
}

static csi_boolean_t
_csi_blob_equal (const csi_list_t *link, void *data)
{
    csi_blob_t *A, *B;

    A = csi_container_of (link, csi_blob_t, list);
    B = data;

    if (A->len != B->len)
	return FALSE;

    if (A->hash != B->hash)
	return FALSE;

    return memcmp (A->bytes, B->bytes, A->len) == 0;
}

static void
_csi_blob_init (csi_blob_t *blob, uint8_t *bytes, int len)
{
    blob->hash = 5381;
    blob->len = len;
    blob->bytes = bytes;
}

static csi_list_t *
_csi_list_unlink (csi_list_t *head, csi_list_t *link)
{
    if (link->next != NULL)
	link->next->prev = link->prev;
    if (link->prev != NULL)
	link->prev->next = link->next;
    else
	head = link->next;
    return head;
}

static csi_list_t *
_csi_list_prepend (csi_list_t *head, csi_list_t *link)
{
    if (head != NULL)
	head->prev = link;
    link->next = head;
    link->prev = NULL;
    return link;
}

static csi_list_t *
_csi_list_find (csi_list_t *head,
		csi_boolean_t (*predicate) (const csi_list_t *link, void *data),
		void *data)
{
    while (head != NULL) {
	if (predicate (head, data))
	    return head;
	head = head->next;
    }

    return NULL;
}

static csi_status_t
_csi_ostack_get_boolean (csi_t *ctx, unsigned int i, csi_boolean_t *out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	*out = obj->datum.boolean;
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	*out = !! obj->datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	*out = obj->datum.real != 0.;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_integer (csi_t *ctx, unsigned int i, csi_integer_t *out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	*out = obj->datum.boolean;
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	*out = obj->datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	*out = obj->datum.real;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_number (csi_t *ctx, unsigned int i, double *out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	*out = obj->datum.boolean;
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	*out = obj->datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	*out = obj->datum.real;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static double
_csi_object_as_real (csi_object_t *obj)
{
    int type;

    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	return obj->datum.boolean;
    case CSI_OBJECT_TYPE_INTEGER:
	return obj->datum.integer;
    case CSI_OBJECT_TYPE_REAL:
	return obj->datum.real;
    default:
	return 0;
    }
}

static csi_status_t
_csi_ostack_get_name (csi_t *ctx, unsigned int i, csi_name_t *out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_NAME))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.name;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_context (csi_t *ctx, unsigned int i, cairo_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_CONTEXT))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.cr;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_font_face (csi_t *ctx, unsigned int i, cairo_font_face_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_FONT))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.font_face;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_pattern (csi_t *ctx, unsigned int i, cairo_pattern_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_PATTERN))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.pattern;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_scaled_font (csi_t *ctx, unsigned int i,
			     cairo_scaled_font_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely
	(csi_object_get_type (obj) != CSI_OBJECT_TYPE_SCALED_FONT))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    *out = obj->datum.scaled_font;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_surface (csi_t *ctx, unsigned int i, cairo_surface_t **out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	*out = cairo_get_target (obj->datum.cr);
	break;
    case CSI_OBJECT_TYPE_SURFACE:
	*out = obj->datum.surface;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_array (csi_t *ctx, unsigned int i, csi_array_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_ARRAY))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.array;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_procedure (csi_t *ctx, unsigned int i, csi_array_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (! csi_object_is_procedure (obj)))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.array;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_dictionary (csi_t *ctx, unsigned int i, csi_dictionary_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely
	(csi_object_get_type (obj) != CSI_OBJECT_TYPE_DICTIONARY))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    *out = obj->datum.dictionary;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_matrix (csi_t *ctx, unsigned int i, cairo_matrix_t *out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_MATRIX:
	*out = obj->datum.matrix->matrix;
	return CSI_STATUS_SUCCESS;

    case CSI_OBJECT_TYPE_ARRAY:
	if (obj->datum.array->stack.len == 6) {
	    cairo_matrix_init (out,
			       csi_number_get_value (&obj->datum.array->stack.objects[0]),
			       csi_number_get_value (&obj->datum.array->stack.objects[1]),
			       csi_number_get_value (&obj->datum.array->stack.objects[2]),
			       csi_number_get_value (&obj->datum.array->stack.objects[3]),
			       csi_number_get_value (&obj->datum.array->stack.objects[4]),
			       csi_number_get_value (&obj->datum.array->stack.objects[5]));
	    return CSI_STATUS_SUCCESS;
	}
	/* else fall through */
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_csi_dictionary_get_integer (csi_t *ctx,
			     csi_dictionary_t *dict,
			     const char *name,
			     csi_boolean_t optional,
			     long *value)
{
    csi_status_t status;
    csi_object_t key, obj;
    int type;

    status = csi_name_new_static (ctx, &key, name);
    if (_csi_unlikely (status))
	return status;

    if (optional && ! csi_dictionary_has (dict, key.datum.name))
	return CSI_STATUS_SUCCESS;

    status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
    if (_csi_unlikely (status))
	return status;

    type = csi_object_get_type (&obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	*value = obj.datum.boolean;
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	*value = obj.datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	*value = obj.datum.real;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_dictionary_get_number (csi_t *ctx,
			    csi_dictionary_t *dict,
			    const char *name,
			    csi_boolean_t optional,
			    double *value)
{
    csi_status_t status;
    csi_object_t key, obj;

    status = csi_name_new_static (ctx, &key, name);
    if (_csi_unlikely (status))
	return status;

    if (optional && ! csi_dictionary_has (dict, key.datum.name))
	return CSI_STATUS_SUCCESS;

    status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
    if (_csi_unlikely (status))
	return status;

    *value = csi_number_get_value (&obj);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_string (csi_t *ctx, unsigned int i, csi_string_t **out)
{
    csi_object_t *obj;

    obj = _csi_peek_ostack (ctx, i);
    if (_csi_unlikely (csi_object_get_type (obj) != CSI_OBJECT_TYPE_STRING))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *out = obj->datum.string;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_csi_ostack_get_string_constant (csi_t *ctx, unsigned int i, const char **out)
{
    csi_object_t *obj;
    int type;

    obj = _csi_peek_ostack (ctx, i);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_NAME:
	*out = (const char *) obj->datum.name;
	break;
    case CSI_OBJECT_TYPE_STRING:
	*out = obj->datum.string->string;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_do_cairo_op (csi_t *ctx, void (*op) (cairo_t *))
{
    cairo_t *cr;
    csi_status_t status;

    check (1);

    status = _csi_ostack_get_context (ctx, 0, &cr);
    if (_csi_unlikely (status))
	return status;

    op (cr);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
end_dict_construction (csi_t *ctx)
{
    csi_object_t obj;
    csi_dictionary_t *dict;
    csi_status_t status;

    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    dict = obj.datum.dictionary;
    do {
	csi_object_t *name, *value;

	check (1);

	value = _csi_peek_ostack (ctx, 0);
	if (csi_object_get_type (value) == CSI_OBJECT_TYPE_MARK) {
	    pop (1);
	    break;
	}

	check (2);

	name = _csi_peek_ostack (ctx, 1);
	if (_csi_unlikely
	    (csi_object_get_type (name) != CSI_OBJECT_TYPE_NAME))
	{
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	status = csi_dictionary_put (ctx, dict, name->datum.name, value);
	if (_csi_unlikely (status))
	    return status;

	pop (2);
    } while (TRUE);

    return push (&obj);
}

static csi_status_t
end_array_construction (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    int len = 0;

    do {
	check (len + 1);

	if (csi_object_get_type (_csi_peek_ostack (ctx, len)) ==
	    CSI_OBJECT_TYPE_MARK)
	{
	    break;
	}

	len++;
    } while (TRUE);

    status = csi_array_new (ctx, len, &obj);
    if (_csi_unlikely (status))
	return status;

    if (len != 0) {
	csi_array_t *array;

	array = obj.datum.array;
	memcpy (array->stack.objects,
		_csi_peek_ostack (ctx, len - 1),
		sizeof (csi_object_t) * len);
	array->stack.len = len;
    }
    ctx->ostack.len -= len + 1;

    return push (&obj);
}

static csi_status_t
_alpha (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double a;

    check (1);

    status = _csi_ostack_get_number (ctx, 0, &a);
    if (_csi_unlikely (status))
	return status;

    pop (1);

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_rgba (0, 0, 0, a);
    return push (&obj);
}

static csi_status_t
_add (csi_t *ctx)
{
    csi_object_t *A;
    csi_object_t *B;
    csi_object_type_t type_a, type_b;

    check (2);

    B = _csi_peek_ostack (ctx, 0);
    A = _csi_peek_ostack (ctx, 1);

    type_a = csi_object_get_type (A);
    if (_csi_unlikely (! (type_a == CSI_OBJECT_TYPE_INTEGER ||
			    type_a == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    type_b = csi_object_get_type (B);
    if (_csi_unlikely (! (type_b == CSI_OBJECT_TYPE_INTEGER ||
			    type_b == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);

    if (type_a == CSI_OBJECT_TYPE_REAL &&
	type_b == CSI_OBJECT_TYPE_REAL)
    {
	return _csi_push_ostack_real (ctx, A->datum.real + B->datum.real);

    }
    else if (type_a == CSI_OBJECT_TYPE_INTEGER &&
	     type_b == CSI_OBJECT_TYPE_INTEGER)
    {
	return _csi_push_ostack_integer (ctx,
					 A->datum.integer + B->datum.integer);
    }
    else
    {
	double v;

	if (type_a == CSI_OBJECT_TYPE_REAL)
	    v = A->datum.real;
	else
	    v = A->datum.integer;

	if (type_b == CSI_OBJECT_TYPE_REAL)
	    v += B->datum.real;
	else
	    v += B->datum.integer;

	return _csi_push_ostack_real (ctx, v);
    }
}

static csi_status_t
_add_color_stop (csi_t *ctx)
{
    csi_status_t status;
    double offset, r, g, b, a;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */

    check (6);

    status = _csi_ostack_get_number (ctx, 0, &a);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &offset);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_pattern (ctx, 5, &pattern);
    if (_csi_unlikely (status))
	return status;

    cairo_pattern_add_color_stop_rgba (pattern, offset, r, g, b, a);

    pop (5);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_and (csi_t *ctx)
{
    csi_object_t *a, *b;
    int type;

    check (2);

    a = _csi_peek_ostack (ctx, 0);
    b = _csi_peek_ostack (ctx, 1);
    if (_csi_unlikely (csi_object_get_type (a) != csi_object_get_type (b)))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    pop (2);
    type = csi_object_get_type (a);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	return _csi_push_ostack_integer (ctx,
					 a->datum.integer & b->datum.integer);
    case CSI_OBJECT_TYPE_BOOLEAN:
	return _csi_push_ostack_boolean (ctx,
					 a->datum.boolean & b->datum.boolean);
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_arc (csi_t *ctx)
{
    csi_status_t status;
    double x, y, r;
    double theta1, theta2;
    cairo_t *cr;

    check (6);

    status = _csi_ostack_get_number (ctx, 0, &theta2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &theta1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 5, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX handle path object */

    cairo_arc (cr, x, y, r, theta1, theta2);
    pop (5);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_arc_negative (csi_t *ctx)
{
    csi_status_t status;
    double x, y, r;
    double theta1, theta2;
    cairo_t *cr;

    check (6);

    status = _csi_ostack_get_number (ctx, 0, &theta2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &theta1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 5, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX handle path object */

    cairo_arc_negative (cr, x, y, r, theta1, theta2);
    pop (5);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_array (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;

    status = csi_array_new (ctx, 0, &obj);
    if (_csi_unlikely (status))
	return status;

    return push (&obj);
}

static csi_status_t
_bind_substitute (csi_t *ctx, csi_array_t *array)
{
    csi_status_t status;
    csi_integer_t i, n;
    csi_dictionary_t *dict;

    /* perform operator substitution on the executable array (procedure) */
    dict = ctx->dstack.objects[0].datum.dictionary;
    n = array->stack.len;
    for (i = 0; i < n; i++) {
	csi_object_t *obj = &array->stack.objects[i];

	if (obj->type == (CSI_OBJECT_TYPE_NAME | CSI_OBJECT_ATTR_EXECUTABLE)) {
	    csi_dictionary_entry_t *entry;

	    entry = _csi_hash_table_lookup (&dict->hash_table,
					    (csi_hash_entry_t *)
					    &obj->datum.name);
	    if (entry != NULL)
		*obj = entry->value;
	} else if (csi_object_is_procedure (obj)) {
	    status = _bind_substitute (ctx, obj->datum.array);
	    if (_csi_unlikely (status))
		return status;
	}
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_idiom_substitute (csi_t *ctx, csi_array_t *array)
{
#if 0
    csi_status_t status;
    csi_integer_t i, j;

    /* XXX substring search, build array once then search for
     * longest matching idiom, repeat. */

    /* scan the top-most array for sequences we can pre-compile */

    /* now recurse for subroutines */
    j = array->stack.len;
    for (i = 0; i < j; i++) {
	csi_object_t *obj = &array->stack.objects[i];

	if (csi_object_is_procedure (obj)) {
	    status = _idiom_substitute (ctx, obj->datum.array);
	    if (_csi_unlikely (_cairo_is_error (status))
		return status;
	}
    }
#endif

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_bind (csi_t *ctx)
{
    csi_array_t *array;
    csi_status_t status;

    check (1);

    status = _csi_ostack_get_procedure (ctx, 0, &array);
    if (_csi_unlikely (status))
	return status;

    status = _bind_substitute (ctx, array);
    if (_csi_unlikely (status))
	return status;

    status = _idiom_substitute (ctx, array);
    if (_csi_unlikely (status))
	return status;

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_bitshift (csi_t *ctx)
{
    long v, shift;
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &shift);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 1, &v);
    if (_csi_unlikely (status))
	return status;

    if (shift < 0) {
	shift = -shift;
	v >>= shift;
    } else
	v <<= shift;

    pop (1);
    _csi_peek_ostack (ctx, 0)->datum.integer = v;

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_clip (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_clip);
}

static csi_status_t
_clip_preserve (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_clip_preserve);
}

static csi_status_t
_close_path (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_close_path);
}

static csi_status_t
_context (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    cairo_surface_t *surface;
    cairo_t *cr;
    csi_context_create_func_t hook;
    csi_proxy_t *proxy;

    check (1);

    status = _csi_ostack_get_surface (ctx, 0, &surface);
    if (_csi_unlikely (status))
	return status;

    hook = ctx->hooks.context_create;
    if (hook != NULL)
	cr = hook (ctx->hooks.closure, surface);
    else
	cr = cairo_create (surface);

    proxy = _csi_proxy_create (ctx, cr, NULL,
			       ctx->hooks.context_destroy,
			       ctx->hooks.closure);
    if (_csi_unlikely (proxy == NULL)) {
	cairo_destroy (cr);
	return _csi_error (CSI_STATUS_NO_MEMORY);
    }

    status = cairo_set_user_data (cr, &_csi_proxy_key,
				  proxy, _csi_proxy_destroy);
    if (_csi_unlikely (status)) {
	_csi_proxy_destroy (proxy);
	cairo_destroy (cr);
	return status;
    }

    pop (1);
    obj.type = CSI_OBJECT_TYPE_CONTEXT;
    obj.datum.cr = cr;
    return push (&obj);
}

static csi_status_t
_copy (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = csi_object_reference (_csi_peek_ostack (ctx, 0));
    pop (1);

    type = csi_object_get_type (obj);
    switch (type) {
	/*XXX array, string, dictionary, etc */
    case CSI_OBJECT_TYPE_INTEGER:
	{
	    long i, n;

	    n = obj->datum.integer;
	    if (_csi_unlikely (n < 0))
		return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	    check (n);

	    for (i = n; i--; ) {
		csi_status_t status;

		status = _csi_push_ostack_copy (ctx,
						_csi_peek_ostack (ctx, n-1));
		if (_csi_unlikely (status))
		    return status;
	    }
	    break;
	}
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_copy_page (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_copy_page (obj->datum.cr);
	if (ctx->hooks.copy_page != NULL)
	    ctx->hooks.copy_page (ctx->hooks.closure, obj->datum.cr);
	break;
    case CSI_OBJECT_TYPE_SURFACE:
	cairo_surface_copy_page (obj->datum.surface);
	/* XXX hook? */
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_curve_to (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    int type;
    double x1, y1;
    double x2, y2;
    double x3, y3;

    check (7);

    status = _csi_ostack_get_number (ctx, 0, &y3);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x3);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &y2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &x2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &y1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 5, &x1);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 6);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_curve_to (obj->datum.cr, x1, y1, x2, y2, x3, y3);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_mesh_pattern_curve_to (obj->datum.pattern,
				     x1, y1, x2, y2, x3, y3);
	break;
	/* XXX handle path object */
    }

    pop (6);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_cvi (csi_t *ctx)
{
    csi_object_t *val, obj;
    int type;

    check (1);

    val = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (val);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	return CSI_STATUS_SUCCESS;

    case CSI_OBJECT_TYPE_REAL:
	pop (1);
	return _csi_push_ostack_integer (ctx, val->datum.real);

    case CSI_OBJECT_TYPE_STRING:
	if (! _csi_parse_number (&obj,
				 val->datum.string->string,
				 val->datum.string->len))
	{
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	pop (1);
	if (csi_object_get_type (&obj) == CSI_OBJECT_TYPE_INTEGER)
	    return push (&obj);
	else
	    return _csi_push_ostack_integer (ctx, obj.datum.real);

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_cvr (csi_t *ctx)
{
    csi_object_t *val, obj;
    int type;

    check (1);

    val = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (val);
    switch (type) {
    case CSI_OBJECT_TYPE_REAL:
	return CSI_STATUS_SUCCESS;

    case CSI_OBJECT_TYPE_INTEGER:
	pop (1);
	return _csi_push_ostack_real (ctx, val->datum.integer);

    case CSI_OBJECT_TYPE_STRING:
	if (! _csi_parse_number (&obj,
				 val->datum.string->string,
				 val->datum.string->len))
	{
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	pop (1);
	if (csi_object_get_type (&obj) == CSI_OBJECT_TYPE_REAL)
	    return push (&obj);
	else
	    return _csi_push_ostack_real (ctx, obj.datum.integer);

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_def (csi_t *ctx)
{
    csi_name_t name = 0; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_name (ctx, 1, &name);
    if (_csi_unlikely (status))
	return status;

    status = _csi_name_define (ctx, name, _csi_peek_ostack (ctx, 0));
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_dict (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;

    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    return push (&obj);
}

static csi_status_t
_div (csi_t *ctx)
{
    csi_object_t *A;
    csi_object_t *B;
    csi_object_type_t type_a, type_b;

    check (2);

    B = _csi_peek_ostack (ctx, 0);
    A = _csi_peek_ostack (ctx, 1);

    type_a = csi_object_get_type (A);
    if (_csi_unlikely (! (type_a == CSI_OBJECT_TYPE_INTEGER ||
			    type_a == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    type_b = csi_object_get_type (B);
    if (_csi_unlikely (! (type_b == CSI_OBJECT_TYPE_INTEGER ||
			    type_b == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);

    if (type_a == CSI_OBJECT_TYPE_REAL &&
	type_b == CSI_OBJECT_TYPE_REAL)
    {
	return _csi_push_ostack_real (ctx, A->datum.real / B->datum.real);

    }
    else if (type_a == CSI_OBJECT_TYPE_INTEGER &&
	     type_b == CSI_OBJECT_TYPE_INTEGER)
    {
	return _csi_push_ostack_integer (ctx,
					 A->datum.integer / B->datum.integer);
    }
    else
    {
	double v;

	if (type_a == CSI_OBJECT_TYPE_REAL)
	    v = A->datum.real;
	else
	    v = A->datum.integer;

	if (type_b == CSI_OBJECT_TYPE_REAL)
	    v /= B->datum.real;
	else
	    v /= B->datum.integer;

	return _csi_push_ostack_real (ctx, v);
    }
}

static csi_status_t
_duplicate (csi_t *ctx)
{
    check (1);

    return _csi_push_ostack_copy (ctx, _csi_peek_ostack (ctx, 0));
}

static csi_status_t
_eq (csi_t *ctx)
{
    csi_object_t *a, *b;
    csi_boolean_t v;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    v = csi_object_eq (a, b);

    pop (2);
    return _csi_push_ostack_boolean (ctx, v);
}

static csi_status_t
_exch (csi_t *ctx)
{
    return _csi_stack_exch (&ctx->ostack);
}

static csi_status_t
_false (csi_t *ctx)
{
    return _csi_push_ostack_boolean (ctx, FALSE);
}

static csi_status_t
_fill (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_fill);
}

static csi_status_t
_fill_preserve (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_fill_preserve);
}

static csi_status_t
_filter (csi_t *ctx)
{
    csi_object_t *src;
    csi_dictionary_t *dict = NULL;
    csi_status_t status;
    const char *name = NULL; /* silence the compiler */
    const struct filters {
	const char *name;
	csi_status_t (*constructor) (csi_t *t,
				       csi_object_t *,
				       csi_dictionary_t *,
				       csi_object_t *);
    } filters[] = {
	{ "ascii85", csi_file_new_ascii85_decode },
#if HAVE_ZLIB
	{ "deflate", csi_file_new_deflate_decode },
#endif
#if 0
	{ "lzw", csi_file_new_lzw_decode },
#endif
	{ NULL, NULL }
    }, *filter;
    int cnt;

    check (2);

    status = _csi_ostack_get_string_constant (ctx, 0, &name);
    if (_csi_unlikely (status))
	return status;

    src = _csi_peek_ostack (ctx, 1);
    cnt = 2;
    if (csi_object_get_type (src) == CSI_OBJECT_TYPE_DICTIONARY) {
	dict = src->datum.dictionary;

	check (3);

	src = _csi_peek_ostack (ctx, 2);
	cnt = 3;
    }

    for (filter = filters; filter->name != NULL; filter++) {
	if (strcmp (name, filter->name) == 0) {
	    csi_object_t file;

	    status = filter->constructor (ctx, &file, dict, src);
	    if (_csi_unlikely (status))
		return status;

	    pop (cnt);
	    return push (&file);
	}
    }

    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
}

static cairo_status_t
_type3_init (cairo_scaled_font_t *scaled_font,
	     cairo_t *cr,
	     cairo_font_extents_t *metrics)
{
    cairo_font_face_t *face;
    csi_proxy_t *proxy;
    csi_t *ctx;
    csi_dictionary_t *font;
    csi_object_t key;
    csi_object_t obj;
    csi_array_t *array;
    csi_status_t status;

    face = cairo_scaled_font_get_font_face (scaled_font);
    proxy = cairo_font_face_get_user_data (face, &_csi_proxy_key);
    if (_csi_unlikely (proxy == NULL))
	return CAIRO_STATUS_NO_MEMORY;

    ctx = proxy->ctx;
    font = proxy->dictionary;

    status = csi_name_new_static (ctx, &key, "metrics");
    if (_csi_unlikely (status))
	return CAIRO_STATUS_NO_MEMORY;

    if (! csi_dictionary_has (font, key.datum.name))
	return CAIRO_STATUS_SUCCESS;

    status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
    if (_csi_unlikely (status))
	return status;

    if (csi_object_get_type (&obj) != CSI_OBJECT_TYPE_ARRAY)
	return CAIRO_STATUS_USER_FONT_ERROR;

    array = obj.datum.array;
    if (array->stack.len != 5)
	return CAIRO_STATUS_USER_FONT_ERROR;

    metrics->ascent  = csi_number_get_value (&array->stack.objects[0]);
    metrics->descent = csi_number_get_value (&array->stack.objects[1]);
    metrics->height  = csi_number_get_value (&array->stack.objects[2]);
    metrics->max_x_advance = csi_number_get_value (&array->stack.objects[3]);
    metrics->max_y_advance = csi_number_get_value (&array->stack.objects[4]);

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_type3_lookup (cairo_scaled_font_t *scaled_font,
	       unsigned long unicode,
	       unsigned long *glyph)
{
    cairo_font_face_t *face;
    csi_proxy_t *proxy;
    csi_t *ctx;
    csi_dictionary_t *font;
    csi_object_t obj, key;
    csi_array_t *array;
    char buf[12];
    csi_integer_t i;
    cairo_status_t status;

    face = cairo_scaled_font_get_font_face (scaled_font);
    proxy = cairo_font_face_get_user_data (face, &_csi_proxy_key);
    if (_csi_unlikely (proxy == NULL))
	return CAIRO_STATUS_USER_FONT_ERROR;

    ctx = proxy->ctx;
    font = proxy->dictionary;

    status = csi_name_new_static (ctx, &key, "encoding");
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    if (! csi_dictionary_has (font, key.datum.name)) {
	*glyph = unicode;
	return CAIRO_STATUS_SUCCESS;
    }

    status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    if (_csi_unlikely (csi_object_get_type (&obj) != CSI_OBJECT_TYPE_ARRAY))
	return CAIRO_STATUS_USER_FONT_ERROR;

    snprintf (buf, sizeof (buf), "uni%04lu", unicode);
    array = obj.datum.array;
    for (i = 0; i < array->stack.len; i++) {
	csi_object_t *name;

	name = &array->stack.objects[i];
	if (csi_object_get_type (name) != CSI_OBJECT_TYPE_NAME)
	    continue;

	if (strcmp ((char *) name->datum.name, buf) == 0) {
	    *glyph = i;
	    return CAIRO_STATUS_SUCCESS;
	}
    }

    return CAIRO_STATUS_USER_FONT_ERROR;
}

static cairo_status_t
_type3_render (cairo_scaled_font_t *scaled_font,
	       unsigned long glyph_index,
	       cairo_t *cr,
	       cairo_text_extents_t *metrics)
{
    cairo_font_face_t *face;
    csi_proxy_t *proxy;
    csi_t *ctx;
    csi_dictionary_t *font;
    csi_array_t *glyphs;
    csi_object_t *glyph;
    csi_object_t key;
    csi_object_t obj;
    csi_object_t render;
    csi_status_t status;

    face = cairo_scaled_font_get_font_face (scaled_font);
    proxy = cairo_font_face_get_user_data (face, &_csi_proxy_key);
    if (_csi_unlikely (proxy == NULL))
	return CAIRO_STATUS_USER_FONT_ERROR;

    ctx = proxy->ctx;
    font = proxy->dictionary;

    status = csi_name_new_static (ctx, &key, "glyphs");
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    if (_csi_unlikely (csi_object_get_type (&obj) != CSI_OBJECT_TYPE_ARRAY))
	return CAIRO_STATUS_USER_FONT_ERROR;

    glyphs = obj.datum.array;
    glyph = &glyphs->stack.objects[glyph_index];
    if (csi_object_get_type (glyph) == CSI_OBJECT_TYPE_NULL)
	return CAIRO_STATUS_SUCCESS; /* .notdef */

    if (_csi_unlikely (csi_object_get_type (glyph) != CSI_OBJECT_TYPE_DICTIONARY))
	return CAIRO_STATUS_USER_FONT_ERROR;

    status = csi_name_new_static (ctx, &key, "metrics");
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    font = glyph->datum.dictionary;
    if (csi_dictionary_has (font, key.datum.name)) {
	csi_array_t *array;

	status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return CAIRO_STATUS_USER_FONT_ERROR;

	if (_csi_unlikely (csi_object_get_type (&obj) !=
			     CSI_OBJECT_TYPE_ARRAY))
	    return CAIRO_STATUS_USER_FONT_ERROR;

	array = obj.datum.array;
	if (_csi_unlikely (array->stack.len != 6))
	    return CAIRO_STATUS_USER_FONT_ERROR;

	metrics->x_bearing = csi_number_get_value (&array->stack.objects[0]);
	metrics->y_bearing = csi_number_get_value (&array->stack.objects[1]);
	metrics->width = csi_number_get_value (&array->stack.objects[2]);
	metrics->height = csi_number_get_value (&array->stack.objects[3]);
	metrics->x_advance = csi_number_get_value (&array->stack.objects[4]);
	metrics->y_advance = csi_number_get_value (&array->stack.objects[5]);
    }

    status = csi_name_new_static (ctx, &key, "render");
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    status = csi_dictionary_get (ctx, font, key.datum.name, &render);
    if (_csi_unlikely (status))
	return CAIRO_STATUS_USER_FONT_ERROR;

    if (_csi_unlikely (! csi_object_is_procedure (&render)))
	return CAIRO_STATUS_USER_FONT_ERROR;

    obj.type = CSI_OBJECT_TYPE_CONTEXT;
    obj.datum.cr = cairo_reference (cr);
    status = push (&obj);
    if (_csi_unlikely (status)) {
	cairo_destroy (cr);
	return CAIRO_STATUS_USER_FONT_ERROR;
    }

    status = csi_object_execute (ctx, &render);
    pop (1);
    return status ? CAIRO_STATUS_USER_FONT_ERROR : CAIRO_STATUS_SUCCESS;
}

static csi_status_t
_font_type3 (csi_t *ctx,
	     csi_dictionary_t *font,
	     cairo_font_face_t **font_face_out)
{
    cairo_font_face_t *font_face;

    font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func (font_face, _type3_init);
    cairo_user_font_face_set_unicode_to_glyph_func (font_face, _type3_lookup);
    cairo_user_font_face_set_render_glyph_func (font_face, _type3_render);

    *font_face_out = font_face;
    return CSI_STATUS_SUCCESS;
}

#if CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library _ft_lib;

struct _ft_face_data {
    csi_t *ctx;
    csi_blob_t blob;
    FT_Face face;
    csi_string_t *source;
    void *bytes;
    cairo_font_face_t *font_face;
};

static void
_ft_done_face (void *closure)
{
    struct _ft_face_data *data = closure;
    csi_t *ctx;

    ctx = data->ctx;

    if (data->face != NULL)
	FT_Done_Face (data->face);

    ctx->_faces = _csi_list_unlink (ctx->_faces, &data->blob.list);

    if (data->source != NULL) {
	if (--data->source->base.ref == 0)
	    csi_string_free (ctx, data->source);
    } else {
#ifdef HAVE_MMAP
	munmap (data->blob.bytes, data->blob.len);
#endif
    }

    if (data->bytes != NULL)
	_csi_free (ctx, data->bytes);

    _csi_slab_free (ctx, data, sizeof (*data));

    cairo_script_interpreter_destroy (ctx);
}

struct mmap_vec {
    const uint8_t *bytes;
    size_t num_bytes;
};

#ifdef HAVE_MMAP
/* manual form of swapping for swapless systems like tiny */
static void *
_mmap_bytes (const struct mmap_vec *vec, int count)
{
    char template[] = "/tmp/csi-font.XXXXXX";
    void *ptr;
    int fd;
    int num_bytes;

    fd = mkstemp (template);
    if (fd == -1)
	return MAP_FAILED;

    unlink (template);
    num_bytes = 0;
    while (count--) {
	const uint8_t *bytes = vec->bytes;
	size_t len = vec->num_bytes;
	while (len) {
	    int ret = write (fd, bytes, len);
	    if (ret < 0) {
		close (fd);
		return MAP_FAILED;
	    }
	    len -= ret;
	    bytes += ret;
	}

	num_bytes += vec->num_bytes;
	vec++;
    }

    ptr = mmap (NULL, num_bytes, PROT_READ, MAP_SHARED, fd, 0);
    close (fd);

    return ptr;
}
#endif

static void *
inflate_string (csi_t *ctx, csi_string_t *src)
{
    uLongf len;
    uint8_t *bytes;

    len = src->deflate;
    bytes = _csi_alloc (ctx, len + 1);
    if (bytes == NULL)
	return NULL;

    switch (src->method) {
    default:
    case NONE:
	free (bytes);
	return NULL;

    case ZLIB:
#if HAVE_ZLIB
	if (uncompress ((Bytef *) bytes, &len,
			(Bytef *) src->string, src->len) != Z_OK)
#endif
	{
	    _csi_free (ctx, bytes);
	    return NULL;
	}
	break;

    case LZO:
#if HAVE_LZO
	if (lzo2a_decompress ((Bytef *) src->string, src->len,
			      (Bytef *) bytes, &len,
			      NULL))
#endif
	{
	    _csi_free (ctx, bytes);
	    return NULL;
	}
	break;
    }

    bytes[len] = '\0';
    return bytes;
}

static csi_status_t
_ft_create_for_source (csi_t *ctx,
		       csi_string_t *source,
		       int index, int load_flags,
		       cairo_font_face_t **font_face_out)
{
    csi_blob_t tmpl;
    struct _ft_face_data *data;
    csi_list_t *link;
    FT_Error err;
    cairo_font_face_t *font_face;
    csi_status_t status;
    struct mmap_vec vec[2];
    int vec_count;
    void *bytes;
    int len;

    /* check for an existing FT_Face (kept alive by the font cache) */
    /* XXX index/flags */
    _csi_blob_init (&tmpl, (uint8_t *) source->string, source->len);
    _csi_blob_hash (&tmpl, (uint32_t *) source->string, source->len / sizeof (uint32_t));
    link = _csi_list_find (ctx->_faces, _csi_blob_equal, &tmpl);
    if (link) {
	if (--source->base.ref == 0)
	    csi_string_free (ctx, source);
	data = csi_container_of (link, struct _ft_face_data, blob.list);
	*font_face_out = cairo_font_face_reference (data->font_face);
	return CSI_STATUS_SUCCESS;
    }

    /* no existing font_face, create new FT_Face */
    if (_ft_lib == NULL) {
	err = FT_Init_FreeType (&_ft_lib);
	if (_csi_unlikely (err != FT_Err_Ok))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    }

    data = _csi_slab_alloc (ctx, sizeof (*data));
    data->bytes = NULL;
    data->source = source;

    vec[0].bytes = tmpl.bytes;
    vec[0].num_bytes = tmpl.len;

    if (source->deflate) {
	len = source->deflate;
	bytes = inflate_string (ctx, source);
	if (_csi_unlikely (bytes == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	vec[1].bytes = bytes;
	vec[1].num_bytes = len;
	data->bytes = bytes;
	vec_count = 2;
    } else {
	bytes = tmpl.bytes;
	len = tmpl.len;
	vec_count = 1;
    }

    data->face = NULL;
    ctx->_faces = _csi_list_prepend (ctx->_faces, &data->blob.list);
    data->ctx = cairo_script_interpreter_reference (ctx);
    data->blob.hash = tmpl.hash;
    data->blob.len = tmpl.len;
#ifdef HAVE_MMAP
    data->blob.bytes = _mmap_bytes (vec, vec_count);
    if (data->blob.bytes != MAP_FAILED) {
	if (--source->base.ref == 0)
	    csi_string_free (ctx, source);

	if (source->deflate) {
	    _csi_free (ctx, bytes);
	    bytes = data->blob.bytes + vec[0].num_bytes;
	} else
	    bytes = data->blob.bytes;

	data->source = NULL;
	data->bytes = NULL;
    } else {
	data->blob.bytes = tmpl.bytes;
    }
#else
    data->blob.bytes = tmpl.bytes;
#endif

    err = FT_New_Memory_Face (_ft_lib,
			      bytes, len,
			      index,
			      &data->face);
    if (_csi_unlikely (err != FT_Err_Ok)) {
	_ft_done_face (data);

	if (err == FT_Err_Out_Of_Memory)
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    font_face = cairo_ft_font_face_create_for_ft_face (data->face, load_flags);
    status = cairo_font_face_set_user_data (font_face,
					    &_csi_blob_key,
					    data, _ft_done_face);
    if (_csi_unlikely (status)) {
	_ft_done_face (data);
	cairo_font_face_destroy (font_face);
	return status;
    }

    data->font_face = font_face;
    *font_face_out = font_face;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_ft_create_for_pattern (csi_t *ctx,
			csi_string_t *string,
			cairo_font_face_t **font_face_out)
{
#if CAIRO_HAS_FC_FONT
    csi_blob_t tmpl;
    struct _ft_face_data *data;
    csi_list_t *link;
    cairo_font_face_t *font_face;
    FcPattern *pattern, *resolved;
    csi_status_t status;
    struct mmap_vec vec;
    void *bytes;

    _csi_blob_init (&tmpl, (uint8_t *) string->string, string->len);
    _csi_blob_hash (&tmpl, (uint32_t *) string->string, string->len / sizeof (uint32_t));
    link = _csi_list_find (ctx->_faces, _csi_blob_equal, &tmpl);
    if (link) {
	if (--string->base.ref == 0)
	    csi_string_free (ctx, string);
	data = csi_container_of (link, struct _ft_face_data, blob.list);
	*font_face_out = cairo_font_face_reference (data->font_face);
	return CSI_STATUS_SUCCESS;
    }

    if (string->deflate) {
	bytes = inflate_string (ctx, string);
	if (_csi_unlikely (bytes == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    } else {
	bytes = tmpl.bytes;
    }

    pattern = FcNameParse (bytes);
    if (!pattern)
    {
      /* Fontconfig's representation of charset changed mid 2014;
       * We used to record charset before that.  Remove everything
       * after charset if that's present, and try again.  */
      char *s = strstr ((char *) bytes, ":charset=");
      if (s)
      {
	*s = '\0';
	pattern = FcNameParse (bytes);
      }
    }
    if (bytes != tmpl.bytes)
	_csi_free (ctx, bytes);

retry:
    resolved = pattern;
    if (cairo_version () < CAIRO_VERSION_ENCODE (1, 9, 0)) {
	/* prior to 1.9, you needed to pass a resolved pattern */
	resolved = FcFontMatch (NULL, pattern, NULL);
	if (_csi_unlikely (resolved == NULL)) {
	    FcPatternDestroy (pattern);
	    return _csi_error (CSI_STATUS_NO_MEMORY);
	}
    }

    font_face = cairo_ft_font_face_create_for_pattern (resolved);
    if (resolved != pattern)
	FcPatternDestroy (resolved);

    if (cairo_font_face_status (font_face)) {
	char *filename = NULL;

	/* Try a manual fallback process by eliminating specific requests */

	if (FcPatternGetString (pattern,
				FC_FILE, 0,
				(FcChar8 **) &filename) == FcResultMatch) {
	    FcPatternDel (pattern, FC_FILE);
	    goto retry;
	}
    }

    FcPatternDestroy (pattern);

    data = _csi_slab_alloc (ctx, sizeof (*data));
    ctx->_faces = _csi_list_prepend (ctx->_faces, &data->blob.list);
    data->ctx = cairo_script_interpreter_reference (ctx);
    data->blob.hash = tmpl.hash;
    data->blob.len = tmpl.len;
    data->bytes = NULL;
    data->face = NULL;
#ifdef HAVE_MMAP
    vec.bytes = tmpl.bytes;
    vec.num_bytes = tmpl.len;
    data->blob.bytes = _mmap_bytes (&vec, 1);
    if (data->blob.bytes != MAP_FAILED) {
	data->source = NULL;
	if (--string->base.ref == 0)
	    csi_string_free (ctx, string);
    } else {
	data->blob.bytes = tmpl.bytes;
	data->source = string;
    }
#else
    data->blob.bytes = tmpl.bytes;
    data->source = string;
#endif

    status = cairo_font_face_set_user_data (font_face,
					    &_csi_blob_key,
					    data, _ft_done_face);
    if (_csi_unlikely (status)) {
	_ft_done_face (data);
	cairo_font_face_destroy (font_face);
	return status;
    }

    data->font_face = font_face;
    *font_face_out = font_face;
    return CSI_STATUS_SUCCESS;
#else
    if (--string->base.ref == 0)
	csi_string_free (ctx, string);
    return CSI_INT_STATUS_UNSUPPORTED;
#endif
}

static csi_status_t
_ft_type42_create (csi_t *ctx,
		   csi_dictionary_t *font,
		   cairo_font_face_t **font_face_out)
{
    csi_object_t key;
    csi_status_t status;

    /* two basic sub-types, either an FcPattern or embedded font */
    status = csi_name_new_static (ctx, &key, "pattern");
    if (_csi_unlikely (status))
	return status;

    if (csi_dictionary_has (font, key.datum.name)) {
	csi_object_t obj;
	int type;

	status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;

	type = csi_object_get_type (&obj);
	switch (type) {
	case CSI_OBJECT_TYPE_FILE:
	    status = _csi_file_as_string (ctx, obj.datum.file, &obj);
	    if (_csi_unlikely (status))
		return status;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    obj.datum.object->ref++;
	    break;
	default:
	    return  _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	return _ft_create_for_pattern (ctx,
				       obj.datum.string,
				       font_face_out);
    }

    status = csi_name_new_static (ctx, &key, "source");
    if (_csi_unlikely (status))
	return status;

    if (csi_dictionary_has (font, key.datum.name)) {
	csi_object_t obj;
	long index, flags;
	int type;

	index = 0;
	status = _csi_dictionary_get_integer (ctx, font, "index", TRUE, &index);
	if (_csi_unlikely (status))
	    return status;

	flags = 0;
	status = _csi_dictionary_get_integer (ctx, font, "flags", TRUE, &flags);
	if (_csi_unlikely (status))
	    return status;

	status = csi_name_new_static (ctx, &key, "source");
	if (_csi_unlikely (status))
	    return status;
	status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;
	type = csi_object_get_type (&obj);
	switch (type) {
	case CSI_OBJECT_TYPE_FILE:
	    status = _csi_file_as_string (ctx, obj.datum.file, &obj);
	    if (_csi_unlikely (status))
		return status;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    obj.datum.object->ref++;
	    break;
	default:
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	return _ft_create_for_source (ctx, obj.datum.string,
				      index, flags,
				      font_face_out);
    }

    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
}
#else
#define _ft_type42_create(ctx, font, face_out) CSI_INT_STATUS_UNSUPPORTED
#endif

static char *
_fc_strcpy (csi_t *ctx, const char *str)
{
    char *ret;
    int len;

    ret = strchr (str, ':');
    if (ret != NULL)
	len = ret - str;
    else
	len = strlen (str);

    ret = _csi_alloc (ctx, len+1);
    if (_csi_unlikely (ret == NULL))
	return NULL;

    memcpy (ret, str, len);
    ret[len] = '\0';

    return ret;
}

static cairo_font_face_t *
_select_font (const char *name)
{
    cairo_surface_t *surface;
    cairo_font_face_t *face;
    cairo_t *cr;

    /* create a dummy context to choose a font */
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_select_font_face (cr, name,
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    face = cairo_font_face_reference (cairo_get_font_face (cr));
    cairo_destroy (cr);

    return face;
}

static csi_status_t
_ft_fallback_create_for_pattern (csi_t *ctx,
				 csi_string_t *string,
				 cairo_font_face_t **font_face_out)
{
    char *str, *name;

    str = string->string;
#if 0
    name = strstr (str, "fullname=");
    if (name != NULL)
	str = name + 9;
#endif

    name = _fc_strcpy (ctx, str);
    if (_csi_unlikely (name == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    *font_face_out = _select_font (name);
    _csi_free (ctx, name);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_ft_type42_fallback_create (csi_t *ctx,
			    csi_dictionary_t *font,
			    cairo_font_face_t **font_face_out)
{
    csi_object_t key;
    csi_status_t status;

    /* attempt to select a similar font */

    /* two basic sub-types, either an FcPattern or embedded font */
    status = csi_name_new_static (ctx, &key, "pattern");
    if (_csi_unlikely (status))
	return status;

    if (csi_dictionary_has (font, key.datum.name)) {
	csi_object_t obj;
	int type;

	status = csi_dictionary_get (ctx, font, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;

	type = csi_object_get_type (&obj);
	switch (type) {
	case CSI_OBJECT_TYPE_FILE:
	    status = _csi_file_as_string (ctx, obj.datum.file, &obj);
	    if (_csi_unlikely (status))
		return status;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    obj.datum.object->ref++;
	    break;
	default:
	    return  _csi_error (CSI_STATUS_INVALID_SCRIPT);
	}

	return _ft_fallback_create_for_pattern (ctx,
						obj.datum.string,
						font_face_out);
    }

    /* XXX: enable the trace to run */
    *font_face_out = _select_font ("Sans");
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_font_type42 (csi_t *ctx, csi_dictionary_t *font, cairo_font_face_t **font_face)
{
    csi_status_t status;

    status = _ft_type42_create (ctx, font, font_face);
    if (_csi_likely (status != CSI_INT_STATUS_UNSUPPORTED))
	return status;

    return _ft_type42_fallback_create (ctx, font, font_face);
}

static csi_status_t
_font (csi_t *ctx)
{
    csi_dictionary_t *font;
    csi_status_t status;
    cairo_font_face_t *font_face = NULL; /* silence the compiler */
    csi_proxy_t *proxy;
    csi_object_t obj;
    long type;

    check (1);

    status = _csi_ostack_get_dictionary (ctx, 0, &font);
    if (_csi_unlikely (status))
	return status;

    status = _csi_dictionary_get_integer (ctx, font, "type", FALSE, &type);
    if (_csi_unlikely (status))
	return status;

    switch (type) {
    case 3:
	status = _font_type3 (ctx, font, &font_face);
	break;
    case 42:
	status = _font_type42 (ctx, font, &font_face);
	break;
    default:
	status = _csi_error (CSI_STATUS_INVALID_SCRIPT);
	break;
    }

    if (_csi_unlikely (status))
	return status;

    /* transfer ownership of dictionary to cairo_font_face_t */
    proxy = _csi_proxy_create (ctx, font_face, font, NULL, NULL);
    if (_csi_unlikely (proxy == NULL)) {
	cairo_font_face_destroy (font_face);
	return _csi_error (CSI_STATUS_NO_MEMORY);
    }

    status = cairo_font_face_set_user_data (font_face,
					    &_csi_proxy_key,
					    proxy, _csi_proxy_destroy);
    if (_csi_unlikely (status)) {
	_csi_proxy_destroy (proxy);
	cairo_font_face_destroy (font_face);
	return status;
    }

    obj.type = CSI_OBJECT_TYPE_FONT;
    obj.datum.font_face = font_face;

    pop (1);
    status = push (&obj);
    if (_csi_unlikely (status)) {
	cairo_font_face_destroy (font_face);
	return status;
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_for (csi_t *ctx)
{
    csi_array_t *proc;
    csi_status_t status;
    long i, inc, limit;

    check (4);

    status = _csi_ostack_get_procedure (ctx, 0, &proc);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 1, &limit);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 2, &inc);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 3, &i);
    if (_csi_unlikely (status))
	return status;

    proc->base.ref++;
    pop (4);

    for (; i <= limit; i += inc) {
	status = _csi_push_ostack_integer (ctx, i);
	if (_csi_unlikely (status))
	    break;

	status = _csi_array_execute (ctx, proc);
	if (_csi_unlikely (status))
	    break;
    }

    if (--proc->base.ref == 0)
	csi_array_free (ctx, proc);
    return status;
}

static csi_status_t
_ge (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *a, *b;
    int cmp;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    status = csi_object_compare (a, b, &cmp);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return _csi_push_ostack_boolean (ctx, cmp >= 0);
}

static csi_status_t
_proxy_get (csi_proxy_t *proxy,
	    csi_name_t key)
{
    csi_object_t obj;
    csi_status_t status;

    if (_csi_unlikely (proxy == NULL || proxy->dictionary == NULL))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    status = csi_dictionary_get (proxy->ctx, proxy->dictionary, key, &obj);
    if (_csi_unlikely (status))
	return status;

    return _csi_push_ostack_copy (proxy->ctx, &obj);
}

static csi_status_t
_context_get (csi_t *ctx,
	      cairo_t *cr,
	      csi_name_t key)
{
    csi_status_t status;
    csi_object_t obj;

    if (strcmp ((char *) key, "current-point") == 0) {
	double x, y;

	cairo_get_current_point (cr, &x, &y);

	status = _csi_push_ostack_real (ctx, x);
	if (_csi_unlikely (status))
	    return status;
	status = _csi_push_ostack_real (ctx, y);
	if (_csi_unlikely (status))
	    return status;

	return CSI_STATUS_SUCCESS;
    } else if (strcmp ((char *) key, "source") == 0) {
	obj.type = CSI_OBJECT_TYPE_PATTERN;
	obj.datum.pattern = cairo_pattern_reference (cairo_get_source (cr));
    } else if (strcmp ((char *) key, "target") == 0) {
	obj.type = CSI_OBJECT_TYPE_SURFACE;
	obj.datum.surface = cairo_surface_reference (cairo_get_target (cr));
    } else if (strcmp ((char *) key, "group-target") == 0) {
	obj.type = CSI_OBJECT_TYPE_SURFACE;
	obj.datum.surface = cairo_surface_reference (cairo_get_group_target (cr));
    } else if (strcmp ((char *) key, "scaled-font") == 0) {
	obj.type = CSI_OBJECT_TYPE_SCALED_FONT;
	obj.datum.scaled_font = cairo_scaled_font_reference (cairo_get_scaled_font (cr));
    } else if (strcmp ((char *) key, "font-face") == 0) {
	obj.type = CSI_OBJECT_TYPE_FONT;
	obj.datum.font_face = cairo_font_face_reference (cairo_get_font_face (cr));
    } else
	return _proxy_get (cairo_get_user_data (cr, &_csi_proxy_key), key);

    return push (&obj);
}

static csi_status_t
_font_get (csi_t *ctx,
	   cairo_font_face_t *font_face,
	   csi_name_t key)
{
    return _proxy_get (cairo_font_face_get_user_data (font_face,
						      &_csi_proxy_key),
		       key);
}

static csi_status_t
_pattern_get (csi_t *ctx,
	      cairo_pattern_t *pattern,
	      csi_name_t key)
{
    csi_status_t status;

    if (strcmp ((char *) key, "type") == 0)
	return _csi_push_ostack_integer (ctx, cairo_pattern_get_type (pattern));

    if (strcmp ((char *) key, "filter") == 0)
	return _csi_push_ostack_integer (ctx, cairo_pattern_get_filter (pattern));

    if (strcmp ((char *) key, "extend") == 0)
	return _csi_push_ostack_integer (ctx, cairo_pattern_get_extend (pattern));

    if (strcmp ((char *) key, "matrix") == 0) {
	csi_object_t obj;
	cairo_matrix_t m;

	cairo_pattern_get_matrix (pattern, &m);
	status = csi_matrix_new_from_matrix (ctx, &obj, &m);
	if (_csi_unlikely (status))
	    return status;

	return push (&obj);
    }

    return _proxy_get (cairo_pattern_get_user_data (pattern, &_csi_proxy_key),
		       key);
}

static csi_status_t
_scaled_font_get (csi_t *ctx,
		  cairo_scaled_font_t *font,
		  csi_name_t key)
{
    return _proxy_get (cairo_scaled_font_get_user_data (font, &_csi_proxy_key),
		       key);
}

static csi_status_t
_surface_get (csi_t *ctx,
	      cairo_surface_t *surface,
	      csi_name_t key)
{
    if (strcmp ((char *) key, "type") == 0) {
	return _csi_push_ostack_integer (ctx, cairo_surface_get_type (surface));
    }

    if (strcmp ((char *) key, "content") == 0) {
	return _csi_push_ostack_integer (ctx,
					 cairo_surface_get_content (surface));
    }

    return _proxy_get (cairo_surface_get_user_data (surface, &_csi_proxy_key),
		       key);
}

static csi_status_t
_get (csi_t *ctx)
{
    csi_object_t *key, *src, obj;
    csi_status_t status;
    int type;

    check (2);

    key = _csi_peek_ostack (ctx, 0);
    src = _csi_peek_ostack (ctx, 1);
    pop (1);
    type = csi_object_get_type (src);
    switch (type) {
    case CSI_OBJECT_TYPE_DICTIONARY:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	status = csi_dictionary_get (ctx,
				     src->datum.dictionary,
				     key->datum.name,
				     &obj);
	break;
    case CSI_OBJECT_TYPE_ARRAY:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_INTEGER))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	status = csi_array_get (ctx,
				src->datum.array,
				key->datum.integer,
				&obj);
	break;
#if 0
    case CSI_OBJECT_TYPE_STRING:
	status = csi_string_get (src, key, &obj);
	break;
#endif

    case CSI_OBJECT_TYPE_CONTEXT:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	return _context_get (ctx, src->datum.cr, key->datum.name);

    case CSI_OBJECT_TYPE_FONT:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	return _font_get (ctx, src->datum.font_face, key->datum.name);

    case CSI_OBJECT_TYPE_PATTERN:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	return _pattern_get (ctx, src->datum.pattern, key->datum.name);

    case CSI_OBJECT_TYPE_SCALED_FONT:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	return _scaled_font_get (ctx, src->datum.scaled_font, key->datum.name);

    case CSI_OBJECT_TYPE_SURFACE:
	if (_csi_unlikely (csi_object_get_type (key) != CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	return _surface_get (ctx, src->datum.surface, key->datum.name);

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    if (_csi_unlikely (status))
	return status;

    return _csi_push_ostack_copy (ctx, &obj);
}

struct glyph_advance_cache {
    csi_t *ctx;
    double glyph_advance[256][2];
    unsigned long have_glyph_advance[256];
};

static void
glyph_advance_cache_destroy (void *closure)
{
    struct glyph_advance_cache *cache = closure;
    _csi_free (cache->ctx, cache);
}

static int
_glyph_string (csi_t *ctx,
	       csi_array_t *array,
	       cairo_scaled_font_t *scaled_font,
	       cairo_glyph_t *glyphs)
{
    struct glyph_advance_cache uncached;
    struct glyph_advance_cache *cache;
    csi_integer_t nglyphs, i, j;
    double x, y, dx;
    cairo_status_t status;

    if (cairo_scaled_font_status (scaled_font))
	return 0;

    cache = cairo_scaled_font_get_user_data (scaled_font,
					     (cairo_user_data_key_t *) ctx);
    if (cache == NULL) {
	cache = _csi_alloc (ctx, sizeof (*cache));
	if (_csi_likely (cache != NULL)) {
	    cache->ctx = ctx;
	    memset (cache->have_glyph_advance, 0xff,
		    sizeof (cache->have_glyph_advance));

	    status = cairo_scaled_font_set_user_data (scaled_font,
						      (cairo_user_data_key_t *) ctx,
						      cache,
						      glyph_advance_cache_destroy);
	    if (_csi_unlikely (status)) {
		_csi_free (ctx, cache);
		cache = NULL;
	    }
	}
    }

    if (_csi_unlikely (cache == NULL)) {
	cache = &uncached;

	cache->ctx = ctx;
	memset (cache->have_glyph_advance, 0xff,
		sizeof (cache->have_glyph_advance));
    }

    nglyphs = 0;
    x = y = 0;
    for (i = 0; i < array->stack.len; i++) {
	const csi_object_t *obj = &array->stack.objects[i];
	int type = csi_object_get_type (obj);

	switch (type) {
	case CSI_OBJECT_TYPE_ARRAY: {
	    const csi_array_t *glyph_array = obj->datum.array;
	    for (j = 0; j < glyph_array->stack.len; j++) {
		unsigned long g;
		int gi;

		obj = &glyph_array->stack.objects[j];
		if (csi_object_get_type (obj) != CSI_OBJECT_TYPE_INTEGER)
		    break;
		g = obj->datum.integer;

		glyphs[nglyphs].index = g;
		glyphs[nglyphs].x = x;
		glyphs[nglyphs].y = y;

		gi = g % ARRAY_LENGTH (cache->have_glyph_advance);
		if (cache->have_glyph_advance[gi] != g) {
		    cairo_text_extents_t extents;

		    cairo_scaled_font_glyph_extents (scaled_font,
						     &glyphs[nglyphs], 1,
						     &extents);

		    cache->glyph_advance[gi][0] = extents.x_advance;
		    cache->glyph_advance[gi][1] = extents.y_advance;
		    cache->have_glyph_advance[gi] = g;
		}

		x += cache->glyph_advance[gi][0];
		y += cache->glyph_advance[gi][1];
		nglyphs++;
	    }
	    break;
	}

	case CSI_OBJECT_TYPE_STRING: {
	    const csi_string_t *glyph_string = obj->datum.string;
	    for (j = 0; j < glyph_string->len; j++) {
		uint8_t g;

		g = glyph_string->string[j];
		glyphs[nglyphs].index = g;
		glyphs[nglyphs].x = x;
		glyphs[nglyphs].y = y;

		if (cache->have_glyph_advance[g] != g) {
		    cairo_text_extents_t extents;

		    cairo_scaled_font_glyph_extents (scaled_font,
						     &glyphs[nglyphs], 1,
						     &extents);

		    cache->glyph_advance[g][0] = extents.x_advance;
		    cache->glyph_advance[g][1] = extents.y_advance;
		    cache->have_glyph_advance[g] = g;
		}

		x += cache->glyph_advance[g][0];
		y += cache->glyph_advance[g][1];
		nglyphs++;
	    }
	    break;
	}

	case CSI_OBJECT_TYPE_INTEGER:
	case CSI_OBJECT_TYPE_REAL: /* dx or x*/
	    dx = csi_number_get_value (obj);
	    if (i+1 == array->stack.len)
		break;

	    type = csi_object_get_type (&array->stack.objects[i+1]);
	    switch (type) {
	    case CSI_OBJECT_TYPE_INTEGER:
	    case CSI_OBJECT_TYPE_REAL: /* y */
		y = csi_number_get_value (&array->stack.objects[i+1]);
		x = dx;
		i++;
		break;

	    default:
		x += dx;
	    }
	}
    }

    return nglyphs;
}

static csi_status_t
_glyph_path (csi_t *ctx)
{
    csi_array_t *array;
    csi_status_t status;
    cairo_t *cr;
    cairo_glyph_t stack_glyphs[256], *glyphs;
    csi_integer_t nglyphs, i;

    check (2);

    status = _csi_ostack_get_array (ctx, 0, &array);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    /* count glyphs */
    nglyphs = 0;
    for (i = 0; i < array->stack.len; i++) {
	csi_object_t *obj = &array->stack.objects[i];
	int type = csi_object_get_type (obj);
	switch (type) {
	case CSI_OBJECT_TYPE_ARRAY:
	    nglyphs += obj->datum.array->stack.len;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    nglyphs += obj->datum.string->len;
	    break;
	}
    }
    if (nglyphs == 0) {
	pop (1);
	return CSI_STATUS_SUCCESS;
    }

    if (nglyphs > ARRAY_LENGTH (stack_glyphs)) {
	if (_csi_unlikely ((unsigned) nglyphs >= INT_MAX / sizeof (cairo_glyph_t)))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	glyphs = _csi_alloc (ctx, sizeof (cairo_glyph_t) * nglyphs);
	if (_csi_unlikely (glyphs == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    } else
	glyphs = stack_glyphs;

    nglyphs = _glyph_string (ctx, array, cairo_get_scaled_font (cr), glyphs);
    cairo_glyph_path (cr, glyphs, nglyphs);

    if (glyphs != stack_glyphs)
	_csi_free (ctx, glyphs);

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_gray (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double g;

    check (1);

    status = _csi_ostack_get_number (ctx, 0, &g);
    if (_csi_unlikely (status))
	return status;

    pop (1);

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_rgba (g, g, g, 1);
    return push (&obj);
}

static csi_status_t
_gt (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *a, *b;
    int cmp;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    status = csi_object_compare (a, b, &cmp);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return _csi_push_ostack_boolean (ctx, cmp > 0);
}

static csi_status_t
_identity (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;

    status = csi_matrix_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    return push (&obj);
}

static csi_status_t
_if (csi_t *ctx)
{
    csi_array_t *proc;
    csi_boolean_t predicate = FALSE; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_procedure (ctx, 0, &proc);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_boolean (ctx, 1, &predicate);
    if (_csi_unlikely (status))
	return status;

    proc->base.ref++;
    pop (2);

    if (predicate)
	status = _csi_array_execute (ctx, proc);

    if (--proc->base.ref == 0)
	csi_array_free (ctx, proc);

    return status;
}

static csi_status_t
_ifelse (csi_t *ctx)
{
    csi_array_t *true_proc, *false_proc;
    csi_boolean_t predicate = FALSE; /* silence the compiler */
    csi_status_t status;

    check (3);

    status = _csi_ostack_get_procedure (ctx, 0, &false_proc);
    if (_csi_unlikely (status))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    status = _csi_ostack_get_procedure (ctx, 1, &true_proc);
    if (_csi_unlikely (status))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    status = _csi_ostack_get_boolean (ctx, 2, &predicate);
    if (_csi_unlikely (status))
	return status;

    true_proc->base.ref++;
    false_proc->base.ref++;
    pop (3);

    if (predicate)
	status = _csi_array_execute (ctx, true_proc);
    else
	status = _csi_array_execute (ctx, false_proc);

    if (--true_proc->base.ref == 0)
	csi_array_free (ctx, true_proc);
    if (--false_proc->base.ref == 0)
	csi_array_free (ctx, false_proc);

    return status;
}

static csi_status_t
_image_read_raw (csi_t *ctx,
		 csi_object_t *src,
		 cairo_format_t format,
		 int width, int height,
		 cairo_surface_t **image_out)
{
    cairo_surface_t *image;
    uint8_t *bp, *data;
    int rem, len, ret, x, rowlen, instride, stride;
    cairo_status_t status;

    if (width == 0 || height == 0) {
	*image_out = cairo_image_surface_create (format, 0, 0);
	return CSI_STATUS_SUCCESS;
    }

    if (ctx->hooks.create_source_image != NULL) {
	image = ctx->hooks.create_source_image (ctx->hooks.closure,
						format, width, height,
						0);

	stride = cairo_image_surface_get_stride (image);
	data = cairo_image_surface_get_data (image);
    } else {
	stride = cairo_format_stride_for_width (format, width);
	data = malloc (stride * height);
	if (data == NULL)
	    return CAIRO_STATUS_NO_MEMORY;

	image = cairo_image_surface_create_for_data (data, format,
						     width, height, stride);
	status = cairo_surface_set_user_data (image,
					      (const cairo_user_data_key_t *) image,
					      data, free);
	if (status) {
	    cairo_surface_destroy (image);
	    free (image);
	    return status;
	}
    }

    switch (format) {
    case CAIRO_FORMAT_A1:
	instride = rowlen = (width+7)/8;
	break;
    case CAIRO_FORMAT_A8:
	instride = rowlen = width;
	break;
    case CAIRO_FORMAT_RGB16_565:
	instride = rowlen = 2 * width;
	break;
    case CAIRO_FORMAT_RGB24:
	rowlen = 3 * width;
	instride = 4 *width;
	break;
    default:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_INVALID:
    case CAIRO_FORMAT_ARGB32:
	instride = rowlen = 4 * width;
	break;
    case CAIRO_FORMAT_RGB96F:
	instride = rowlen = 12 * width;
	break;
    case CAIRO_FORMAT_RGBA128F:
	instride = rowlen = 16 * width;
	break;
    }
    len = rowlen * height;

    if (rowlen == instride &&
	src->type == CSI_OBJECT_TYPE_STRING &&
	len == src->datum.string->deflate)
    {
	csi_string_t *s = src->datum.string;
	unsigned long out = s->deflate;

	switch (s->method) {
	default:
	case NONE:
err_decompress:
	    cairo_surface_destroy (image);
	    return _csi_error (CSI_STATUS_READ_ERROR);

	case ZLIB:
#if HAVE_ZLIB
	    if (uncompress ((Bytef *) data, &out,
			    (Bytef *) s->string, s->len) != Z_OK)
#endif
		goto err_decompress;
	    break;

	case LZO:
#if HAVE_LZO
	    if (lzo2a_decompress ((Bytef *) s->string, s->len,
				  (Bytef *) data, &out,
				  NULL))
#endif
		goto err_decompress;
	    break;
	}
    }
    else
    {
	csi_object_t file;

	status = csi_object_as_file (ctx, src, &file);
	if (_csi_unlikely (status)) {
	    cairo_surface_destroy (image);
	    return status;
	}

	bp = data;
	rem = len;
	while (rem) {
	    ret = csi_file_read (file.datum.file, bp, rem);
	    if (_csi_unlikely (ret == 0)) {
		cairo_surface_destroy (image);
		return _csi_error (CSI_STATUS_READ_ERROR);
	    }
	    rem -= ret;
	    bp += ret;
	}

	if (len != height * stride) {
	    while (--height) {
		uint8_t *row = data + height * stride;

		/* XXX pixel conversion */
		switch (format) {
		case CAIRO_FORMAT_A1:
		    for (x = rowlen; x--; ) {
			uint8_t byte = *--bp;
			row[x] = CSI_BITSWAP8_IF_LITTLE_ENDIAN (byte);
		    }
		    break;
		case CAIRO_FORMAT_A8:
		    for (x = width; x--; )
			row[x] = *--bp;
		    break;
		case CAIRO_FORMAT_RGB16_565:
		    for (x = width; x--; ) {
#ifdef WORDS_BIGENDIAN
			row[2*x + 1] = *--bp;
			row[2*x + 0] = *--bp;
#else
			row[2*x + 0] = *--bp;
			row[2*x + 1] = *--bp;
#endif
		    }
		    break;
		case CAIRO_FORMAT_RGB24:
		    for (x = width; x--; ) {
#ifdef WORDS_BIGENDIAN
			row[4*x + 3] = *--bp;
			row[4*x + 2] = *--bp;
			row[4*x + 1] = *--bp;
			row[4*x + 0] = 0xff;
#else
			row[4*x + 0] = *--bp;
			row[4*x + 1] = *--bp;
			row[4*x + 2] = *--bp;
			row[4*x + 3] = 0xff;
#endif
		    }
		    break;
		case CAIRO_FORMAT_RGB96F:
		case CAIRO_FORMAT_RGBA128F:
		case CAIRO_FORMAT_RGB30:
		case CAIRO_FORMAT_INVALID:
		case CAIRO_FORMAT_ARGB32:
		    /* stride == width */
		    break;
		}

		memset (row + instride, 0, stride - instride);
	    }

	    /* need to treat last row carefully */
	    switch (format) {
	    case CAIRO_FORMAT_A1:
		for (x = rowlen; x--; ) {
		    uint8_t byte = *--bp;
		    data[x] = CSI_BITSWAP8_IF_LITTLE_ENDIAN (byte);
		}
		break;
	    case CAIRO_FORMAT_A8:
		for (x = width; x--; )
		    data[x] = *--bp;
		break;
	    case CAIRO_FORMAT_RGB16_565:
		for (x = width; x--; ) {
#ifdef WORDS_BIGENDIAN
		    data[2*x + 1] = *--bp;
		    data[2*x + 0] = *--bp;
#else
		    data[2*x + 0] = *--bp;
		    data[2*x + 1] = *--bp;
#endif
		}
		break;
	    case CAIRO_FORMAT_RGB24:
		for (x = width; --x>1; ) {
#ifdef WORDS_BIGENDIAN
		    data[4*x + 3] = *--bp;
		    data[4*x + 2] = *--bp;
		    data[4*x + 1] = *--bp;
		    data[4*x + 0] = 0xff;
#else
		    data[4*x + 0] = *--bp;
		    data[4*x + 1] = *--bp;
		    data[4*x + 2] = *--bp;
		    data[4*x + 3] = 0xff;
#endif
		}
		if (width > 1) {
		    uint8_t rgb[2][3];
		    /* shuffle the last couple of overlapping pixels */
		    rgb[1][0] = data[5];
		    rgb[1][1] = data[4];
		    rgb[1][2] = data[3];
		    rgb[0][0] = data[2];
		    rgb[0][1] = data[1];
		    rgb[0][2] = data[0];
#ifdef WORDS_BIGENDIAN
		    data[4] = 0xff;
		    data[5] = rgb[1][2];
		    data[6] = rgb[1][1];
		    data[7] = rgb[1][0];
		    data[0] = 0xff;
		    data[1] = rgb[0][2];
		    data[2] = rgb[0][1];
		    data[3] = rgb[0][0];
#else
		    data[7] = 0xff;
		    data[6] = rgb[1][2];
		    data[5] = rgb[1][1];
		    data[4] = rgb[1][0];
		    data[3] = 0xff;
		    data[2] = rgb[0][2];
		    data[1] = rgb[0][1];
		    data[0] = rgb[0][0];
#endif
		} else {
#ifdef WORDS_BIGENDIAN
		    data[0] = 0xff;
		    data[1] = data[0];
		    data[2] = data[1];
		    data[3] = data[2];
#else
		    data[3] = data[0];
		    data[0] = data[2];
		    data[2] = data[3];
		    data[3] = 0xff;
#endif
		}
		break;
	    case CAIRO_FORMAT_RGBA128F:
	    case CAIRO_FORMAT_RGB96F:
	    case CAIRO_FORMAT_RGB30:
	    case CAIRO_FORMAT_INVALID:
	    case CAIRO_FORMAT_ARGB32:
		/* stride == width */
		break;
	    }
	    memset (data + instride, 0, stride - instride);
	} else {
#ifndef WORDS_BIGENDIAN
	    switch (format) {
	    case CAIRO_FORMAT_A1:
		for (x = 0; x < len; x++) {
		    uint8_t byte = data[x];
		    data[x] = CSI_BITSWAP8_IF_LITTLE_ENDIAN (byte);
		}
		break;
	    case CAIRO_FORMAT_RGB16_565:
		{
		    uint32_t *rgba = (uint32_t *) data;
		    for (x = len/2; x--; rgba++) {
			*rgba = bswap_16 (*rgba);
		    }
		}
		break;
	    case CAIRO_FORMAT_ARGB32:
		{
		    uint32_t *rgba = (uint32_t *) data;
		    for (x = len/4; x--; rgba++) {
			*rgba = bswap_32 (*rgba);
		    }
		}
		break;

	    case CAIRO_FORMAT_A8:
		break;

	    case CAIRO_FORMAT_RGBA128F:
	    case CAIRO_FORMAT_RGB96F:
	    case CAIRO_FORMAT_RGB30:
	    case CAIRO_FORMAT_RGB24:
	    case CAIRO_FORMAT_INVALID:
	    default:
		break;
	    }
#endif
	}
	csi_object_free (ctx, &file);
    }

    cairo_surface_mark_dirty (image);
    *image_out = image;
    return CSI_STATUS_SUCCESS;
}

static cairo_status_t
png_read_func (void *closure, uint8_t *data, unsigned int len)
{
    int ret;

    ret = csi_file_read (closure, data, len);
    if ((unsigned int) ret != len)
	return CAIRO_STATUS_READ_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

static csi_status_t
_image_read_png (csi_file_t *src, cairo_surface_t **out)
{
#if CAIRO_HAS_PNG_FUNCTIONS
    *out = cairo_image_surface_create_from_png_stream (png_read_func, src);
    return cairo_surface_status (*out);
#else
    return CAIRO_STATUS_READ_ERROR;
#endif
}

struct _image_tag {
    csi_t *ctx;
    csi_blob_t blob;
    cairo_surface_t *surface;
};

static void
_image_tag_done (void *closure)
{
    struct _image_tag *tag = closure;
    csi_t *ctx = tag->ctx;

    ctx->_images = _csi_list_unlink (ctx->_images, &tag->blob.list);
    _csi_slab_free (ctx, tag, sizeof (*tag));
    cairo_script_interpreter_destroy (ctx);
}

static void
_image_hash (csi_blob_t *blob,
	     cairo_surface_t *surface)
{
    uint32_t  value;

    value = cairo_image_surface_get_width (surface);
    _csi_blob_hash (blob, &value, 1);

    value = cairo_image_surface_get_height (surface);
    _csi_blob_hash (blob, &value, 1);

    value = cairo_image_surface_get_format (surface);
    _csi_blob_hash (blob, &value, 1);
}

static cairo_surface_t *
_image_cached (csi_t *ctx, cairo_surface_t *surface)
{
    csi_blob_t tmpl;
    csi_list_t *link;
    uint8_t *data;
    int stride, height;
    struct _image_tag *tag;

    /* check for an existing image  */

    data = cairo_image_surface_get_data (surface);
    stride = cairo_image_surface_get_stride (surface);
    height = cairo_image_surface_get_height (surface);
    _csi_blob_init (&tmpl, data, stride * height);
    _image_hash (&tmpl, surface);
    link = _csi_list_find (ctx->_images, _csi_blob_equal, &tmpl);
    if (link) {
	cairo_surface_destroy (surface);
	tag = csi_container_of (link, struct _image_tag, blob.list);
	return cairo_surface_reference (tag->surface);
    }

    /* none found, insert a tag for this one */

    tag = _csi_slab_alloc (ctx, sizeof (struct _image_tag));
    if (tag == NULL)
	return surface;

    ctx->_images = _csi_list_prepend (ctx->_images, &tag->blob.list);
    tag->ctx = cairo_script_interpreter_reference (ctx);
    tag->blob.hash = tmpl.hash;
    tag->blob.bytes = tmpl.bytes;
    tag->blob.len = tmpl.len;
    tag->surface = surface;

    if (cairo_surface_set_user_data (surface, &_csi_blob_key,
				     tag, _image_tag_done))
    {
	_image_tag_done (tag);
    }

    return surface;
}

static csi_status_t
_image_load_from_dictionary (csi_t *ctx,
			     csi_dictionary_t *dict,
			     cairo_surface_t **image_out)
{
    csi_object_t obj, key;
    long width;
    long height;
    long format;
    cairo_surface_t *image = NULL; /* silence the compiler */
    csi_status_t status;

    /* check for "status? */

    status = _csi_dictionary_get_integer (ctx, dict, "width", FALSE, &width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_dictionary_get_integer (ctx, dict, "height", FALSE, &height);
    if (_csi_unlikely (status))
	return status;

    format = CAIRO_FORMAT_ARGB32;
    status = _csi_dictionary_get_integer (ctx, dict, "format", TRUE, &format);
    if (_csi_unlikely (status))
	return status;

    status = csi_name_new_static (ctx, &key, "source");
    if (_csi_unlikely (status))
	return status;

    if (csi_dictionary_has (dict, key.datum.name)) {
	enum mime_type mime_type;
	csi_object_t file;

	status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;

	status = csi_name_new_static (ctx, &key, "mime-type");
	if (_csi_unlikely (status))
	    return status;

	mime_type = MIME_TYPE_NONE;
	if (csi_dictionary_has (dict, key.datum.name)) {
	    csi_object_t type_obj;
	    const char *type_str;
	    int type;

	    status = csi_dictionary_get (ctx, dict, key.datum.name, &type_obj);
	    if (_csi_unlikely (status))
		return status;

	    type = csi_object_get_type (&type_obj);
	    switch (type) {
	    case CSI_OBJECT_TYPE_STRING:
		type_str = type_obj.datum.string->string;
		break;
	    case CSI_OBJECT_TYPE_NAME:
		type_str = (char *) type_obj.datum.name;
		break;
	    default:
		return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	    }

	    if (strcmp (type_str, CAIRO_MIME_TYPE_PNG) == 0)
		mime_type = MIME_TYPE_PNG;
	}


	/* XXX hook for general mime-type decoder */

	switch (mime_type) {
	case MIME_TYPE_NONE:
	    status = _image_read_raw (ctx, &obj, format, width, height, &image);
	    break;
	case MIME_TYPE_PNG:
	    status = csi_object_as_file (ctx, &obj, &file);
	    if (_csi_unlikely (status))
		return status;

	    status = _image_read_png (file.datum.file, &image);
	    csi_object_free (ctx, &file);
	    break;
	}
	if (_csi_unlikely (status))
	    return status;

	image = _image_cached (ctx, image);
    } else
	image = cairo_image_surface_create (format, width, height);

    *image_out = image;
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_image (csi_t *ctx)
{
    csi_dictionary_t *dict;
    cairo_surface_t *image;
    csi_status_t status;
    csi_object_t obj;

    check (1);

    status = _csi_ostack_get_dictionary (ctx, 0, &dict);
    if (_csi_unlikely (status))
	return status;

    status = _image_load_from_dictionary (ctx, dict, &image);
    if (_csi_unlikely (status))
	return status;

    pop (1);
    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = image;
    return push (&obj);
}

static csi_status_t
_index (csi_t *ctx)
{
    csi_status_t status;
    long n;

    check (1);

    status = _csi_ostack_get_integer (ctx, 0,  &n);
    if (_csi_unlikely (status))
	return status;

    pop (1);

    check (n);
    return _csi_push_ostack_copy (ctx, _csi_peek_ostack (ctx, n));
}

static csi_status_t
_integer (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	break;
    case CSI_OBJECT_TYPE_REAL:
	obj->datum.integer = obj->datum.real;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    obj->type = CSI_OBJECT_TYPE_INTEGER;

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_invert (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    cairo_matrix_t m;

    check (1);

    status = _csi_ostack_get_matrix (ctx, 0, &m);
    if (_csi_unlikely (status))
	return status;

    cairo_matrix_invert (&m);

    status = csi_matrix_new_from_matrix (ctx, &obj, &m);
    if (_csi_unlikely (status))
	return status;

    pop (1);

    return push (&obj);
}

static csi_status_t
_le (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *a, *b;
    int cmp;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    status = csi_object_compare (a, b, &cmp);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return _csi_push_ostack_boolean (ctx, cmp <= 0);
}

static csi_status_t
_linear (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double x1, y1, x2, y2;

    check (4);

    status = _csi_ostack_get_number (ctx, 0, &y2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &y1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &x1);
    if (_csi_unlikely (status))
	return status;

    pop (4);

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_linear (x1, y1, x2, y2);
    return push (&obj);
}

static csi_status_t
_line_to (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    int type;
    double x, y;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;

    /* XXX path object */

    obj = _csi_peek_ostack (ctx, 2);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_line_to (obj->datum.cr, x, y);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_mesh_pattern_line_to (obj->datum.pattern, x, y);
	break;
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_lt (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *a, *b;
    int cmp;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    status = csi_object_compare (a, b, &cmp);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return _csi_push_ostack_boolean (ctx, cmp < 0);
}

static csi_status_t
_mark (csi_t *ctx)
{
    return _csi_push_ostack_mark (ctx);
}

static csi_status_t
_ne (csi_t *ctx)
{
    csi_object_t *a, *b;
    csi_boolean_t v;

    check (2);

    b = _csi_peek_ostack (ctx, 0);
    a = _csi_peek_ostack (ctx, 1);

    v = ! csi_object_eq (a, b);

    pop (2);
    return _csi_push_ostack_boolean (ctx, v);
}

static csi_status_t
_neg (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	obj->datum.integer = -obj->datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	obj->datum.real = -obj->datum.real;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_not (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
	obj->datum.boolean = ! obj->datum.boolean;
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	obj->type = CSI_OBJECT_TYPE_BOOLEAN;
	obj->datum.boolean = ! obj->datum.integer;
	break;
    case CSI_OBJECT_TYPE_REAL:
	obj->type = CSI_OBJECT_TYPE_BOOLEAN;
	obj->datum.boolean = obj->datum.real == 0.0;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_new_path (csi_t *ctx)
{
    /* XXX handle path object */
    return _do_cairo_op (ctx, cairo_new_path);
}

static csi_status_t
_new_sub_path (csi_t *ctx)
{
    /* XXX handle path object */
    return _do_cairo_op (ctx, cairo_new_sub_path);
}

static csi_status_t
_null (csi_t *ctx)
{
    return _csi_push_ostack_null (ctx);
}

static csi_status_t
_mask (csi_t *ctx)
{
    cairo_t *cr;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_pattern (ctx, 0, &pattern);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_mask (cr, pattern);
    pop (1);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_matrix (csi_t *ctx)
{
    csi_object_t *obj, matrix;
    double v[6];
    csi_status_t status;
    int n;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    if (csi_object_is_number (obj)) {
	check (6);

	for (n = 6; n--; ) {
	    status = _csi_ostack_get_number (ctx, 5-n, &v[n]);
	    if (_csi_unlikely (status))
		return status;
	}
	status = csi_matrix_new_from_values (ctx, &matrix, v);
	if (_csi_unlikely (status))
	    return status;

	pop (6);
    } else {
	csi_array_t *array;

	status = _csi_ostack_get_array (ctx, 0, &array);
	if (_csi_unlikely (status))
	    return status;

	status = csi_matrix_new_from_array (ctx, &matrix, array);
	if (_csi_unlikely (status))
	    return status;

	pop (1);
    }

    return push (&matrix);
}

static csi_status_t
_map_to_image (csi_t *ctx)
{
    csi_object_t obj;
    csi_array_t *array;
    csi_status_t status;
    cairo_rectangle_int_t extents, *r;
    cairo_surface_t *surface;

    check (2);

    status = _csi_ostack_get_array (ctx, 0, &array);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_surface (ctx, 1, &surface);
    if (_csi_unlikely (status))
	return status;

    switch (array->stack.len) {
    case 0:
	r = NULL;
	break;
    case 4:
	extents.x = floor (_csi_object_as_real (&array->stack.objects[0]));
	extents.y = floor (_csi_object_as_real (&array->stack.objects[1]));
	extents.width = ceil (_csi_object_as_real (&array->stack.objects[2]));
	extents.height = ceil (_csi_object_as_real (&array->stack.objects[3]));
	r = &extents;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = cairo_surface_reference (cairo_surface_map_to_image (surface, r));
    pop (1);
    return push (&obj);
}

static csi_status_t
_unmap_image (csi_t *ctx)
{
    cairo_surface_t *surface, *image;
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_surface (ctx, 0, &image);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 1, &surface);
    if (_csi_unlikely (status))
	return status;

    cairo_surface_unmap_image (surface, image);

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mesh (csi_t *ctx)
{
    csi_object_t obj;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_mesh ();
    return push (&obj);
}

static csi_status_t
_mesh_begin_patch (csi_t *ctx)
{
    csi_status_t status;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */

    check (1);

    status = _csi_ostack_get_pattern (ctx, 0, &pattern);
    if (_csi_unlikely (status))
	return status;

    cairo_mesh_pattern_begin_patch (pattern);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mesh_end_patch (csi_t *ctx)
{
    csi_status_t status;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */

    check (1);

    status = _csi_ostack_get_pattern (ctx, 0, &pattern);
    if (_csi_unlikely (status))
	return status;

    cairo_mesh_pattern_end_patch (pattern);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mesh_set_control_point (csi_t *ctx)
{
    csi_status_t status;
    double x, y;
    csi_integer_t point;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */

    check (4);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 2, &point);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_pattern (ctx, 3, &pattern);
    if (_csi_unlikely (status))
	return status;

    cairo_mesh_pattern_set_control_point (pattern, point, x, y);

    pop (3);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mesh_set_corner_color (csi_t *ctx)
{
    csi_status_t status;
    double r, g, b, a;
    csi_integer_t corner;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */

    check (6);

    status = _csi_ostack_get_number (ctx, 0, &a);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 4, &corner);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_pattern (ctx, 5, &pattern);
    if (_csi_unlikely (status))
	return status;

    cairo_mesh_pattern_set_corner_color_rgba (pattern, corner, r, g, b, a);

    pop (5);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mod (csi_t *ctx)
{
    csi_integer_t x, y;
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    return _csi_push_ostack_integer (ctx, x % y);
}

static csi_status_t
_move_to (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    int type;
    double x, y;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 2);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_move_to (obj->datum.cr, x, y);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_mesh_pattern_move_to (obj->datum.pattern, x, y);
	break;

	/* XXX path object */
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_mul (csi_t *ctx)
{
    csi_object_t *A;
    csi_object_t *B;
    csi_object_type_t type_a, type_b;

    check (2);

    B = _csi_peek_ostack (ctx, 0);
    A = _csi_peek_ostack (ctx, 1);

    type_a = csi_object_get_type (A);
    if (_csi_unlikely (! (type_a == CSI_OBJECT_TYPE_INTEGER ||
			    type_a == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
    type_b = csi_object_get_type (B);
    if (_csi_unlikely (! (type_b == CSI_OBJECT_TYPE_INTEGER ||
			    type_b == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);

    if (type_a == CSI_OBJECT_TYPE_REAL &&
	type_b == CSI_OBJECT_TYPE_REAL)
    {
	return _csi_push_ostack_real (ctx, A->datum.real * B->datum.real);

    }
    else if (type_a == CSI_OBJECT_TYPE_INTEGER &&
	     type_b == CSI_OBJECT_TYPE_INTEGER)
    {
	return _csi_push_ostack_integer (ctx,
					 A->datum.integer * B->datum.integer);
    }
    else
    {
	double v;

	if (type_a == CSI_OBJECT_TYPE_REAL)
	    v = A->datum.real;
	else
	    v = A->datum.integer;

	if (type_b == CSI_OBJECT_TYPE_REAL)
	    v *= B->datum.real;
	else
	    v *= B->datum.integer;

	return _csi_push_ostack_real (ctx, v);
    }
}

static csi_status_t
_or (csi_t *ctx)
{
    csi_object_t *a, *b;
    int type;

    check (2);

    a = _csi_peek_ostack (ctx, 0);
    b = _csi_peek_ostack (ctx, 1);
    if (_csi_unlikely (csi_object_get_type (a) != csi_object_get_type (b)))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    pop (2);
    type = csi_object_get_type (a);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	return _csi_push_ostack_integer (ctx,
					 a->datum.integer | b->datum.integer);
    case CSI_OBJECT_TYPE_BOOLEAN:
	return _csi_push_ostack_boolean (ctx,
					 a->datum.boolean | b->datum.boolean);
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_paint (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_paint);
}

static csi_status_t
_paint_with_alpha (csi_t *ctx)
{
    cairo_t *cr;
    csi_status_t status;
    double alpha;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &alpha);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_paint_with_alpha (cr, alpha);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_pattern (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    cairo_surface_t *surface;

    check (1);

    status = _csi_ostack_get_surface (ctx, 0, &surface);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_for_surface (surface);

    pop (1);
    return push (&obj);
}

static csi_status_t
_pop (csi_t *ctx)
{
    check (1);
    pop (1);

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_pop_group (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    cairo_t *cr;

    check (1);

    status = _csi_ostack_get_context (ctx, 0, &cr);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pop_group (cr);

    return push (&obj);
}

static csi_status_t
_push_group (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    long content;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &content);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_push_group_with_content (cr, content);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_radial (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double x1, y1, r1, x2, y2, r2;

    check (6);

    status = _csi_ostack_get_number (ctx, 0, &r2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &y2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &x2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &r1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &y1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 5, &x1);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_radial (x1, y1, r1, x2, y2, r2);
    pop (6);
    return push (&obj);
}

static csi_status_t
_rectangle (csi_t *ctx)
{
    csi_status_t status;
    double x, y;
    double w, h;
    cairo_t *cr;

    check (5);

    status = _csi_ostack_get_number (ctx, 0, &h);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &w);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 4, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX path object */

    cairo_rectangle (cr, x, y, w, h);
    pop(4);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_rel_curve_to (csi_t *ctx)
{
    csi_status_t status;
    double x1, y1;
    double x2, y2;
    double x3, y3;
    cairo_t *cr;

    check (7);

    status = _csi_ostack_get_number (ctx, 0, &y3);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x3);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &y2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &x2);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 4, &y1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 5, &x1);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 6, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX path object */

    cairo_rel_curve_to (cr, x1, y1, x2, y2, x3, y3);
    pop (6);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_rel_line_to (csi_t *ctx)
{
    csi_status_t status;
    double x, y;
    cairo_t *cr;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 2, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX path object */

    cairo_rel_line_to (cr, x, y);
    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_rel_move_to (csi_t *ctx)
{
    csi_status_t status;
    double x, y;
    cairo_t *cr;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 2, &cr);
    if (_csi_unlikely (status))
	return status;

    /* XXX path object */
    cairo_rel_move_to (cr, x, y);
    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_repeat (csi_t *ctx)
{
    csi_array_t *proc;
    csi_integer_t count;
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_procedure (ctx, 0, &proc);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_integer (ctx, 1, &count);
    if (_csi_unlikely (status))
	return status;

    if (_csi_unlikely (count < 0))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    proc->base.ref++;
    pop (2);

    while (count--) {
	status = _csi_array_execute (ctx, proc);
	if (_csi_unlikely (status))
	    break;
    }

    if (--proc->base.ref == 0)
	csi_array_free (ctx, proc);

    return status;
}

static csi_status_t
_reset_clip (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_reset_clip);
}

static csi_status_t
_restore (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_restore);
}

static csi_status_t
_rgb (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double r,g,b;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &r);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_rgb (r, g, b);
    pop (3);
    return push (&obj);
}

static csi_status_t
_rgba (csi_t *ctx)
{
    csi_object_t obj;
    csi_status_t status;
    double r,g,b,a;

    check (4);

    status = _csi_ostack_get_number (ctx, 0, &a);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &r);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_PATTERN;
    obj.datum.pattern = cairo_pattern_create_rgba (r, g, b, a);
    pop (4);
    return push (&obj);
}

static csi_status_t
_roll (csi_t *ctx)
{
    csi_status_t status;
    long j, n;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &j);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 1, &n);
    if (_csi_unlikely (status))
	return status;

    pop (2);
    check (n);
    return _csi_stack_roll (ctx, &ctx->ostack, j, n);
}

static csi_status_t
_rotate (csi_t *ctx)
{
    csi_object_t *obj;
    csi_status_t status;
    double theta;
    int type;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &theta);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_rotate (obj->datum.cr, theta);
	break;

    case CSI_OBJECT_TYPE_PATTERN:
	{
	    cairo_matrix_t ctm;
	    cairo_pattern_get_matrix (obj->datum.pattern, &ctm);
	    cairo_matrix_rotate (&ctm, theta);
	    cairo_pattern_set_matrix (obj->datum.pattern, &ctm);
	}
	break;


    case CSI_OBJECT_TYPE_MATRIX:
	cairo_matrix_rotate (&obj->datum.matrix->matrix, theta);
	break;

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_save (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_save);
}

static csi_status_t
_scale (csi_t *ctx)
{
    csi_object_t *obj;
    csi_status_t status;
    double x, y;
    int type;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 2);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_scale (obj->datum.cr, x, y);
	break;

    case CSI_OBJECT_TYPE_PATTERN:
	{
	    cairo_matrix_t ctm;
	    cairo_pattern_get_matrix (obj->datum.pattern, &ctm);
	    cairo_matrix_scale (&ctm, x, y);
	    cairo_pattern_set_matrix (obj->datum.pattern, &ctm);
	}
	break;


    case CSI_OBJECT_TYPE_MATRIX:
	cairo_matrix_scale (&obj->datum.matrix->matrix, x, y);
	break;

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_font_options_load_from_dictionary (csi_t *ctx,
				    csi_dictionary_t *dict,
				    cairo_font_options_t *options)
{
    const struct {
	const char *key;
	void (*setter) (cairo_font_options_t *, int val);
    } properties[] = {
	{ "antialias",
	    (void (*)(cairo_font_options_t *, int val))
		cairo_font_options_set_antialias },
	{ "subpixel-order",
	    (void (*)(cairo_font_options_t *, int val))
		cairo_font_options_set_subpixel_order },
	{ "hint-style",
	    (void (*)(cairo_font_options_t *, int val))
		cairo_font_options_set_hint_style },
	{ "hint-metrics",
	    (void (*)(cairo_font_options_t *, int val))
		cairo_font_options_set_hint_metrics },
	{ NULL, NULL },
    }, *prop = properties;

    while (prop->key != NULL) {
	csi_object_t key, value;
	csi_status_t status;

	status = csi_name_new_static (ctx, &key, prop->key);
	if (_csi_unlikely (status))
	    return status;

	if (csi_dictionary_has (dict, key.datum.name)) {
	    status = csi_dictionary_get (ctx, dict, key.datum.name, &value);
	    if (_csi_unlikely (status))
		return status;

	    if (_csi_unlikely (csi_object_get_type (&value) !=
				 CSI_OBJECT_TYPE_INTEGER))
	    {
		csi_object_free (ctx, &value);
		return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	    }

	    prop->setter (options, value.datum.integer);
	}

	prop++;
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_scaled_font (csi_t *ctx)
{
    csi_object_t obj;
    csi_dictionary_t *dict;
    cairo_font_face_t *font_face = NULL; /* silence the compiler */
    cairo_matrix_t font_matrix, ctm;
    cairo_font_options_t *options;
    csi_status_t status;

    check (4);

    status = _csi_ostack_get_dictionary (ctx, 0, &dict);
    if (_csi_unlikely (status))
	return status;
    options = cairo_font_options_create ();
    status = _font_options_load_from_dictionary (ctx, dict, options);
    if (_csi_unlikely (status)) {
	cairo_font_options_destroy (options);
	return status;
    }

    status = _csi_ostack_get_matrix (ctx, 1, &ctm);
    if (_csi_unlikely (status)) {
	cairo_font_options_destroy (options);
	return status;
    }

    status = _csi_ostack_get_matrix (ctx, 2, &font_matrix);
    if (_csi_unlikely (status)) {
	cairo_font_options_destroy (options);
	return status;
    }

    status = _csi_ostack_get_font_face (ctx, 3, &font_face);
    if (_csi_unlikely (status)) {
	cairo_font_options_destroy (options);
	return status;
    }

    obj.type = CSI_OBJECT_TYPE_SCALED_FONT;
    obj.datum.scaled_font = cairo_scaled_font_create (font_face,
						      &font_matrix,
						      &ctm,
						      options);
    cairo_font_options_destroy (options);
    pop (4);
    return push (&obj);
}

static csi_status_t
_select_font_face (csi_t *ctx)
{
    cairo_t *cr;
    long weight;
    long slant;
    csi_string_t *family;
    csi_status_t status;

    check (4);

    status = _csi_ostack_get_integer (ctx, 0,  &weight);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 1, &slant);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_string (ctx, 2, &family);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 3, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_select_font_face (cr, family->string, slant, weight);
    pop (3);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_context_set (csi_t *ctx,
	      cairo_t *cr,
	      csi_name_t key,
	      csi_object_t *obj)
{
    if (strcmp ((char *) key, "source") == 0) {
	if (_csi_unlikely (csi_object_get_type (obj) !=
			     CSI_OBJECT_TYPE_PATTERN))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	cairo_set_source (cr, obj->datum.pattern);
	return CSI_STATUS_SUCCESS;
    }

    if (strcmp ((char *) key, "scaled-font") == 0) {
	if (_csi_unlikely (csi_object_get_type (obj) !=
			     CSI_OBJECT_TYPE_SCALED_FONT))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	cairo_set_scaled_font (cr, obj->datum.scaled_font);
	return CSI_STATUS_SUCCESS;
    }

    if (strcmp ((char *) key, "font-face") == 0) {
	if (_csi_unlikely (csi_object_get_type (obj) !=
			     CSI_OBJECT_TYPE_FONT))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	cairo_set_font_face (cr, obj->datum.font_face);
	return CSI_STATUS_SUCCESS;
    }

    /* return _proxy_set()? */
    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
}

static csi_status_t
_set (csi_t *ctx)
{
    csi_object_t *key, *value, *dst;
    csi_status_t status;
    int type;

    check (3);

    value = _csi_peek_ostack (ctx, 0);
    key = _csi_peek_ostack (ctx, 1);
    dst = _csi_peek_ostack (ctx, 2);

    type = csi_object_get_type (dst);
    switch (type) {
    case CSI_OBJECT_TYPE_DICTIONARY:
	if (_csi_unlikely (csi_object_get_type (key) !=
			     CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	status = csi_dictionary_put (ctx,
				     dst->datum.dictionary,
				     key->datum.name,
				     value);
	break;
    case CSI_OBJECT_TYPE_ARRAY:
	if (_csi_unlikely (csi_object_get_type (key) !=
			     CSI_OBJECT_TYPE_INTEGER))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	status = csi_array_put (ctx,
				dst->datum.array,
				key->datum.integer,
				value);
	break;

    case CSI_OBJECT_TYPE_CONTEXT:
	if (_csi_unlikely (csi_object_get_type (key) !=
			     CSI_OBJECT_TYPE_NAME))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	status = _context_set (ctx,
			       dst->datum.cr,
			       key->datum.name,
			       value);
	break;

    case CSI_OBJECT_TYPE_STRING:
#if 0
	status = csi_string_put (dst, key, value);
	break;
#endif
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);
    return status;
}

static csi_status_t
_set_antialias (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    long antialias;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &antialias);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_antialias (cr, antialias);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_dash (csi_t *ctx)
{
    csi_array_t *array;
    csi_status_t status;
    cairo_t *cr;
    double offset;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &offset);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_array (ctx, 1, &array);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 2, &cr);
    if (_csi_unlikely (status))
	return status;

    if (array->stack.len == 0) {
	cairo_set_dash (cr, NULL, 0., 0.);
    } else {
	double stack_dashes[8];
	double *dashes;
	csi_integer_t n;

	if (_csi_likely (array->stack.len < ARRAY_LENGTH (stack_dashes))) {
	    dashes = stack_dashes;
	} else {
	    if (_csi_unlikely ((unsigned) array->stack.len >= INT_MAX / sizeof (double)))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	    dashes = _csi_alloc (ctx, sizeof (double) * array->stack.len);
	    if (_csi_unlikely (dashes == NULL))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	}

	for (n = 0; n < array->stack.len; n++) {
	    if (_csi_unlikely (! csi_object_is_number
				 (&array->stack.objects[n])))
	    {
		if (dashes != stack_dashes)
		    _csi_free (ctx, dashes);
		return _csi_error (CSI_STATUS_INVALID_SCRIPT);
	    }

	    dashes[n] = csi_number_get_value (&array->stack.objects[n]);
	}

	cairo_set_dash (cr, dashes, n, offset);

	if (dashes != stack_dashes)
	    _csi_free (ctx, dashes);
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_device_offset (csi_t *ctx)
{
    csi_status_t status;
    cairo_surface_t *surface;
    double x, y;

    check (3);

    status = _csi_ostack_get_number (ctx, 0,  &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 2, &surface);
    if (_csi_unlikely (status))
	return status;

    cairo_surface_set_device_offset (surface, x, y);
    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_device_scale (csi_t *ctx)
{
    csi_status_t status;
    cairo_surface_t *surface;
    double x, y;

    check (3);

    status = _csi_ostack_get_number (ctx, 0,  &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 2, &surface);
    if (_csi_unlikely (status))
	return status;

    cairo_surface_set_device_scale (surface, x, y);
    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_extend (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    long extend;
    int type;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &extend);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_pattern_set_extend (cairo_get_source (obj->datum.cr),
				  extend);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_pattern_set_extend (obj->datum.pattern, extend);
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_fallback_resolution (csi_t *ctx)
{
    csi_status_t status;
    cairo_surface_t *surface;
    double dpi_x, dpi_y;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &dpi_y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &dpi_x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 2, &surface);
    if (_csi_unlikely (status))
	return status;

    cairo_surface_set_fallback_resolution (surface, dpi_x, dpi_y);
    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_fill_rule (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    long fill_rule;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &fill_rule);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_fill_rule (cr, fill_rule);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_filter (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    long filter;
    int type;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &filter);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_pattern_set_filter (cairo_get_source (obj->datum.cr),
				  filter);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_pattern_set_filter (obj->datum.pattern, filter);
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_font_face (csi_t *ctx)
{
    cairo_t *cr;
    cairo_font_face_t *font = NULL; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_font_face (ctx, 0, &font);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_font_face (cr, font);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_font_options (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    csi_dictionary_t *dict;
    cairo_font_options_t *options;

    check (2);

    status = _csi_ostack_get_dictionary (ctx, 0, &dict);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    options = cairo_font_options_create ();
    status = _font_options_load_from_dictionary (ctx, dict, options);
    if (_csi_unlikely (status))
	return status;

    cairo_set_font_options (cr, options);
    cairo_font_options_destroy (options);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_font_matrix (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    cairo_matrix_t m;

    check (2);

    status = _csi_ostack_get_matrix (ctx, 0, &m);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_font_matrix (cr, &m);
    pop(1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_font_size (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    double size;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &size);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_font_size (cr, size);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_line_cap (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    long line_cap;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &line_cap);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_line_cap (cr, line_cap);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_line_join (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    long line_join;

    status = _csi_ostack_get_integer (ctx, 0, &line_join);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_line_join (cr, line_join);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_line_width (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    double line_width;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &line_width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_line_width (cr, line_width);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_hairline (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    cairo_bool_t set_hairline = FALSE; /* silence the compiler */

    check (2);

    status = _csi_ostack_get_boolean (ctx, 0, &set_hairline);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
	if (_csi_unlikely (status))
	return status;

    cairo_set_hairline (cr, set_hairline);
	pop (1);
	return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_matrix (csi_t *ctx)
{
    csi_object_t *obj;
    csi_status_t status;
    cairo_matrix_t m;
    int type;

    check (2);

    status = _csi_ostack_get_matrix (ctx, 0, &m);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_set_matrix (obj->datum.cr, &m);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	cairo_pattern_set_matrix (obj->datum.pattern, &m);
	break;
    case CSI_OBJECT_TYPE_MATRIX:
	obj->datum.matrix->matrix = m;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

struct _mime_tag {
    csi_t *ctx;
    csi_string_t *source;
};
static void
_mime_tag_destroy (void *closure)
{
    struct _mime_tag *tag = closure;

    if (--tag->source->base.ref)
	csi_string_free (tag->ctx, tag->source);

    _csi_slab_free (tag->ctx, tag, sizeof (struct _mime_tag));
}

static csi_status_t
_set_mime_data (csi_t *ctx)
{
    csi_status_t status;
    csi_object_t *obj;
    const char *mime = NULL; /* silence the compiler */
    csi_object_t source;
    cairo_surface_t *surface;
    struct _mime_tag *tag;
    int type;

    check (3);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_FILE:
	status = _csi_file_as_string (ctx, obj->datum.file, &source);
	if (_csi_unlikely (status))
	    return status;

	break;

    case CSI_OBJECT_TYPE_STRING:
	source = *csi_object_reference (obj);
	break;

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    status = _csi_ostack_get_string_constant (ctx, 1, &mime);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_surface (ctx, 2, &surface);
    if (_csi_unlikely (status))
	return status;


    /* XXX free source */
    tag = _csi_slab_alloc (ctx, sizeof (struct _mime_tag));
    if (_csi_unlikely (tag == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);
    tag->ctx = cairo_script_interpreter_reference (ctx);
    tag->source = source.datum.string;
    tag->source->base.ref++;

    status = cairo_surface_set_mime_data (surface,
					  mime,
					  (uint8_t *)
					  source.datum.string->string,
					  source.datum.string->len,
					  _mime_tag_destroy, tag);
    if (_csi_unlikely (status)) {
	_mime_tag_destroy (tag);
	return status;
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_miter_limit (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    double miter_limit;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &miter_limit);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_miter_limit (cr, miter_limit);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_operator (csi_t *ctx)
{
    cairo_t *cr;
    long val;
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_integer (ctx, 0, &val);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_operator (cr, val);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_scaled_font (csi_t *ctx)
{
    cairo_t *cr;
    cairo_scaled_font_t *font = NULL; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_scaled_font (ctx, 0, &font);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_scaled_font (cr, font);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_source (csi_t *ctx)
{
    cairo_t *cr;
    cairo_pattern_t *pattern = NULL; /* silence the compiler */
    csi_status_t status;

    check (2);

    status = _csi_ostack_get_pattern (ctx, 0, &pattern);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_source (cr, pattern);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_boolean_t
_matching_images (cairo_surface_t *a, cairo_surface_t *b)
{
    cairo_format_t format_a, format_b;

    if (cairo_surface_get_type (a) != CAIRO_SURFACE_TYPE_IMAGE)
	return FALSE;
    if (cairo_surface_get_type (b) != CAIRO_SURFACE_TYPE_IMAGE)
	return FALSE;

    if (cairo_image_surface_get_height (a) != cairo_image_surface_get_height (b))
	return FALSE;

    if (cairo_image_surface_get_width (a) != cairo_image_surface_get_width (b))
	return FALSE;

    format_a = cairo_image_surface_get_format (a);
    if (format_a == CAIRO_FORMAT_RGB24)
	format_a = CAIRO_FORMAT_ARGB32;

    format_b = cairo_image_surface_get_format (b);
    if (format_b == CAIRO_FORMAT_RGB24)
	format_b = CAIRO_FORMAT_ARGB32;

    if (format_a != format_b)
	return FALSE;

    return TRUE;
}

static csi_status_t
_set_source_image (csi_t *ctx)
{
    csi_status_t status;
    cairo_surface_t *surface;
    cairo_surface_t *source;

    check (2);

    status = _csi_ostack_get_surface (ctx, 0, &source);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 1, &surface);
    if (_csi_unlikely (status))
	return status;

    /* Catch the most frequent use of simply uploading pixel data,
     * principally to remove the pixman ops from the profiles.
     */
    if (_csi_likely (_matching_images (surface, source))) {
	if (cairo_surface_get_reference_count (surface) == 1 &&
	    cairo_surface_get_reference_count (source) == 1)
	{
	    _csi_peek_ostack (ctx, 0)->datum.surface = surface;
	    _csi_peek_ostack (ctx, 1)->datum.surface = source;
	}
	else
	{
	    cairo_surface_flush (surface);
	    memcpy (cairo_image_surface_get_data (surface),
		    cairo_image_surface_get_data (source),
		    cairo_image_surface_get_height (source) * cairo_image_surface_get_stride (source));
	    cairo_surface_mark_dirty (surface);
	}
    } else {
	cairo_t *cr;

	cr = cairo_create (surface);
	cairo_set_source_surface (cr, source, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_source_rgb (csi_t *ctx)
{
    csi_status_t status;
    double r,g,b;
    cairo_t *cr;

    check (4);

    status = _csi_ostack_get_number (ctx, 0, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 3, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_source_rgb (cr, r, g, b);
    pop (3);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_source_rgba (csi_t *ctx)
{
    csi_status_t status;
    double r,g,b,a;
    cairo_t *cr;

    check (5);

    status = _csi_ostack_get_number (ctx, 0, &a);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &b);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &g);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &r);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 4, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_source_rgba (cr, r, g, b, a);
    pop (4);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_set_tolerance (csi_t *ctx)
{
    csi_status_t status;
    cairo_t *cr;
    double tolerance;

    check (2);

    status = _csi_ostack_get_number (ctx, 0, &tolerance);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_set_tolerance (cr, tolerance);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_transform (csi_t *ctx)
{
    csi_object_t *obj;
    csi_status_t status;
    cairo_matrix_t m;
    int type;

    check (2);

    status = _csi_ostack_get_matrix (ctx, 0, &m);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_transform (obj->datum.cr, &m);
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	{
	    cairo_matrix_t ctm;
	    cairo_pattern_get_matrix (obj->datum.pattern, &ctm);
	    cairo_matrix_multiply (&ctm, &m, &ctm);
	    cairo_pattern_set_matrix (obj->datum.pattern, &ctm);
	}
	break;
    case CSI_OBJECT_TYPE_MATRIX:
	    cairo_matrix_multiply (&obj->datum.matrix->matrix,
				   &m,
				   &obj->datum.matrix->matrix);
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_translate (csi_t *ctx)
{
    csi_object_t *obj;
    csi_status_t status;
    double x, y;
    int type;

    check (3);

    status = _csi_ostack_get_number (ctx, 0, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &x);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 2);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_translate (obj->datum.cr, x, y);
	break;

    case CSI_OBJECT_TYPE_PATTERN:
	{
	    cairo_matrix_t ctm;
	    cairo_pattern_get_matrix (obj->datum.pattern, &ctm);
	    cairo_matrix_translate (&ctm, x, y);
	    cairo_pattern_set_matrix (obj->datum.pattern, &ctm);
	}
	break;


    case CSI_OBJECT_TYPE_MATRIX:
	cairo_matrix_translate (&obj->datum.matrix->matrix, x, y);
	break;

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_true (csi_t *ctx)
{
    return _csi_push_ostack_boolean (ctx, TRUE);
}

static csi_status_t
_show_page (csi_t *ctx)
{
    csi_object_t *obj;
    int type;

    check (1);

    obj = _csi_peek_ostack (ctx, 0);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_CONTEXT:
	cairo_show_page (obj->datum.cr);
	if (ctx->hooks.copy_page != NULL)
	    ctx->hooks.copy_page (ctx->hooks.closure, obj->datum.cr);
	break;
    case CSI_OBJECT_TYPE_SURFACE:
	cairo_surface_show_page (obj->datum.surface);
	/* XXX hook? */
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_similar (csi_t *ctx)
{
    csi_object_t obj;
    long content;
    double width, height;
    cairo_surface_t *other;
    csi_status_t status;

    check (4);

    status = _csi_ostack_get_integer (ctx, 0, &content);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &height);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 3, &other);
    if (_csi_unlikely (status))
	return status;

    /* silently fix-up a common bug when writing CS */
    if ((content & CAIRO_CONTENT_COLOR_ALPHA) == 0) {
	if (_csi_unlikely (content & ~CAIRO_CONTENT_COLOR_ALPHA))
	    return _csi_error (CSI_STATUS_INVALID_SCRIPT);

	switch ((int) content) {
	default:
	case CAIRO_FORMAT_ARGB32:
	    content = CAIRO_CONTENT_COLOR_ALPHA;
	    break;
	case CAIRO_FORMAT_RGB16_565:
	case CAIRO_FORMAT_RGB24:
	    content = CAIRO_CONTENT_COLOR;
	    break;
	case CAIRO_FORMAT_A8:
	case CAIRO_FORMAT_A1:
	    content = CAIRO_CONTENT_ALPHA;
	    break;
	}
    }

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = cairo_surface_create_similar (other,
						      content, width, height);
    pop (4);
    return push (&obj);
}

static csi_status_t
_similar_image (csi_t *ctx)
{
    csi_object_t obj;
    long format;
    double width, height;
    cairo_surface_t *other;
    csi_status_t status;

    check (4);

    status = _csi_ostack_get_number (ctx, 0, &height);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_integer (ctx, 2, &format);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 3, &other);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = cairo_surface_create_similar_image (other,
							    format,
							    width, height);
    pop (4);
    return push (&obj);
}

static csi_status_t
_subsurface (csi_t *ctx)
{
    csi_object_t obj;
    double x, y, width, height;
    cairo_surface_t *target;
    csi_status_t status;

    check (5);

    status = _csi_ostack_get_number (ctx, 0, &height);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 1, &width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 2, &y);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_number (ctx, 3, &x);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 4, &target);
    if (_csi_unlikely (status))
	return status;

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = cairo_surface_create_for_rectangle (target, x, y, width, height);
    pop (5);
    return push (&obj);
}

static csi_status_t
_show_text (csi_t *ctx)
{
    csi_status_t status;
    csi_string_t *text;
    cairo_t *cr;

    check (2);

    status = _csi_ostack_get_string (ctx, 0, &text);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_show_text (cr, text->string);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_show_glyphs (csi_t *ctx)
{
    csi_array_t *array;
    csi_status_t status;
    cairo_t *cr;
    cairo_glyph_t stack_glyphs[256], *glyphs;
    csi_integer_t nglyphs, i;

    check (2);

    status = _csi_ostack_get_array (ctx, 0, &array);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    /* count glyphs */
    nglyphs = 0;
    for (i = 0; i < array->stack.len; i++) {
	csi_object_t *obj = &array->stack.objects[i];
	int type = csi_object_get_type (obj);
	switch (type) {
	case CSI_OBJECT_TYPE_ARRAY:
	    nglyphs += obj->datum.array->stack.len;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    nglyphs += obj->datum.string->len;
	    break;
	}
    }
    if (nglyphs == 0) {
	pop (1);
	return CSI_STATUS_SUCCESS;
    }

    if (nglyphs > ARRAY_LENGTH (stack_glyphs)) {
	if (_csi_unlikely ((unsigned) nglyphs >= INT_MAX / sizeof (cairo_glyph_t)))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	glyphs = _csi_alloc (ctx, sizeof (cairo_glyph_t) * nglyphs);
	if (_csi_unlikely (glyphs == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    } else
	glyphs = stack_glyphs;

    nglyphs = _glyph_string (ctx, array, cairo_get_scaled_font (cr), glyphs);
    cairo_show_glyphs (cr, glyphs, nglyphs);

    if (glyphs != stack_glyphs)
	_csi_free (ctx, glyphs);

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_show_text_glyphs (csi_t *ctx)
{
    csi_object_t *obj;
    csi_array_t *array;
    csi_string_t *string;
    csi_string_t *utf8_string;
    csi_status_t status;
    cairo_t *cr;
    cairo_text_cluster_t stack_clusters[256], *clusters;
    cairo_glyph_t stack_glyphs[256], *glyphs;
    csi_integer_t nglyphs, nclusters, i;
    long direction;
    int type;

    check (5);

    status = _csi_ostack_get_integer (ctx, 0, &direction);
    if (_csi_unlikely (status))
	return status;

    obj = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_ARRAY:
	array = obj->datum.array;
	nclusters = array->stack.len / 2;
	if (nclusters > ARRAY_LENGTH (stack_clusters)) {
	    if (_csi_unlikely ((unsigned) nclusters >= INT_MAX / sizeof (cairo_text_cluster_t)))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	    clusters = _csi_alloc (ctx, sizeof (cairo_text_cluster_t) * nclusters);
	    if (_csi_unlikely (clusters == NULL))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	} else
	    clusters = stack_clusters;

	for (i = 0; i < nclusters; i++) {
	    clusters[i].num_bytes = csi_number_get_value (&array->stack.objects[2*i+0]);
	    clusters[i].num_glyphs = csi_number_get_value (&array->stack.objects[2*i+1]);
	}
	break;

    case CSI_OBJECT_TYPE_STRING:
	string = obj->datum.string;
	nclusters = string->len / 2;
	if (nclusters > ARRAY_LENGTH (stack_clusters)) {
	    if (_csi_unlikely ((unsigned) nclusters >= INT_MAX / sizeof (cairo_text_cluster_t)))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	    clusters = _csi_alloc (ctx, sizeof (cairo_text_cluster_t) * nclusters);
	    if (_csi_unlikely (clusters == NULL))
		return _csi_error (CSI_STATUS_NO_MEMORY);
	} else
	    clusters = stack_clusters;

	for (i = 0; i < nclusters; i++) {
	    clusters[i].num_bytes = string->string[2*i+0];
	    clusters[i].num_glyphs = string->string[2*i+1];
	}
	break;

    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    status = _csi_ostack_get_array (ctx, 2, &array);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_string (ctx, 3, &utf8_string);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 4, &cr);
    if (_csi_unlikely (status))
	return status;

    /* count glyphs */
    nglyphs = 0;
    for (i = 0; i < array->stack.len; i++) {
	obj = &array->stack.objects[i];
	type = csi_object_get_type (obj);
	switch (type) {
	case CSI_OBJECT_TYPE_ARRAY:
	    nglyphs += obj->datum.array->stack.len;
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    nglyphs += obj->datum.string->len;
	    break;
	}
    }
    if (nglyphs == 0) {
	pop (4);
	return CSI_STATUS_SUCCESS;
    }

    if (nglyphs > ARRAY_LENGTH (stack_glyphs)) {
	if (_csi_unlikely ((unsigned) nglyphs >= INT_MAX / sizeof (cairo_glyph_t)))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	glyphs = _csi_alloc (ctx, sizeof (cairo_glyph_t) * nglyphs);
	if (_csi_unlikely (glyphs == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    } else
	glyphs = stack_glyphs;

    nglyphs = _glyph_string (ctx, array, cairo_get_scaled_font (cr), glyphs);
    cairo_show_text_glyphs (cr,
			    utf8_string->string, utf8_string->len,
			    glyphs, nglyphs,
			    clusters, nclusters,
			    direction);

    if (clusters != stack_clusters)
	_csi_free (ctx, clusters);
    if (glyphs != stack_glyphs)
	_csi_free (ctx, glyphs);

    pop (4);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_stroke (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_stroke);
}

static csi_status_t
_stroke_preserve (csi_t *ctx)
{
    return _do_cairo_op (ctx, cairo_stroke_preserve);
}

static csi_status_t
_sub (csi_t *ctx)
{
    csi_object_t *A;
    csi_object_t *B;
    csi_object_type_t type_a, type_b;

    check (2);

    B = _csi_peek_ostack (ctx, 0);
    A = _csi_peek_ostack (ctx, 1);

    type_a = csi_object_get_type (A);
    if (_csi_unlikely (! (type_a == CSI_OBJECT_TYPE_INTEGER ||
			    type_a == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    type_b = csi_object_get_type (B);
    if (_csi_unlikely (! (type_b == CSI_OBJECT_TYPE_INTEGER ||
			    type_b == CSI_OBJECT_TYPE_REAL)))
    {
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (2);

    if (type_a == CSI_OBJECT_TYPE_REAL &&
	type_b == CSI_OBJECT_TYPE_REAL)
    {
	return _csi_push_ostack_real (ctx, A->datum.real - B->datum.real);

    }
    else if (type_a == CSI_OBJECT_TYPE_INTEGER &&
	     type_b == CSI_OBJECT_TYPE_INTEGER)
    {
	return _csi_push_ostack_integer (ctx,
					 A->datum.integer - B->datum.integer);
    }
    else
    {
	double v;

	if (type_a == CSI_OBJECT_TYPE_REAL)
	    v = A->datum.real;
	else
	    v = A->datum.integer;

	if (type_b == CSI_OBJECT_TYPE_REAL)
	    v -= B->datum.real;
	else
	    v -= B->datum.integer;

	return _csi_push_ostack_real (ctx, v);
    }
}

static csi_status_t
_surface (csi_t *ctx)
{
    csi_object_t obj;
    csi_dictionary_t *dict;
    csi_proxy_t *proxy;
    csi_object_t key;
    double width, height;
    csi_surface_create_func_t hook;
    long content;
    cairo_surface_t *surface;
    long uid;
    csi_status_t status;

    check (1);

    status = _csi_ostack_get_dictionary (ctx, 0, &dict);
    if (_csi_unlikely (status))
	return status;

    status = _csi_dictionary_get_number (ctx, dict, "width", FALSE, &width);
    if (_csi_unlikely (status))
	return status;
    status = _csi_dictionary_get_number (ctx, dict, "height", FALSE, &height);
    if (_csi_unlikely (status))
	return status;

    content = CAIRO_CONTENT_COLOR_ALPHA;
    status = _csi_dictionary_get_integer (ctx, dict, "content", TRUE, &content);
    if (_csi_unlikely (status))
	return status;

    uid = 0;
    status = _csi_dictionary_get_integer (ctx, dict, "uid", TRUE, &uid);
    if (_csi_unlikely (status))
	return status;
    if (uid == 0) {
	status = _csi_dictionary_get_integer (ctx, dict, "drawable", TRUE, &uid);
	if (_csi_unlikely (status))
	    return status;
    }

    hook = ctx->hooks.surface_create;
    assert (hook != NULL);

    surface = hook (ctx->hooks.closure, content, width, height, uid);
    if (_csi_unlikely (surface == NULL)) {
	return _csi_error (CSI_STATUS_NULL_POINTER);
    }

    proxy = _csi_proxy_create (ctx, surface, dict,
			       ctx->hooks.surface_destroy,
			       ctx->hooks.closure);
    if (_csi_unlikely (proxy == NULL)) {
	cairo_surface_destroy (surface);
	return _csi_error (CSI_STATUS_NO_MEMORY);
    }

    status = cairo_surface_set_user_data (surface,
					  &_csi_proxy_key,
					  proxy, _csi_proxy_destroy);
    if (_csi_unlikely (status)) {
	_csi_proxy_destroy (proxy);
	cairo_surface_destroy (surface);
	return status;
    }

    status = csi_name_new_static (ctx, &key, "fallback-resolution");
    if (_csi_unlikely (status)) {
	cairo_surface_destroy (surface);
	return status;
    }
    if (csi_dictionary_has (dict, key.datum.name)) {
	status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
	if (_csi_unlikely (status)) {
	    cairo_surface_destroy (surface);
	    return status;
	}
	if (csi_object_get_type (&obj) == CSI_OBJECT_TYPE_ARRAY) {
	    csi_array_t *array = obj.datum.array;
	    if (array->stack.len == 2) {
		cairo_surface_set_fallback_resolution (surface,
						       csi_number_get_value
						       (&array->stack.objects[0]),
						       csi_number_get_value
						       (&array->stack.objects[1]));
	    }
	}
    }
    /* initialise surface to source */
    status = csi_name_new_static (ctx, &key, "source");
    if (_csi_unlikely (status)) {
	cairo_surface_destroy (surface);
	return status;
    }
    if (csi_dictionary_has (dict, key.datum.name)) {
	cairo_surface_t *image;
	cairo_t *cr;

	status = _image_load_from_dictionary (ctx, dict, &image);
	if (_csi_unlikely (status)) {
	    cairo_surface_destroy (surface);
	    return status;
	}

	cr = cairo_create (surface);
	cairo_set_source_surface (cr, image, 0, 0);
	cairo_surface_destroy (image);
	cairo_paint (cr);
	status = cairo_status (cr);
	cairo_destroy (cr);

	if (_csi_unlikely (status))
	    return status;
    }

    status = csi_name_new_static (ctx, &key, "device-offset");
    if (_csi_unlikely (status)) {
	cairo_surface_destroy (surface);
	return status;
    }
    if (csi_dictionary_has (dict, key.datum.name)) {
	status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;

	if (csi_object_get_type (&obj) == CSI_OBJECT_TYPE_ARRAY) {
	    csi_array_t *array = obj.datum.array;

	    if (array->stack.len == 2) {
		cairo_surface_set_device_offset (surface,
						 csi_number_get_value
						 (&array->stack.objects[0]),
						 csi_number_get_value
						 (&array->stack.objects[1]));
	    }
	}
    }

    status = csi_name_new_static (ctx, &key, "device-scale");
    if (_csi_unlikely (status)) {
	cairo_surface_destroy (surface);
	return status;
    }
    if (csi_dictionary_has (dict, key.datum.name)) {
	status = csi_dictionary_get (ctx, dict, key.datum.name, &obj);
	if (_csi_unlikely (status))
	    return status;

	if (csi_object_get_type (&obj) == CSI_OBJECT_TYPE_ARRAY) {
	    csi_array_t *array = obj.datum.array;

	    if (array->stack.len == 2) {
		cairo_surface_set_device_scale (surface,
						csi_number_get_value
						(&array->stack.objects[0]),
						csi_number_get_value
						(&array->stack.objects[1]));
	    }
	}
    }

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = surface;
    pop (1);
    return push (&obj);
}

static csi_status_t
_record (csi_t *ctx)
{
    csi_object_t obj;
    long content;
    csi_array_t *array;
    csi_status_t status;
    cairo_rectangle_t extents;
    cairo_rectangle_t *r;

    check (2);

    status = _csi_ostack_get_array (ctx, 0, &array);
    if (_csi_unlikely (status))
	return status;

    status = _csi_ostack_get_integer (ctx, 1, &content);
    if (_csi_unlikely (status))
	return status;

    switch (array->stack.len) {
    case 0:
	r = NULL;
	break;
    case 2:
	extents.x = extents.y = 0;
	extents.width = _csi_object_as_real (&array->stack.objects[0]);
	extents.height = _csi_object_as_real (&array->stack.objects[1]);
	r = &extents;
	break;
    case 4:
	extents.x = _csi_object_as_real (&array->stack.objects[0]);
	extents.y = _csi_object_as_real (&array->stack.objects[1]);
	extents.width = _csi_object_as_real (&array->stack.objects[2]);
	extents.height = _csi_object_as_real (&array->stack.objects[3]);
	r = &extents;
	break;
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    obj.type = CSI_OBJECT_TYPE_SURFACE;
    obj.datum.surface = cairo_recording_surface_create (content, r);
    pop (2);
    return push (&obj);
}

static csi_status_t
_text_path (csi_t *ctx)
{
    csi_status_t status;
    csi_string_t *text;
    cairo_t *cr;

    check (2);

    status = _csi_ostack_get_string (ctx, 0, &text);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_context (ctx, 1, &cr);
    if (_csi_unlikely (status))
	return status;

    cairo_text_path (cr, text->string);
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_undef (csi_t *ctx)
{
    csi_name_t name = 0; /* silence the compiler */
    csi_status_t status;

    check (1);

    status = _csi_ostack_get_name (ctx, 0, &name);
    if (_csi_unlikely (status))
	return status;

    status = _csi_name_undefine (ctx, name);
    if (_csi_unlikely (status))
	return status;

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_unset (csi_t *ctx)
{
    csi_object_t *dst;
    csi_name_t name = 0; /* silence the compiler */
    csi_status_t status;
    int type;

    check (2);

    status = _csi_ostack_get_name (ctx, 0, &name);
    if (_csi_unlikely (status))
	return status;

    dst = _csi_peek_ostack (ctx, 1);
    type = csi_object_get_type (dst);
    switch (type) {
    case CSI_OBJECT_TYPE_DICTIONARY:
	csi_dictionary_remove (ctx, dst->datum.dictionary, name);
	break;
    case CSI_OBJECT_TYPE_STRING:
    case CSI_OBJECT_TYPE_ARRAY:
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_write_to_png (csi_t *ctx)
{
    csi_status_t status;
    csi_string_t *filename;
    cairo_surface_t *surface;

    check (2);

    status = _csi_ostack_get_string (ctx, 0, &filename);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 1, &surface);
    if (_csi_unlikely (status))
	return status;

#if CAIRO_HAS_PNG_FUNCTIONS
    status = cairo_surface_write_to_png (surface, filename->string);
    if (_csi_unlikely (status))
	return status;
#else
    return CAIRO_STATUS_WRITE_ERROR;
#endif

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_write_to_script (csi_t *ctx)
{
    csi_status_t status;
    csi_string_t *filename;
    cairo_surface_t *record;

    check (2);

    status = _csi_ostack_get_string (ctx, 0, &filename);
    if (_csi_unlikely (status))
	return status;
    status = _csi_ostack_get_surface (ctx, 1, &record);
    if (_csi_unlikely (status))
	return status;

    if (cairo_surface_get_type (record) != CAIRO_SURFACE_TYPE_RECORDING)
	return CAIRO_STATUS_SURFACE_TYPE_MISMATCH;

#if CAIRO_HAS_SCRIPT_SURFACE
    {
	cairo_device_t *script;

	script = cairo_script_create (filename->string);
	status = cairo_script_from_recording_surface (script, record);
	cairo_device_destroy (script);
	if (_csi_unlikely (status))
	    return status;
    }
#else
    return CAIRO_STATUS_WRITE_ERROR;
#endif

    pop (1);
    return CSI_STATUS_SUCCESS;
}

static csi_status_t
_xor (csi_t *ctx)
{
    csi_object_t *a, *b;
    int type;

    check (2);

    a = _csi_peek_ostack (ctx, 0);
    b = _csi_peek_ostack (ctx, 1);
    if (_csi_unlikely (csi_object_get_type (a) != csi_object_get_type (b)))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    pop (2);
    type = csi_object_get_type (a);
    switch (type) {
    case CSI_OBJECT_TYPE_INTEGER:
	return _csi_push_ostack_integer (ctx,
					 a->datum.integer ^ b->datum.integer);
    case CSI_OBJECT_TYPE_BOOLEAN:
	return _csi_push_ostack_boolean (ctx,
					 a->datum.boolean ^ b->datum.boolean);
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static csi_status_t
_debug_print (csi_t *ctx)
{
    csi_object_t *obj;

    check (1);
    obj = _csi_peek_ostack (ctx, 0);
    switch (csi_object_get_type (obj)) {
    case CSI_OBJECT_TYPE_NULL:
	fprintf (stderr, "NULL\n");
	break;

	/* atomics */
    case CSI_OBJECT_TYPE_BOOLEAN:
	fprintf (stderr, "boolean: %s\n",
		 obj->datum.boolean ? "true" : "false");
	break;
    case CSI_OBJECT_TYPE_INTEGER:
	fprintf (stderr, "integer: %ld\n", obj->datum.integer);
	break;
    case CSI_OBJECT_TYPE_MARK:
	fprintf (stderr, "mark\n");
	break;
    case CSI_OBJECT_TYPE_NAME:
	fprintf (stderr, "name: %s\n", (char *) obj->datum.name);
	break;
    case CSI_OBJECT_TYPE_OPERATOR:
	fprintf (stderr, "operator: %p\n", obj->datum.ptr);
	break;
    case CSI_OBJECT_TYPE_REAL:
	fprintf (stderr, "real: %g\n", obj->datum.real);
	break;

	/* compound */
    case CSI_OBJECT_TYPE_ARRAY:
	fprintf (stderr, "array\n");
	break;
    case CSI_OBJECT_TYPE_DICTIONARY:
	fprintf (stderr, "dictionary\n");
	break;
    case CSI_OBJECT_TYPE_FILE:
	fprintf (stderr, "file\n");
	break;
    case CSI_OBJECT_TYPE_MATRIX:
	fprintf (stderr, "matrix: [%g %g %g %g %g %g]\n",
		 obj->datum.matrix->matrix.xx,
		 obj->datum.matrix->matrix.yx,
		 obj->datum.matrix->matrix.xy,
		 obj->datum.matrix->matrix.yy,
		 obj->datum.matrix->matrix.x0,
		 obj->datum.matrix->matrix.y0);
	break;
    case CSI_OBJECT_TYPE_STRING:
	fprintf (stderr, "string: %s\n", obj->datum.string->string);
	break;

	/* cairo */
    case CSI_OBJECT_TYPE_CONTEXT:
	fprintf (stderr, "context\n");
	break;
    case CSI_OBJECT_TYPE_FONT:
	fprintf (stderr, "font\n");
	break;
    case CSI_OBJECT_TYPE_PATTERN:
	fprintf (stderr, "pattern\n");
	break;
    case CSI_OBJECT_TYPE_SCALED_FONT:
	fprintf (stderr, "scaled-font\n");
	break;
    case CSI_OBJECT_TYPE_SURFACE:
	fprintf (stderr, "surface\n");
	break;
    }
    pop (1);
    return CSI_STATUS_SUCCESS;
}

static const csi_operator_def_t
_defs[] = {
    { "<<", _mark },
    { ">>", end_dict_construction },
    { "[", _mark },
    { "]", end_array_construction },
    { "a", _alpha },
    { "abs", NULL },
    { "add", _add },
    { "add-color-stop", _add_color_stop },
    { "and", _and },
    { "arc", _arc },
    { "arc-negative", _arc_negative },
    { "arc-", _arc_negative },
    { "arc-to", NULL },
    { "array", _array },
    { "astore", NULL },
    { "atan", NULL },
    { "bind", _bind },
    { "bitshift", _bitshift },
    { "c", _curve_to },
    { "C", _rel_curve_to },
    { "ceiling", NULL },
    { "clear", NULL },
    { "clear-to-mark", NULL },
    { "clip", _clip },
    { "clip-extents", NULL },
    { "clip-preserve", _clip_preserve },
    { "clip+", _clip_preserve },
    { "close-path", _close_path },
    { "context", _context },
    { "copy", _copy },
    { "copy-page", _copy_page },
    { "cos", NULL },
    { "count", NULL },
    { "count-to-mark", NULL },
    { "curve-to", _curve_to },
    { "cvi", _cvi },
    { "cvr", _cvr },
    { "def", _def },
    { "device-to-user", NULL },
    { "device-to-user-distance", NULL },
    { "dict", _dict },
    { "div", _div },
    { "dup", _duplicate },
    { "eq", _eq },
    { "exch", _exch },
    { "exec", NULL },
    { "exp", NULL },
    { "false", _false },
    { "fill", _fill },
    { "fill-extents", NULL },
    { "fill-preserve", _fill_preserve },
    { "fill+", _fill_preserve },
    { "filter", _filter },
    { "floor", NULL },
    { "font", _font },
    { "for", _for },
    { "forall", NULL },
    { "g", _gray },
    { "ge", _ge },
    { "get", _get },
    { "glyph-path", _glyph_path },
    { "gt", _gt },
    { "h", _close_path },
    { "identity", _identity },
    { "if", _if },
    { "ifelse", _ifelse },
    { "image", _image },
    { "index", _index },
    { "integer", _integer },
    { "invert", _invert },
    { "in-stroke", NULL },
    { "in-fill", NULL },
    { "known", NULL },
    { "l", _line_to },
    { "L", _rel_line_to },
    { "languagelevel", NULL },
    { "le", _le },
    { "length", NULL },
    { "linear", _linear },
    { "line-to", _line_to },
    { "ln", NULL },
    { "load", NULL },
    { "log", NULL },
    { "loop", NULL },
    { "lt", _lt },
    { "m", _move_to },
    { "M", _rel_move_to },
    { "map-to-image", _map_to_image },
    { "mark", _mark },
    { "mask", _mask },
    { "matrix", _matrix },

    { "mesh", _mesh },
    { "begin-patch", _mesh_begin_patch },
    { "end-patch", _mesh_end_patch },
    { "set-control-point", _mesh_set_control_point },
    { "set-corner-color", _mesh_set_corner_color },

    { "mod", _mod },
    { "move-to", _move_to },
    { "mul", _mul },
    { "multiply", NULL },
    { "n", _new_path },
    { "N", _new_sub_path },
    { "ne", _ne },
    { "neg", _neg },
    { "new-path", _new_path },
    { "new-sub-path", _new_sub_path },
    { "not", _not },
    { "null", _null },
    { "or", _or },
    { "paint", _paint },
    { "paint-with-alpha", _paint_with_alpha },
    { "pattern", _pattern },
    { "pop", _pop },
    { "pop-group", _pop_group },
    { "push-group", _push_group },
    { "radial", _radial },
    { "rand", NULL },
    { "record", _record },
    { "rectangle", _rectangle },
    { "repeat", _repeat },
    { "restore", _restore },
    { "rel-curve-to", _rel_curve_to },
    { "rel-line-to", _rel_line_to },
    { "rel-move-to", _rel_move_to },
    { "reset-clip", _reset_clip },
    { "rgb", _rgb },
    { "rgba", _rgba },
    { "roll", _roll },
    { "rotate", _rotate },
    { "round", NULL },
    { "run", NULL },
    { "save", _save },
    { "scale", _scale },
    { "scaled-font", _scaled_font },
    { "select-font-face", _select_font_face },
    { "set", _set },
    { "set-antialias", _set_antialias },
    { "set-dash", _set_dash },
    { "set-device-offset", _set_device_offset },
    { "set-device-scale", _set_device_scale },
    { "set-extend", _set_extend },
    { "set-fallback-resolution", _set_fallback_resolution },
    { "set-fill-rule", _set_fill_rule },
    { "set-filter", _set_filter },
    { "set-font-face", _set_font_face },
    { "set-font-options", _set_font_options },
    { "set-font-matrix", _set_font_matrix },
    { "set-font-size", _set_font_size },
    { "set-line-cap", _set_line_cap },
    { "set-line-join", _set_line_join },
    { "set-line-width", _set_line_width },
    { "set-hairline", _set_hairline },
    { "set-matrix", _set_matrix },
    { "set-miter-limit", _set_miter_limit },
    { "set-mime-data", _set_mime_data },
    { "set-operator", _set_operator },
    { "set-scaled-font", _set_scaled_font },
    { "set-source", _set_source },
    { "set-source-image", _set_source_image },
    { "set-source-rgb", _set_source_rgb },
    { "set-source-rgba", _set_source_rgba },
    { "set-tolerance", _set_tolerance },
    { "show-glyphs", _show_glyphs },
    { "show-text", _show_text },
    { "show-text-glyphs", _show_text_glyphs },
    { "show-page", _show_page },
    { "similar", _similar },
    { "similar-image", _similar_image },
    { "sin", NULL },
    { "sqrt", NULL },
    { "sub", _sub },
    { "subsurface", _subsurface },
    { "surface", _surface },
    { "string", NULL },
    { "stroke", _stroke },
    { "stroke-extents", NULL },
    { "stroke-preserve", _stroke_preserve },
    { "stroke+", _stroke_preserve },
    { "text-path", _text_path },
    { "transform", _transform },
    { "transform-distance", NULL },
    { "transform-point", NULL },
    { "translate", _translate },
    { "true", _true },
    { "type", NULL },
    { "undef", _undef },
    { "unmap-image", _unmap_image },
    { "unset", _unset },
    { "user-to-device", NULL },
    { "user-to-device-distance", NULL },
    { "where", NULL },
    { "write-to-png", _write_to_png },
    { "write-to-script", _write_to_script },
    { "xor", _xor },

    { "=", _debug_print },

    { NULL, NULL },
};

const csi_operator_def_t *
_csi_operators (void)
{
    return _defs;
}

static const csi_integer_constant_def_t
_integer_constants[] = {
    { "CLEAR",		CAIRO_OPERATOR_CLEAR },
    { "SOURCE",		CAIRO_OPERATOR_SOURCE },
    { "OVER",		CAIRO_OPERATOR_OVER },
    { "IN",		CAIRO_OPERATOR_IN },
    { "OUT",		CAIRO_OPERATOR_OUT },
    { "ATOP",		CAIRO_OPERATOR_ATOP },
    { "DEST",		CAIRO_OPERATOR_DEST },
    { "DEST_OVER",	CAIRO_OPERATOR_DEST_OVER },
    { "DEST_IN",	CAIRO_OPERATOR_DEST_IN },
    { "DEST_OUT",	CAIRO_OPERATOR_DEST_OUT },
    { "DEST_ATOP",	CAIRO_OPERATOR_DEST_ATOP },
    { "XOR",		CAIRO_OPERATOR_XOR },
    { "ADD",		CAIRO_OPERATOR_ADD },
    { "SATURATE",	CAIRO_OPERATOR_SATURATE },
    { "MULTIPLY",	CAIRO_OPERATOR_MULTIPLY },
    { "SCREEN",		CAIRO_OPERATOR_SCREEN },
    { "OVERLAY",	CAIRO_OPERATOR_OVERLAY },
    { "DARKEN",		CAIRO_OPERATOR_DARKEN },
    { "LIGHTEN",	CAIRO_OPERATOR_LIGHTEN },
    { "DODGE",		CAIRO_OPERATOR_COLOR_DODGE },
    { "BURN",		CAIRO_OPERATOR_COLOR_BURN },
    { "HARD_LIGHT",	CAIRO_OPERATOR_HARD_LIGHT },
    { "SOFT_LIGHT",	CAIRO_OPERATOR_SOFT_LIGHT },
    { "DIFFERENCE",	CAIRO_OPERATOR_DIFFERENCE },
    { "EXCLUSION",	CAIRO_OPERATOR_EXCLUSION },
    { "HSL_HUE",	CAIRO_OPERATOR_HSL_HUE },
    { "HSL_SATURATION", CAIRO_OPERATOR_HSL_SATURATION },
    { "HSL_COLOR",	CAIRO_OPERATOR_HSL_COLOR },
    { "HSL_LUMINOSITY", CAIRO_OPERATOR_HSL_LUMINOSITY },

    { "WINDING",	CAIRO_FILL_RULE_WINDING },
    { "EVEN_ODD",	CAIRO_FILL_RULE_EVEN_ODD },

    { "ANTIALIAS_DEFAULT",	CAIRO_ANTIALIAS_DEFAULT },
    { "ANTIALIAS_NONE",		CAIRO_ANTIALIAS_NONE },
    { "ANTIALIAS_GRAY",		CAIRO_ANTIALIAS_GRAY },
    { "ANTIALIAS_SUBPIXEL",	CAIRO_ANTIALIAS_SUBPIXEL },
    { "ANTIALIAS_FAST",		CAIRO_ANTIALIAS_FAST },
    { "ANTIALIAS_GOOD",		CAIRO_ANTIALIAS_GOOD },
    { "ANTIALIAS_BEST",		CAIRO_ANTIALIAS_BEST },

    { "LINE_CAP_BUTT",		CAIRO_LINE_CAP_BUTT },
    { "LINE_CAP_ROUND",		CAIRO_LINE_CAP_ROUND },
    { "LINE_CAP_SQUARE",	CAIRO_LINE_CAP_SQUARE },

    { "LINE_JOIN_MITER",	CAIRO_LINE_JOIN_MITER },
    { "LINE_JOIN_ROUND",	CAIRO_LINE_JOIN_ROUND },
    { "LINE_JOIN_BEVEL",	CAIRO_LINE_JOIN_BEVEL },

    { "EXTEND_NONE",		CAIRO_EXTEND_NONE },
    { "EXTEND_REPEAT",		CAIRO_EXTEND_REPEAT },
    { "EXTEND_REFLECT",		CAIRO_EXTEND_REFLECT },
    { "EXTEND_PAD",		CAIRO_EXTEND_PAD },

    { "FILTER_FAST",		CAIRO_FILTER_FAST },
    { "FILTER_GOOD",		CAIRO_FILTER_GOOD },
    { "FILTER_BEST",		CAIRO_FILTER_BEST },
    { "FILTER_BILINEAR",	CAIRO_FILTER_BILINEAR },
    { "FILTER_NEAREST",		CAIRO_FILTER_NEAREST },
    { "FILTER_GAUSSIAN",	CAIRO_FILTER_GAUSSIAN },

    { "SLANT_NORMAL",		CAIRO_FONT_SLANT_NORMAL },
    { "SLANT_ITALIC",		CAIRO_FONT_SLANT_ITALIC },
    { "SLANT_OBLIQUE",		CAIRO_FONT_SLANT_OBLIQUE },

    { "WEIGHT_NORMAL",		CAIRO_FONT_WEIGHT_NORMAL },
    { "WEIGHT_BOLD",		CAIRO_FONT_WEIGHT_BOLD },

    { "SUBPIXEL_ORDER_DEFAULT",	CAIRO_SUBPIXEL_ORDER_DEFAULT },
    { "SUBPIXEL_ORDER_RGB",	CAIRO_SUBPIXEL_ORDER_RGB },
    { "SUBPIXEL_ORDER_BGR",	CAIRO_SUBPIXEL_ORDER_BGR },
    { "SUBPIXEL_ORDER_VRGB",	CAIRO_SUBPIXEL_ORDER_VRGB },
    { "SUBPIXEL_ORDER_VBGR",	CAIRO_SUBPIXEL_ORDER_VBGR },

    { "HINT_STYLE_DEFAULT",	CAIRO_HINT_STYLE_DEFAULT },
    { "HINT_STYLE_NONE",	CAIRO_HINT_STYLE_NONE },
    { "HINT_STYLE_SLIGHT",	CAIRO_HINT_STYLE_SLIGHT },
    { "HINT_STYLE_MEDIUM",	CAIRO_HINT_STYLE_MEDIUM },
    { "HINT_STYLE_FULL",	CAIRO_HINT_STYLE_FULL },

    { "HINT_METRICS_DEFAULT",	CAIRO_HINT_METRICS_DEFAULT },
    { "HINT_METRICS_OFF",	CAIRO_HINT_METRICS_OFF },
    { "HINT_METRICS_ON",	CAIRO_HINT_METRICS_ON },

    { "FORWARD",		0 },
    { "BACKWARD",		1 },

    { "COLOR",			CAIRO_CONTENT_COLOR },
    { "ALPHA",			CAIRO_CONTENT_ALPHA },
    { "COLOR_ALPHA",		CAIRO_CONTENT_COLOR_ALPHA },

    { "A1",			CAIRO_FORMAT_A1 },
    { "A8",			CAIRO_FORMAT_A8 },
    { "RGB16_565",		CAIRO_FORMAT_RGB16_565 },
    { "RGB24",			CAIRO_FORMAT_RGB24 },
    { "ARGB32",			CAIRO_FORMAT_ARGB32 },
    { "RGB30",			CAIRO_FORMAT_RGB30 },
    { "RGB96F",			CAIRO_FORMAT_RGB96F },
    { "RGBA128F",		CAIRO_FORMAT_RGBA128F },
    { "INVALID",		CAIRO_FORMAT_INVALID },

    { NULL, 0 }
};


const csi_integer_constant_def_t *
_csi_integer_constants (void)
{
    return _integer_constants;
}

static const csi_real_constant_def_t
_real_constants[] = {
    { "math.pi",		M_PI },
    { "math.2pi",		2 * M_PI },
    { "math.sqrt2",		M_SQRT2 },
    { "math.ln2",		M_LN2 },

    { NULL, 0 }
};

const csi_real_constant_def_t *
_csi_real_constants (void)
{
    return _real_constants;
}
