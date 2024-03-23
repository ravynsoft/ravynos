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
_csi_stack_init (csi_t *ctx, csi_stack_t *stack, csi_integer_t size)
{
    csi_status_t status = CSI_STATUS_SUCCESS;

    stack->len = 0;
    stack->size = size;
    /* assert ((unsigned) size < INT_MAX / sizeof (csi_object_t)); */
    stack->objects = _csi_alloc (ctx, size * sizeof (csi_object_t));
    if (_csi_unlikely (stack->objects == NULL))
	status = _csi_error (CSI_STATUS_NO_MEMORY);

    return status;
}

void
_csi_stack_fini (csi_t *ctx, csi_stack_t *stack)
{
    csi_integer_t n;

    for (n = 0; n < stack->len; n++)
	csi_object_free (ctx, &stack->objects[n]);

    _csi_free (ctx, stack->objects);
}

csi_status_t
_csi_stack_roll (csi_t *ctx,
		 csi_stack_t *stack,
		 csi_integer_t mod, csi_integer_t n)
{
    csi_object_t stack_copy[128];
    csi_object_t *copy;
    csi_integer_t last, i, len;

    switch (mod) { /* special cases */
    case 1:
	last = stack->len - 1;
	stack_copy[0] = stack->objects[last];
	for (i = last; --n; i--)
	    stack->objects[i] = stack->objects[i-1];
	stack->objects[i] = stack_copy[0];
	return CSI_STATUS_SUCCESS;
    case -1:
	last = stack->len - 1;
	stack_copy[0] = stack->objects[i = last - n + 1];
	for (; --n; i++)
	    stack->objects[i] = stack->objects[i+1];
	stack->objects[i] = stack_copy[0];
	return CSI_STATUS_SUCCESS;
    }

    /* fall back to a copy */
    if (n > ARRAY_LENGTH (stack_copy)) {
	if (_csi_unlikely ((unsigned) n > INT_MAX / sizeof (csi_object_t)))
	    return _csi_error (CSI_STATUS_NO_MEMORY);
	copy = _csi_alloc (ctx, n * sizeof (csi_object_t));
	if (copy == NULL)
	    return _csi_error (CSI_STATUS_NO_MEMORY);
    } else
	copy = stack_copy;

    i = stack->len - n;
    memcpy (copy, stack->objects + i, n * sizeof (csi_object_t));
    mod = -mod;
    if (mod < 0)
	mod += n;
    last = mod;
    for (len = n; n--; i++) {
	stack->objects[i] = copy[last];
	if (++last == len)
	    last = 0;
    }

    if (copy != stack_copy)
	_csi_free (ctx, copy);

    return CSI_STATUS_SUCCESS;
}

csi_status_t
_csi_stack_grow (csi_t *ctx, csi_stack_t *stack, csi_integer_t cnt)
{
    csi_integer_t newsize;
    csi_object_t *newstack;

    if (_csi_likely (cnt <= stack->size))
	return CSI_STATUS_SUCCESS;
    if (_csi_unlikely ((unsigned) cnt >= INT_MAX / sizeof (csi_object_t)))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    newsize = stack->size;
    do {
	newsize *= 2;
    } while (newsize <= cnt);

    newstack = _csi_realloc (ctx,
			     stack->objects,
			     newsize * sizeof (csi_object_t));
    if (_csi_unlikely (newstack == NULL))
	return _csi_error (CSI_STATUS_NO_MEMORY);

    stack->objects = newstack;
    stack->size  = newsize;

    return CSI_STATUS_SUCCESS;
}

csi_status_t
_csi_stack_push_internal (csi_t *ctx, csi_stack_t *stack,
			  const csi_object_t *obj)
{
    csi_status_t status;

    status = _csi_stack_grow (ctx, stack, stack->size + 1);
    if (_csi_unlikely (status))
	return status;

    stack->objects[stack->len++] = *obj;
    return CSI_STATUS_SUCCESS;
}

csi_object_t *
_csi_stack_peek (csi_stack_t *stack, csi_integer_t i)
{
    if (_csi_unlikely (stack->len < i))
	return NULL;

    return &stack->objects[stack->len - i -1];
}

void
_csi_stack_pop (csi_t *ctx, csi_stack_t *stack, csi_integer_t count)
{
    if (_csi_unlikely (stack->len < count))
	count = stack->len;

    while (count--)
	csi_object_free (ctx, &stack->objects[--stack->len]);
}

csi_status_t
_csi_stack_exch (csi_stack_t *stack)
{
    csi_object_t tmp;
    csi_integer_t n;

    if (_csi_unlikely (stack->len < 2))
	return _csi_error (CSI_STATUS_INVALID_SCRIPT);

    n = stack->len - 1;
    tmp = stack->objects[n];
    stack->objects[n] = stack->objects[n - 1];
    stack->objects[n - 1] = tmp;

    return CSI_STATUS_SUCCESS;
}
