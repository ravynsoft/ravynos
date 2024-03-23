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

#include "cairo.h"

#include "cairo-script-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef MAX
#define MAX(a,b) (((a)>=(b))?(a):(b))
#endif

csi_status_t
_csi_error (csi_status_t status)
{
    return status;
}

/* XXX track global/local memory, cap etc, mark/sweep GC */
void *
_csi_alloc (csi_t *ctx, int size)
{
    return malloc (size);
}

void *
_csi_alloc0 (csi_t *ctx, int size)
{
    void *ptr;

    ptr = _csi_alloc (ctx, size);
    if (_csi_likely (ptr != NULL))
	memset (ptr, 0, size);

    return ptr;
}

void *
_csi_realloc (csi_t *ctx, void *ptr, int size)
{
    return realloc (ptr, size);
}

void
_csi_free (csi_t *ctx, void *ptr)
{
    if (_csi_unlikely (ptr == NULL))
	return;

    free (ptr);
}

void *
_csi_perm_alloc (csi_t *ctx, int size)
{
    csi_chunk_t *chunk;
    void *ptr;

    size = (size + sizeof (void *)-1) & -sizeof (void *);

    chunk = ctx->perm_chunk;
    if (chunk == NULL || chunk->rem < size) {
	int chunk_size = (size + 8191) & -8192;
	chunk = _csi_alloc (ctx, sizeof (csi_chunk_t) + chunk_size);
	if (_csi_unlikely (chunk == NULL))
	    return NULL;

	chunk->rem = chunk_size;
	chunk->ptr = (char *) (chunk + 1);
	chunk->next = ctx->perm_chunk;
	ctx->perm_chunk = chunk;
    }

    ptr = chunk->ptr;
    chunk->ptr += size;
    chunk->rem -= size;

    return ptr;
}

void *
_csi_slab_alloc (csi_t *ctx, int size)
{
#if CSI_DEBUG_MALLOC
    return malloc (size);
#else
    int chunk_size;
    csi_chunk_t *chunk;
    void *ptr;

    chunk_size = 2 * sizeof (void *);
    chunk_size = (size + chunk_size - 1) / chunk_size;

    if (ctx->slabs[chunk_size].free_list) {
	ptr = ctx->slabs[chunk_size].free_list;
	ctx->slabs[chunk_size].free_list = *(void **) ptr;
	return ptr;
    }

    chunk = ctx->slabs[chunk_size].chunk;
    if (chunk == NULL || ! chunk->rem) {
	int cnt = MAX (128, 8192 / (chunk_size * 2 * sizeof (void *)));

	chunk = _csi_alloc (ctx,
			    sizeof (csi_chunk_t) +
			    cnt * chunk_size * 2 * sizeof (void *));
	if (_csi_unlikely (chunk == NULL))
	    return NULL;

	chunk->rem = cnt;
	chunk->ptr = (char *) (chunk + 1);
	chunk->next = ctx->slabs[chunk_size].chunk;
	ctx->slabs[chunk_size].chunk = chunk;
    }

    ptr = chunk->ptr;
    chunk->ptr += chunk_size * 2 * sizeof (void *);
    chunk->rem--;

    return ptr;
#endif
}

void
_csi_slab_free (csi_t *ctx, void *ptr, int size)
{
    int chunk_size;
    void **free_list;

    if (_csi_unlikely (ptr == NULL))
	return;

#if CSI_DEBUG_MALLOC
    free (ptr);
#else
    chunk_size = 2 * sizeof (void *);
    chunk_size = (size + chunk_size - 1) / chunk_size;

    free_list = ptr;
    *free_list = ctx->slabs[chunk_size].free_list;
    ctx->slabs[chunk_size].free_list = ptr;
#endif
}

csi_status_t
_csi_stack_push (csi_t *ctx, csi_stack_t *stack,
		 const csi_object_t *obj)
{
    if (_csi_unlikely (stack->len == stack->size))
	return _csi_stack_push_internal (ctx, stack, obj);

    stack->objects[stack->len++] = *obj;
    return CSI_STATUS_SUCCESS;
}

static void
_csi_perm_fini (csi_t *ctx)
{
    while (ctx->perm_chunk != NULL) {
	csi_chunk_t *chunk = ctx->perm_chunk;
	ctx->perm_chunk = chunk->next;
	_csi_free (ctx, chunk);
    }
}

static void
_csi_slab_fini (csi_t *ctx)
{
    unsigned int i;

    for (i = 0; i < sizeof (ctx->slabs) / sizeof (ctx->slabs[0]); i++) {
	while (ctx->slabs[i].chunk != NULL) {
	    csi_chunk_t *chunk = ctx->slabs[i].chunk;
	    ctx->slabs[i].chunk = chunk->next;
	    _csi_free (ctx, chunk);
	}
    }
}

static csi_status_t
_add_operator (csi_t *ctx,
	       csi_dictionary_t *dict,
	       const csi_operator_def_t *def)
{
    csi_object_t name;
    csi_object_t operator;
    csi_status_t status;

    status = csi_name_new_static (ctx, &name, def->name);
    if (status)
	return status;

    csi_operator_new (&operator, def->op);

    return csi_dictionary_put (ctx, dict, name.datum.name, &operator);
}

static csi_status_t
_add_integer_constant (csi_t *ctx,
		       csi_dictionary_t *dict,
		       const csi_integer_constant_def_t *def)
{
    csi_object_t name;
    csi_object_t constant;
    csi_status_t status;

    status = csi_name_new_static (ctx, &name, def->name);
    if (status)
	return status;

    csi_integer_new (&constant, def->value);

    return csi_dictionary_put (ctx, dict, name.datum.name, &constant);
}

static csi_status_t
_add_real_constant (csi_t *ctx,
		    csi_dictionary_t *dict,
		    const csi_real_constant_def_t *def)
{
    csi_object_t name;
    csi_object_t constant;
    csi_status_t status;

    status = csi_name_new_static (ctx, &name, def->name);
    if (status)
	return status;

    csi_real_new (&constant, def->value);

    return csi_dictionary_put (ctx, dict, name.datum.name, &constant);
}

static csi_status_t
_init_dictionaries (csi_t *ctx)
{
    csi_status_t status;
    csi_stack_t *stack;
    csi_object_t obj;
    csi_dictionary_t *dict, *opcodes;
    const csi_operator_def_t *odef;
    const csi_integer_constant_def_t *idef;
    const csi_real_constant_def_t *rdef;
    unsigned n;

    stack = &ctx->dstack;

    status = _csi_stack_init (ctx, stack, 4);
    if (_csi_unlikely (status))
	return status;

    /* systemdict */
    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    status = _csi_stack_push (ctx, stack, &obj);
    if (_csi_unlikely (status))
	return status;

    dict = obj.datum.dictionary;

    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;

    opcodes = obj.datum.dictionary;

    n = 0;
    csi_integer_new (&obj, n);
    status = csi_dictionary_put (ctx, opcodes, 0, &obj);
    if (_csi_unlikely (status))
	return status;
    ctx->opcode[n++] = NULL;

    /* fill systemdict with operators */
    for (odef = _csi_operators (); odef->name != NULL; odef++) {
	status = _add_operator (ctx, dict, odef);
	if (_csi_unlikely (status))
	    return status;

	if (! csi_dictionary_has (opcodes, (csi_name_t) odef->op)) {
	    csi_integer_new (&obj, n);
	    status = csi_dictionary_put (ctx,
		                         opcodes, (csi_name_t) odef->op, &obj);
	    if (_csi_unlikely (status))
		return status;

	    assert (n < sizeof (ctx->opcode) / sizeof (ctx->opcode[0]));
	    ctx->opcode[n++] = odef->op;
	}
    }
    csi_dictionary_free (ctx, opcodes);

    /* add constants */
    for (idef = _csi_integer_constants (); idef->name != NULL; idef++) {
	status = _add_integer_constant (ctx, dict, idef);
	if (_csi_unlikely (status))
	    return status;
    }
    for (rdef = _csi_real_constants (); rdef->name != NULL; rdef++) {
	status = _add_real_constant (ctx, dict, rdef);
	if (_csi_unlikely (status))
	    return status;
    }

    /* and seal */
    //dict.type &= ~CSI_OBJECT_ATTR_WRITABLE;


    /* globaldict */
    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;
    status = _csi_stack_push (ctx, stack, &obj);
    if (_csi_unlikely (status))
	return status;

    /* userdict */
    status = csi_dictionary_new (ctx, &obj);
    if (_csi_unlikely (status))
	return status;
    status = _csi_stack_push (ctx, stack, &obj);
    if (_csi_unlikely (status))
	return status;

    return CSI_STATUS_SUCCESS;
}

/* intern string */

typedef struct _cairo_intern_string {
    csi_hash_entry_t hash_entry;
    int len;
    char *string;
} csi_intern_string_t;

static unsigned long
_intern_string_hash (const char *str, int len)
{
    const signed char *p = (const signed char *) str;
    if (len > 0) {
	unsigned int h = *p;

	while (--len)
	    h = (h << 5) - h + *++p;

	return h;
    }
    return 0;
}

static cairo_bool_t
_intern_string_equal (const void *_a, const void *_b)
{
    const csi_intern_string_t *a = _a;
    const csi_intern_string_t *b = _b;

    if (a->len != b->len)
	return FALSE;

    return memcmp (a->string, b->string, a->len) == 0;
}

static void
_csi_init (csi_t *ctx)
{
    csi_status_t status;

    memset (ctx, 0, sizeof (*ctx));

    ctx->status = CSI_STATUS_SUCCESS;
    ctx->ref_count = 1;
    ctx->scanner.line_number = -1;

    status = _csi_hash_table_init (&ctx->strings, _intern_string_equal);
    if (status)
	goto FAIL;

    status = _csi_stack_init (ctx, &ctx->ostack, 2048);
    if (status)
	goto FAIL;
    status = _init_dictionaries (ctx);
    if (status)
	goto FAIL;

    status = _csi_scanner_init (ctx, &ctx->scanner);
    if (status)
	goto FAIL;

    return;

FAIL:
    if (ctx->status == CSI_STATUS_SUCCESS)
	ctx->status = status;
}

static void
_csi_finish (csi_t *ctx)
{
    _csi_stack_fini (ctx, &ctx->ostack);
    _csi_stack_fini (ctx, &ctx->dstack);
    _csi_scanner_fini (ctx, &ctx->scanner);

    _csi_hash_table_fini (&ctx->strings);
}

csi_status_t
_csi_name_define (csi_t *ctx, csi_name_t name, csi_object_t *obj)
{
    return csi_dictionary_put (ctx,
			ctx->dstack.objects[ctx->dstack.len-1].datum.dictionary,
			name,
			obj);
}

csi_status_t
_csi_name_lookup (csi_t *ctx, csi_name_t name, csi_object_t *obj)
{
    int i;

    for (i = ctx->dstack.len; i--; ) {
	csi_dictionary_t *dict;
	csi_dictionary_entry_t *entry;

	dict = ctx->dstack.objects[i].datum.dictionary;
	entry = _csi_hash_table_lookup (&dict->hash_table,
					(csi_hash_entry_t *) &name);
	if (entry != NULL) {
	    *obj = entry->value;
	    return CSI_STATUS_SUCCESS;
	}
    }

    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
}

csi_status_t
_csi_name_undefine (csi_t *ctx, csi_name_t name)
{
    unsigned int i;

    for (i = ctx->dstack.len; --i; ) {
	if (csi_dictionary_has (ctx->dstack.objects[i].datum.dictionary,
				name))
	{
	    csi_dictionary_remove (ctx,
				   ctx->dstack.objects[i].datum.dictionary,
				   name);
	    return CSI_STATUS_SUCCESS;
	}
    }

    return _csi_error (CSI_STATUS_INVALID_SCRIPT);
}

csi_status_t
_csi_intern_string (csi_t *ctx, const char **str_inout, int len)
{
    char *str = (char *) *str_inout;
    csi_intern_string_t tmpl, *istring;
    csi_status_t status = CSI_STATUS_SUCCESS;

    tmpl.hash_entry.hash = _intern_string_hash (str, len);
    tmpl.len = len;
    tmpl.string = (char *) str;

    istring = _csi_hash_table_lookup (&ctx->strings, &tmpl.hash_entry);
    if (istring == NULL) {
	istring = _csi_perm_alloc (ctx,
				   sizeof (csi_intern_string_t) + len + 1);
	if (istring != NULL) {
	    istring->hash_entry.hash = tmpl.hash_entry.hash;
	    istring->len = tmpl.len;
	    istring->string = (char *) (istring + 1);
	    memcpy (istring->string, str, len);
	    istring->string[len] = '\0';

	    status = _csi_hash_table_insert (&ctx->strings,
					     &istring->hash_entry);
	    if (_csi_unlikely (status)) {
		_csi_free (ctx, istring);
		return status;
	    }
	} else
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    }

    *str_inout = istring->string;
    return CSI_STATUS_SUCCESS;
}

/* Public */

static csi_t _csi_nil = { -1, CSI_STATUS_NO_MEMORY };

csi_t *
cairo_script_interpreter_create (void)
{
    csi_t *ctx;

    ctx = malloc (sizeof (csi_t));
    if (ctx == NULL)
	return (csi_t *) &_csi_nil;

    _csi_init (ctx);

    return ctx;
}

void
cairo_script_interpreter_install_hooks (csi_t *ctx,
					const csi_hooks_t *hooks)
{
    if (ctx->status)
	return;

    ctx->hooks = *hooks;
}

cairo_status_t
cairo_script_interpreter_run (csi_t *ctx, const char *filename)
{
    csi_object_t file;

    if (ctx->status)
	return ctx->status;
    if (ctx->finished)
	return ctx->status = CSI_STATUS_INTERPRETER_FINISHED;

    ctx->status = csi_file_new (ctx, &file, filename, "r");
    if (ctx->status)
	return ctx->status;

    file.type |= CSI_OBJECT_ATTR_EXECUTABLE;

    ctx->status = csi_object_execute (ctx, &file);
    csi_object_free (ctx, &file);

    return ctx->status;
}

cairo_status_t
cairo_script_interpreter_feed_stream (csi_t *ctx, FILE *stream)
{
    csi_object_t file;

    if (ctx->status)
	return ctx->status;
    if (ctx->finished)
	return ctx->status = CSI_STATUS_INTERPRETER_FINISHED;

    ctx->status = csi_file_new_for_stream (ctx, &file, stream);
    if (ctx->status)
	return ctx->status;

    file.type |= CSI_OBJECT_ATTR_EXECUTABLE;

    ctx->status = csi_object_execute (ctx, &file);
    csi_object_free (ctx, &file);

    return ctx->status;
}

cairo_status_t
cairo_script_interpreter_feed_string (csi_t *ctx, const char *line, int len)
{
    csi_object_t file;

    if (ctx->status)
	return ctx->status;
    if (ctx->finished)
	return ctx->status = CSI_STATUS_INTERPRETER_FINISHED;

    if (len < 0)
	len = strlen (line);
    ctx->status = csi_file_new_for_bytes (ctx, &file, line, len);
    if (ctx->status)
	return ctx->status;

    file.type |= CSI_OBJECT_ATTR_EXECUTABLE;

    ctx->status = csi_object_execute (ctx, &file);
    csi_object_free (ctx, &file);

    return ctx->status;
}

unsigned int
cairo_script_interpreter_get_line_number (csi_t *ctx)
{
    return ctx->scanner.line_number + 1; /* 1 index based */
}

csi_t *
cairo_script_interpreter_reference (csi_t *ctx)
{
    ctx->ref_count++;
    return ctx;
}

cairo_status_t
cairo_script_interpreter_finish (csi_t *ctx)
{
    csi_status_t status;

    status = ctx->status;
    if (! ctx->finished) {
	_csi_finish (ctx);
	ctx->finished = 1;
    } else if (status == CSI_STATUS_SUCCESS) {
	status = ctx->status = CSI_STATUS_INTERPRETER_FINISHED;
    }

    return status;
}

static void
_csi_fini (csi_t *ctx)
{
    if (! ctx->finished)
	_csi_finish (ctx);

    if (ctx->free_array != NULL)
	csi_array_free (ctx, ctx->free_array);
    if (ctx->free_dictionary != NULL)
	csi_dictionary_free (ctx, ctx->free_dictionary);
    if (ctx->free_string != NULL)
	csi_string_free (ctx, ctx->free_string);

    _csi_slab_fini (ctx);
    _csi_perm_fini (ctx);
}

cairo_status_t
cairo_script_interpreter_destroy (csi_t *ctx)
{
    csi_status_t status;

    status = ctx->status;
    if (--ctx->ref_count)
	return status;

    _csi_fini (ctx);
    free (ctx);

    return status;
}

cairo_status_t
cairo_script_interpreter_translate_stream (FILE *stream,
	                                   cairo_write_func_t write_func,
					   void *closure)
{
    csi_t ctx;
    csi_object_t src;
    csi_status_t status;

    _csi_init (&ctx);

    status = csi_file_new_for_stream (&ctx, &src, stream);
    if (status)
	goto BAIL;

    status = _csi_translate_file (&ctx, src.datum.file, write_func, closure);

BAIL:
    csi_object_free (&ctx, &src);
    _csi_fini (&ctx);

    return status;
}
