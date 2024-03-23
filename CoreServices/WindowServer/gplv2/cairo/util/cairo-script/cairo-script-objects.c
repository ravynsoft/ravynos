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
#include <string.h>

csi_status_t
csi_array_new (csi_t *ctx,
	       csi_integer_t initial_size,
	       csi_object_t *obj)

{
    csi_array_t *array;

    if (ctx->free_array == NULL ||
	ctx->free_array->stack.size <= initial_size)
    {
	csi_status_t status;

	array = _csi_slab_alloc (ctx, sizeof (csi_array_t));
	if (_csi_unlikely (array == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	status = _csi_stack_init (ctx, &array->stack,
				  initial_size ? initial_size : 32);
	if (_csi_unlikely (status)) {
	    _csi_slab_free (ctx, array, sizeof (csi_array_t));
	    return status;
	}
    } else {
	array = ctx->free_array;
	ctx->free_array = NULL;
    }

    array->base.type = CSI_OBJECT_TYPE_ARRAY;
    array->base.ref = 1;

    obj->type = CSI_OBJECT_TYPE_ARRAY;
    obj->datum.array = array;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_array_put (csi_t *ctx,
	       csi_array_t *array,
	       csi_integer_t elem,
	       csi_object_t *value)
{
    if (_csi_unlikely (elem < 0))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    if (_csi_unlikely (elem >= array->stack.len)) {
	csi_status_t status;

	status = _csi_stack_grow (ctx, &array->stack, elem + 1);
	if (_csi_unlikely (status))
	    return status;

	memset (array->stack.objects + array->stack.len,
		0, (elem - array->stack.len + 1) * sizeof (csi_object_t));
	array->stack.len = elem + 1;
    }

    csi_object_free (ctx, &array->stack.objects[elem]);
    array->stack.objects[elem] = *csi_object_reference (value);

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_array_get (csi_t *ctx,
	       csi_array_t *array,
	       csi_integer_t elem,
	       csi_object_t *value)
{
    if (_csi_unlikely (elem < 0))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    if (_csi_unlikely (elem > array->stack.len))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *value = array->stack.objects[elem];
    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_array_append (csi_t *ctx,
		  csi_array_t *array,
		  csi_object_t *obj)
{
    return _csi_stack_push (ctx, &array->stack, csi_object_reference (obj));
}

inline csi_status_t
_csi_array_execute (csi_t *ctx, csi_array_t *array)
{
    csi_integer_t i;
    csi_status_t status;

    if (_csi_unlikely (array->stack.len == 0))
	return CSI_STATUS_SUCCESS;

    for (i = 0; i < array->stack.len; i++) {
	csi_object_t *obj = &array->stack.objects[i];

	if (obj->type & CSI_OBJECT_ATTR_EXECUTABLE) {
	    if (obj->type == (CSI_OBJECT_TYPE_ARRAY |
			      CSI_OBJECT_ATTR_EXECUTABLE))
	    {
		status = _csi_push_ostack_copy (ctx, &array->stack.objects[i]);
	    }
	    else
		status = csi_object_execute (ctx, &array->stack.objects[i]);
	} else
	    status = _csi_push_ostack_copy (ctx, &array->stack.objects[i]);
	if (_csi_unlikely (status))
	    return status;
    }

    return CSI_STATUS_SUCCESS;
}

void
csi_array_free (csi_t *ctx, csi_array_t *array)
{
#if CSI_DEBUG_MALLOC
    _csi_stack_fini (ctx, &array->stack);
    _csi_slab_free (ctx, array, sizeof (csi_array_t));
#else
    csi_integer_t n;

    for (n = 0; n < array->stack.len; n++)
	csi_object_free (ctx, &array->stack.objects[n]);
    array->stack.len = 0;

    if (ctx->free_array != NULL) {
	if (array->stack.size > ctx->free_array->stack.size) {
	    csi_array_t *tmp = ctx->free_array;
	    ctx->free_array = array;
	    array = tmp;
	}

	_csi_stack_fini (ctx, &array->stack);
	_csi_slab_free (ctx, array, sizeof (csi_array_t));
    } else
	ctx->free_array = array;
#endif
}

static cairo_bool_t
_dictionary_name_equal (const void *_a, const void *_b)
{
    return TRUE;
}

csi_status_t
csi_dictionary_new (csi_t *ctx,
		    csi_object_t *obj)

{
    csi_dictionary_t *dict;

    if (ctx->free_dictionary != NULL) {
	dict = ctx->free_dictionary;
	ctx->free_dictionary = NULL;
    } else {
	csi_status_t status;

	dict = _csi_slab_alloc (ctx, sizeof (csi_dictionary_t));
	if (_csi_unlikely (dict == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	status = _csi_hash_table_init (&dict->hash_table,
				       _dictionary_name_equal);
	if (_csi_unlikely (status)) {
	    _csi_slab_free (ctx, dict, sizeof (csi_dictionary_t));
	    return status;
	}
    }

    dict->base.type = CSI_OBJECT_TYPE_DICTIONARY;
    dict->base.ref = 1;

    obj->type = CSI_OBJECT_TYPE_DICTIONARY;
    obj->datum.dictionary = dict;

    return CSI_STATUS_SUCCESS;
}

struct _dictionary_entry_pluck {
    csi_t *ctx;
    csi_hash_table_t *hash_table;
};

static void
_dictionary_entry_pluck (void *entry, void *data)
{
    csi_dictionary_entry_t *dict_entry;
    struct _dictionary_entry_pluck *pluck_data;

    dict_entry = entry;
    pluck_data = data;

    _csi_hash_table_remove (pluck_data->hash_table, entry);
    csi_object_free (pluck_data->ctx, &dict_entry->value);
    _csi_slab_free (pluck_data->ctx, entry, sizeof (csi_dictionary_entry_t));
}

void
csi_dictionary_free (csi_t *ctx,
		     csi_dictionary_t *dict)
{
    struct _dictionary_entry_pluck data;

    data.ctx = ctx;
    data.hash_table = &dict->hash_table;
    _csi_hash_table_foreach (&dict->hash_table,
			     _dictionary_entry_pluck,
			     &data);

#if CSI_DEBUG_MALLOC
    _csi_hash_table_fini (&dict->hash_table);
    _csi_slab_free (ctx, dict, sizeof (csi_dictionary_t));
#else
    if (ctx->free_dictionary != NULL) {
	_csi_hash_table_fini (&dict->hash_table);
	_csi_slab_free (ctx, dict, sizeof (csi_dictionary_t));
    } else
	ctx->free_dictionary = dict;
#endif
}

csi_status_t
csi_dictionary_put (csi_t *ctx,
		    csi_dictionary_t *dict,
		    csi_name_t name,
		    csi_object_t *value)
{
    csi_dictionary_entry_t *entry;
    csi_status_t status;

    entry = _csi_hash_table_lookup (&dict->hash_table,
				    (csi_hash_entry_t *) &name);
    if (entry != NULL) {
	/* replace the existing entry */
	csi_object_free (ctx, &entry->value);
	entry->value = *csi_object_reference (value);
	return CSI_STATUS_SUCCESS;
    }

    entry = _csi_slab_alloc (ctx, sizeof (*entry));
    if (_csi_unlikely (entry == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    entry->hash_entry.hash = name;
    status = _csi_hash_table_insert (&dict->hash_table, &entry->hash_entry);
    if (_csi_unlikely (status)) {
	_csi_slab_free (ctx, entry, sizeof (*entry));
	return status;
    }

    entry->value = *csi_object_reference (value);

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_dictionary_get (csi_t *ctx,
		    csi_dictionary_t *dict,
		    csi_name_t name,
		    csi_object_t *value)
{
    csi_dictionary_entry_t *entry;

    entry = _csi_hash_table_lookup (&dict->hash_table,
				    (csi_hash_entry_t *) &name);
    if (_csi_unlikely (entry == NULL))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    *value = entry->value;
    return CSI_STATUS_SUCCESS;
}

csi_boolean_t
csi_dictionary_has (csi_dictionary_t *dict,
		    csi_name_t name)
{
    return _csi_hash_table_lookup (&dict->hash_table,
				   (csi_hash_entry_t *) &name) != NULL;
}

void
csi_dictionary_remove (csi_t *ctx,
		       csi_dictionary_t *dict,
		       csi_name_t name)
{
    csi_dictionary_entry_t *entry;

    entry = _csi_hash_table_lookup (&dict->hash_table,
				    (csi_hash_entry_t *) &name);
    if (entry != NULL) {
	_csi_hash_table_remove (&dict->hash_table, &entry->hash_entry);
	csi_object_free (ctx, &entry->value);
	_csi_slab_free (ctx, entry, sizeof (csi_dictionary_entry_t));
    }
}

csi_status_t
csi_matrix_new (csi_t *ctx,
		csi_object_t *obj)
{
    csi_matrix_t *matrix;

    matrix = _csi_slab_alloc (ctx, sizeof (csi_matrix_t));
    if (_csi_unlikely (matrix == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    matrix->base.type = CSI_OBJECT_TYPE_MATRIX;
    matrix->base.ref = 1;
    cairo_matrix_init_identity (&matrix->matrix);

    obj->type = CSI_OBJECT_TYPE_MATRIX;
    obj->datum.matrix = matrix;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_matrix_new_from_array (csi_t *ctx,
			   csi_object_t *obj,
			   csi_array_t *array)
{
    csi_matrix_t *matrix;

    if (_csi_unlikely (array->stack.len != 6))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    matrix = _csi_slab_alloc (ctx, sizeof (csi_matrix_t));
    if (_csi_unlikely (matrix == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    matrix->base.type = CSI_OBJECT_TYPE_MATRIX;
    matrix->base.ref = 1;
    cairo_matrix_init (&matrix->matrix,
		       csi_number_get_value (&array->stack.objects[0]),
		       csi_number_get_value (&array->stack.objects[1]),
		       csi_number_get_value (&array->stack.objects[2]),
		       csi_number_get_value (&array->stack.objects[3]),
		       csi_number_get_value (&array->stack.objects[4]),
		       csi_number_get_value (&array->stack.objects[5]));

    obj->type = CSI_OBJECT_TYPE_MATRIX;
    obj->datum.matrix = matrix;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_matrix_new_from_matrix (csi_t *ctx,
			    csi_object_t *obj,
			    const cairo_matrix_t *m)
{
    csi_matrix_t *matrix;

    matrix = _csi_slab_alloc (ctx, sizeof (csi_matrix_t));
    if (_csi_unlikely (matrix == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    matrix->base.type = CSI_OBJECT_TYPE_MATRIX;
    matrix->base.ref = 1;
    matrix->matrix = *m;

    obj->type = CSI_OBJECT_TYPE_MATRIX;
    obj->datum.matrix = matrix;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_matrix_new_from_values (csi_t *ctx,
			    csi_object_t *obj,
			    double v[6])
{
    csi_matrix_t *matrix;

    matrix = _csi_slab_alloc (ctx, sizeof (csi_matrix_t));
    if (_csi_unlikely (matrix == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    matrix->base.type = CSI_OBJECT_TYPE_MATRIX;
    matrix->base.ref = 1;
    cairo_matrix_init (&matrix->matrix, v[0], v[1], v[2], v[3], v[4], v[5]);

    obj->type = CSI_OBJECT_TYPE_MATRIX;
    obj->datum.matrix = matrix;

    return CSI_STATUS_SUCCESS;
}

void
csi_matrix_free (csi_t *ctx,
		 csi_matrix_t *obj)
{
    _csi_slab_free (ctx, obj, sizeof (csi_matrix_t));
}


csi_status_t
csi_name_new (csi_t *ctx,
	      csi_object_t *obj,
	      const char *str,
	      int len)
{
    csi_status_t status;

    status = _csi_intern_string (ctx, &str, len);
    if (_csi_unlikely (status))
	return status;

    obj->type = CSI_OBJECT_TYPE_NAME;
    obj->datum.name = (csi_name_t) str;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_name_new_static (csi_t *ctx,
		     csi_object_t *obj,
		     const char *str)
{
    csi_status_t status;

    status = _csi_intern_string (ctx, &str, strlen (str));
    if (_csi_unlikely (status))
	return status;

    obj->type = CSI_OBJECT_TYPE_NAME;
    obj->datum.name = (csi_name_t) str;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_string_new (csi_t *ctx,
		csi_object_t *obj,
		const char *str,
		int len)
{
    csi_string_t *string;

    if (len < 0)
	len = strlen (str);
    if (_csi_unlikely (len >= INT_MAX))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    if (ctx->free_string == NULL || ctx->free_string->len <= len) {
	string = _csi_slab_alloc (ctx, sizeof (csi_string_t));
	if (_csi_unlikely (string == NULL))
	    return _csi_error (CSI_STATUS_NO_MEMORY);

	string->string = _csi_alloc (ctx, len + 1);
	if (_csi_unlikely (string->string == NULL)) {
	    _csi_slab_free (ctx, string, sizeof (csi_string_t));
	    return _csi_error (CSI_STATUS_NO_MEMORY);
	}
    } else {
	string = ctx->free_string;
	ctx->free_string = NULL;
    }

    if (str != NULL) {
	memcpy (string->string, str, len);
	string->string[len] = '\0';
    }
    string->len = len;
    string->deflate = 0;
    string->method = NONE;

    string->base.type = CSI_OBJECT_TYPE_STRING;
    string->base.ref = 1;

    obj->type = CSI_OBJECT_TYPE_STRING;
    obj->datum.string = string;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_string_deflate_new (csi_t *ctx,
			csi_object_t *obj,
			void *bytes,
			int in_len,
			int out_len)
{
    csi_status_t status;
    csi_string_t *string;

    status = csi_string_new (ctx, obj, bytes, in_len);
    if (_csi_unlikely (status))
	return status;

    string = obj->datum.string;
    string->deflate = out_len;
    string->method = ZLIB;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
csi_string_new_from_bytes (csi_t *ctx,
	                   csi_object_t *obj,
			   char *bytes,
			   unsigned int len)
{
    csi_string_t *string;

    if (_csi_unlikely (len >= INT_MAX))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    string = _csi_slab_alloc (ctx, sizeof (csi_string_t));
    if (_csi_unlikely (string == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    string->string = bytes;
    string->len = len;
    string->deflate = 0;
    string->method = NONE;

    string->base.type = CSI_OBJECT_TYPE_STRING;
    string->base.ref = 1;

    obj->type = CSI_OBJECT_TYPE_STRING;
    obj->datum.string = string;

    return CSI_STATUS_SUCCESS;
}

static inline csi_status_t
_csi_string_execute (csi_t *ctx, csi_string_t *string)
{
    csi_status_t status;
    csi_object_t obj;

    if (_csi_unlikely (string->len == 0))
	return CSI_STATUS_SUCCESS;

    status = csi_file_new_for_bytes (ctx, &obj, string->string, string->len);
    if (_csi_unlikely (status))
	return status;

    status = _csi_scan_file (ctx, obj.datum.file);
    csi_object_free (ctx, &obj);

    return status;
}

void
csi_string_free (csi_t *ctx, csi_string_t *string)
{
#if CSI_DEBUG_MALLOC
    _csi_free (ctx, string->string);
    _csi_slab_free (ctx, string, sizeof (csi_string_t));
#else
    if (ctx->free_string != NULL) {
	if (string->len > ctx->free_string->len) {
	    csi_string_t *tmp = ctx->free_string;
	    ctx->free_string = string;
	    string = tmp;
	}

	_csi_free (ctx, string->string);
	_csi_slab_free (ctx, string, sizeof (csi_string_t));
    } else
	ctx->free_string = string;
#endif
}

csi_status_t
csi_object_execute (csi_t *ctx, csi_object_t *obj)
{
    csi_status_t status;
    csi_object_t indirect;

 INDIRECT:
    switch (obj->type & CSI_OBJECT_TYPE_MASK) {
    case CSI_OBJECT_TYPE_NAME:
	status = _csi_name_lookup (ctx, obj->datum.name, &indirect);
	if (_csi_unlikely (status))
	    return status;
	if (indirect.type & CSI_OBJECT_ATTR_EXECUTABLE) {
	    obj = &indirect;
	    goto INDIRECT;
	} else
	    return _csi_push_ostack_copy (ctx, &indirect);

    case CSI_OBJECT_TYPE_OPERATOR:
	return obj->datum.op (ctx);

    case CSI_OBJECT_TYPE_ARRAY:
	return _csi_array_execute (ctx, obj->datum.array);
    case CSI_OBJECT_TYPE_FILE:
	return _csi_file_execute (ctx, obj->datum.file);
    case CSI_OBJECT_TYPE_STRING:
	return _csi_string_execute (ctx, obj->datum.string);

    default:
	return _csi_push_ostack_copy (ctx, obj);
    }
}

csi_object_t *
csi_object_reference (csi_object_t *obj)
{
    if (CSI_OBJECT_IS_CAIRO (obj)) {
	switch (obj->type & CSI_OBJECT_TYPE_MASK) {
	case CSI_OBJECT_TYPE_CONTEXT:
	    cairo_reference (obj->datum.cr);
	    break;
	case CSI_OBJECT_TYPE_FONT:
	    cairo_font_face_reference (obj->datum.font_face);
	    break;
	case CSI_OBJECT_TYPE_PATTERN:
	    cairo_pattern_reference (obj->datum.pattern);
	    break;
	case CSI_OBJECT_TYPE_SCALED_FONT:
	    cairo_scaled_font_reference (obj->datum.scaled_font);
	    break;
	case CSI_OBJECT_TYPE_SURFACE:
	    cairo_surface_reference (obj->datum.surface);
	    break;
	}
    } else if (CSI_OBJECT_IS_COMPOUND (obj)) {
	obj->datum.object->ref++;
    }

    return obj;
}

void
csi_object_free (csi_t *ctx,
		 csi_object_t *obj)
{
    if (CSI_OBJECT_IS_CAIRO (obj)) {
	switch (obj->type & CSI_OBJECT_TYPE_MASK) {
	case CSI_OBJECT_TYPE_CONTEXT:
	    cairo_destroy (obj->datum.cr);
	    break;
	case CSI_OBJECT_TYPE_FONT:
	    cairo_font_face_destroy (obj->datum.font_face);
	    break;
	case CSI_OBJECT_TYPE_PATTERN:
	    cairo_pattern_destroy (obj->datum.pattern);
	    break;
	case CSI_OBJECT_TYPE_SCALED_FONT:
	    cairo_scaled_font_destroy (obj->datum.scaled_font);
	    break;
	case CSI_OBJECT_TYPE_SURFACE:
	    cairo_surface_destroy (obj->datum.surface);
	    break;
	}
    } else if (CSI_OBJECT_IS_COMPOUND (obj)) {
	if (--obj->datum.object->ref)
	    return;

	switch (obj->type & CSI_OBJECT_TYPE_MASK) {
	case CSI_OBJECT_TYPE_ARRAY:
	    csi_array_free (ctx, obj->datum.array);
	    break;
	case CSI_OBJECT_TYPE_DICTIONARY:
	    csi_dictionary_free (ctx, obj->datum.dictionary);
	    break;
	case CSI_OBJECT_TYPE_FILE:
	    _csi_file_free (ctx, obj->datum.file);
	    break;
	case CSI_OBJECT_TYPE_MATRIX:
	    csi_matrix_free (ctx, obj->datum.matrix);
	    break;
	case CSI_OBJECT_TYPE_STRING:
	    csi_string_free (ctx, obj->datum.string);
	    break;
	default:
	    break;
	}
    }
}

csi_status_t
csi_object_as_file (csi_t *ctx,
		    csi_object_t *src,
		    csi_object_t *file)
{
    int type = csi_object_get_type (src);
    switch (type) {
    case CSI_OBJECT_TYPE_FILE:
	*file = *csi_object_reference (src);
	return CSI_STATUS_SUCCESS;
    case CSI_OBJECT_TYPE_STRING:
	 return csi_file_new_from_string (ctx, file, src->datum.string);
    case CSI_OBJECT_TYPE_ARRAY:
#if 0
	if (src->type & CSI_OBJECT_ATTR_EXECUTABLE)
	    return _csi_file_new_from_procedure (cs, src);
#endif
    default:
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);
    }
}

static int
lexcmp (void const *a, size_t alen,
	void const *b, size_t blen)
{
    size_t len = alen < blen ? alen : blen;
    int cmp = memcmp (a, b, len);
    if (cmp)
	return cmp;
    if (alen == blen)
	return 0;
    return alen < blen ? -1 : +1;
}

csi_boolean_t
csi_object_eq (csi_object_t *a,
	       csi_object_t *b)
{
    csi_object_type_t atype = csi_object_get_type (a);
    csi_object_type_t btype = csi_object_get_type (b);

    if (atype == btype) {
	switch (atype) {
	case CSI_OBJECT_TYPE_BOOLEAN:
	    return a->datum.boolean == b->datum.boolean;
	case CSI_OBJECT_TYPE_INTEGER:
	    return a->datum.integer == b->datum.integer;
	case CSI_OBJECT_TYPE_REAL:
	    return a->datum.real == b->datum.real;
	case CSI_OBJECT_TYPE_NAME:
	    return a->datum.name == b->datum.name;
	case CSI_OBJECT_TYPE_STRING:
	    return 0 == lexcmp (a->datum.string->string,
				a->datum.string->len,
				b->datum.string->string,
				b->datum.string->len);
	case CSI_OBJECT_TYPE_NULL:
	case CSI_OBJECT_TYPE_MARK:
	    return TRUE;
	case CSI_OBJECT_TYPE_OPERATOR:
	    return a->datum.op == b->datum.op;
	case CSI_OBJECT_TYPE_ARRAY:
	case CSI_OBJECT_TYPE_DICTIONARY:
	case CSI_OBJECT_TYPE_FILE:
	case CSI_OBJECT_TYPE_MATRIX:
	case CSI_OBJECT_TYPE_CONTEXT:
	case CSI_OBJECT_TYPE_FONT:
	case CSI_OBJECT_TYPE_PATTERN:
	case CSI_OBJECT_TYPE_SCALED_FONT:
	case CSI_OBJECT_TYPE_SURFACE:
	    return a->datum.ptr == b->datum.ptr;
	}
    }

    if (atype < btype) {
	csi_object_t *c;
	csi_object_type_t ctype;
	c = a; a = b; b = c;
	ctype = atype; atype = btype; btype = ctype;
    }

    switch ((int) atype) {
    case CSI_OBJECT_TYPE_INTEGER:
	if (btype == CSI_OBJECT_TYPE_BOOLEAN) {
	    return a->datum.integer == b->datum.boolean;
	}
	break;
    case CSI_OBJECT_TYPE_REAL:
	if (btype == CSI_OBJECT_TYPE_INTEGER) {
	    return a->datum.real == b->datum.integer;
	}
	else if (btype == CSI_OBJECT_TYPE_BOOLEAN) {
	    return a->datum.real == b->datum.boolean;
	}
	break;

    case CSI_OBJECT_TYPE_STRING:
	if (btype == CSI_OBJECT_TYPE_NAME) {
	    const char *bstr = (const char *) b->datum.name;
	    return 0 == lexcmp (a->datum.string->string,
				a->datum.string->len,
				bstr,
				strlen (bstr));
	}
	break;

    default:
	break;
    }

    return FALSE;
}

csi_status_t
csi_object_compare (csi_object_t *a,
		    csi_object_t *b,
		    int *out)
{
    csi_object_type_t atype = csi_object_get_type (a);
    csi_object_type_t btype = csi_object_get_type (b);
    int sign;

    if (csi_object_eq (a, b)){
	*out = 0;
	return CSI_STATUS_SUCCESS;
    }

#define CMP(x,y) ((x) < (y) ? -1 : +1)

    if (atype == btype) {
	switch (atype) {
	case CSI_OBJECT_TYPE_BOOLEAN:
	    *out = CMP (a->datum.boolean, b->datum.boolean);
	    return CSI_STATUS_SUCCESS;
	case CSI_OBJECT_TYPE_INTEGER:
	    *out = CMP (a->datum.integer, b->datum.integer);
	    return CSI_STATUS_SUCCESS;
	case CSI_OBJECT_TYPE_REAL:
	    *out = CMP (a->datum.real, b->datum.real);
	    return CSI_STATUS_SUCCESS;
	case CSI_OBJECT_TYPE_NAME: {
	    const char *x = (char const *) a->datum.name;
	    const char *y = (char const *) b->datum.name;
	    *out = lexcmp (x, strlen(x), y, strlen (y));
	    return CSI_STATUS_SUCCESS;
	}
	case CSI_OBJECT_TYPE_STRING:
	    *out = lexcmp (a->datum.string->string,
			   a->datum.string->len,
			   b->datum.string->string,
			   b->datum.string->len);
	    return CSI_STATUS_SUCCESS;
	case CSI_OBJECT_TYPE_NULL:
	case CSI_OBJECT_TYPE_MARK:
	case CSI_OBJECT_TYPE_OPERATOR:
	case CSI_OBJECT_TYPE_ARRAY:
	case CSI_OBJECT_TYPE_DICTIONARY:
	case CSI_OBJECT_TYPE_FILE:
	case CSI_OBJECT_TYPE_MATRIX:
	case CSI_OBJECT_TYPE_CONTEXT:
	case CSI_OBJECT_TYPE_FONT:
	case CSI_OBJECT_TYPE_PATTERN:
	case CSI_OBJECT_TYPE_SCALED_FONT:
	case CSI_OBJECT_TYPE_SURFACE:
	    goto TYPE_CHECK_ERROR;
	}
    }

    sign = +1;
    if (atype < btype) {
	csi_object_t *c;
	csi_object_type_t ctype;
	c = a; a = b; b = c;
	ctype = atype; atype = btype; btype = ctype;
	sign = -1;
    }

    switch ((int) atype) {
    case CSI_OBJECT_TYPE_INTEGER:
	if (btype == CSI_OBJECT_TYPE_BOOLEAN) {
	    *out = sign * CMP (a->datum.integer, !!b->datum.boolean);
	    return CSI_STATUS_SUCCESS;
	}
	break;
    case CSI_OBJECT_TYPE_REAL:
	if (btype == CSI_OBJECT_TYPE_INTEGER) {
	    *out = sign * CMP (a->datum.real, b->datum.integer);
	    return CSI_STATUS_SUCCESS;
	}
	else if (btype == CSI_OBJECT_TYPE_BOOLEAN) {
	    *out = sign * CMP (a->datum.real, !!b->datum.boolean);
	    return CSI_STATUS_SUCCESS;
	}
	break;

    case CSI_OBJECT_TYPE_STRING:
	if (btype == CSI_OBJECT_TYPE_NAME) {
	    const char *bstr = (const char *) b->datum.name;
	    *out = sign * lexcmp (a->datum.string->string,
				  a->datum.string->len,
				  bstr,
				  strlen (bstr));
	    return CSI_STATUS_SUCCESS;
	}
	break;

    default:
	break;
    }

#undef CMP

 TYPE_CHECK_ERROR:
    return _csi_error (CSI_STATUS_SCRIPT_INVALID_TYPE);
}
