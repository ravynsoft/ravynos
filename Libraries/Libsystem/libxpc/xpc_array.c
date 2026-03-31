/*
 * Copyright 2014-2015 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <xpc/xpc.h>
#include "xpc_internal.h"

xpc_object_t
xpc_array_create(const xpc_object_t *objects, size_t count)
{
	struct xpc_object *xo;
	size_t i;
	xpc_u val;

	xo = _xpc_prim_create(_XPC_TYPE_ARRAY, val, 0);
	
	for (i = 0; i < count; i++)
		xpc_array_append_value(xo, objects[i]);

	return (xo);
}

void
xpc_array_set_value(xpc_object_t xarray, size_t index, xpc_object_t value)
{
	struct xpc_object *xo, *xotmp, *xotmp2;
	struct xpc_array_head *arr;
	size_t i;

	xo = xarray;
	arr = &xo->xo_array;
	i = 0;

	if (index == XPC_ARRAY_APPEND)
		return xpc_array_append_value(xarray, value);

	if (index >= (size_t)xo->xo_size)
		return;

	TAILQ_FOREACH_SAFE(xotmp, arr, xo_link, xotmp2) {
		if (i++ == index) {
			TAILQ_INSERT_AFTER(arr, (struct xpc_object *)value,
			    xotmp, xo_link);
			TAILQ_REMOVE(arr, xotmp, xo_link);
			xpc_retain(value);
			free(xotmp);
			break;
		}
	}
}
	
void
xpc_array_append_value(xpc_object_t xarray, xpc_object_t value)
{
	struct xpc_object *xo;
	struct xpc_array_head *arr;
	
	xo = xarray;
	arr = &xo->xo_array;

	TAILQ_INSERT_TAIL(arr, (struct xpc_object *)value, xo_link);
	xpc_retain(value);
}


xpc_object_t
xpc_array_get_value(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xo, *xotmp;
	struct xpc_array_head *arr;
	size_t i;

	xo = xarray;
	arr = &xo->xo_array;
	i = 0;

	if (index > xo->xo_size)
		return (NULL);
	
	TAILQ_FOREACH(xotmp, arr, xo_link) {
		if (i++ == index)
			return (xotmp);
	}

	return (NULL);
}

size_t
xpc_array_get_count(xpc_object_t xarray)
{
	struct xpc_object *xo;

	xo = xarray;
	return (xo->xo_size);
}

void
xpc_array_set_bool(xpc_object_t xarray, size_t index, bool value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_bool_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}


void
xpc_array_set_int64(xpc_object_t xarray, size_t index, int64_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_int64_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_uint64(xpc_object_t xarray, size_t index, uint64_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_uint64_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_double(xpc_object_t xarray, size_t index, double value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_double_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_date(xpc_object_t xarray, size_t index, int64_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_date_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_data(xpc_object_t xarray, size_t index, const void *data,
    size_t length)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_data_create(data, length);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_string(xpc_object_t xarray, size_t index, const char *string)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_string_create(string);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_uuid(xpc_object_t xarray, size_t index, const uuid_t value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	xotmp = xpc_uuid_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_fd(xpc_object_t xarray, size_t index, int value)
{
	struct xpc_object *xo, *xotmp;

	xo = xarray;
	//xotmp = xpc_fd_create(value);
	return (xpc_array_set_value(xarray, index, xotmp));
}

void
xpc_array_set_connection(xpc_object_t xarray, size_t index,
    xpc_connection_t value)
{

}

bool
xpc_array_get_bool(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_bool_get_value(xotmp));
}

int64_t
xpc_array_get_int64(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_int64_get_value(xotmp));
}

uint64_t
xpc_array_get_uint64(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_uint64_get_value(xotmp));
}

double
xpc_array_get_double(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_double_get_value(xotmp));
}

int64_t
xpc_array_get_date(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_date_get_value(xotmp));
}

const void *
xpc_array_get_data(xpc_object_t xarray, size_t index, size_t *length)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	*length = xpc_data_get_length(xotmp);
	return (xpc_data_get_bytes_ptr(xotmp));
}

const uint8_t *
xpc_array_get_uuid(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_uuid_get_bytes(xotmp));
}

const char *
xpc_array_get_string(xpc_object_t xarray, size_t index)
{
	struct xpc_object *xotmp;

	xotmp = xpc_array_get_value(xarray, index);
	return (xpc_string_get_string_ptr(xotmp));
}

int
xpc_array_dup_fd(xpc_object_t array, size_t index)
{
	/* XXX */
	return (-1);
}

xpc_connection_t
xpc_array_get_connection(xpc_object_t array, size_t index)
{
	/* XXX */
	return (NULL);
}

bool
xpc_array_apply(xpc_object_t xarray, xpc_array_applier_t applier)
{
	struct xpc_object *xo, *xotmp;
	struct xpc_array_head *arr;
	size_t i;

	i = 0;
	xo = xarray;
	arr = &xo->xo_array;

	TAILQ_FOREACH(xotmp, arr, xo_link) {
		if (!applier(i++, xotmp))
			return (false);
	}

	return (true);
}
