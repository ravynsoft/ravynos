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

#include <xpc/xpc.h>
#include "blocks_wrapper.h"

bool xpc_dictionary_apply_f(xpc_object_t obj,
    xpc_dictionary_applier_func_t func, void *context)
{
	return (xpc_dictionary_apply(obj, ^(const char *key, xpc_object_t val) {
	    return func(key, val, context);
	}));
}

bool xpc_array_apply_f(xpc_object_t obj, xpc_array_applier_func_t func,
    void *context)
{
	return (xpc_array_apply(obj, ^(size_t index, xpc_object_t val) {
	    return func(index, val, context);
	}));
}

void xpc_connection_set_event_handler_f(xpc_connection_t conn,
    xpc_handler_func_t func, void *context)
{
	xpc_connection_set_event_handler(conn, ^(xpc_object_t obj) {
	    func(obj, context);
	});
}
