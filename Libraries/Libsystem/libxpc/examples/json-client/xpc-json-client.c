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

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <xpc/xpc.h>

static xpc_object_t
to_xpc(json_t *json)
{
	size_t idx;
	xpc_object_t arr, dict;
	const char *key;
	json_t *val;

	switch (json_typeof(json)) {
	case JSON_STRING:
		return xpc_string_create(json_string_value(json));

	case JSON_INTEGER:
		return xpc_int64_create(json_integer_value(json));

	case JSON_TRUE:
		return xpc_bool_create(true);

	case JSON_FALSE:
		return xpc_bool_create(false);

	case JSON_ARRAY:
		arr = xpc_array_create(NULL, 0);
		json_array_foreach(json, idx, val) {
			xpc_array_append_value(arr, to_xpc(val));
		}

		return arr;

	case JSON_OBJECT:
		dict = xpc_dictionary_create(NULL, NULL, 0);
		json_object_foreach(json, key, val) {
			xpc_dictionary_set_value(dict, key, to_xpc(val));
		}

		return dict;

	default:
		return xpc_null_create();

	}
}

static json_t *
to_json(xpc_object_t xo)
{
	char *txt;
	json_t *arr, *obj;
	xpc_type_t type;
	size_t i;

	type = xpc_get_type(xo);

	printf("to_json: type=%p\n", type);

	if (type == XPC_TYPE_STRING)
		return json_string(xpc_string_get_string_ptr(xo));

	if (type == XPC_TYPE_INT64)
		return json_integer(xpc_int64_get_value(xo));

	if (type == XPC_TYPE_UINT64)
		return json_integer(xpc_uint64_get_value(xo));		

	if (type == XPC_TYPE_BOOL)
		return json_boolean(xpc_bool_get_value(xo));

	if (type == XPC_TYPE_ARRAY) {
		arr = json_array();
		xpc_array_apply(xo, ^(size_t index, xpc_object_t value) {
			json_array_append_new(arr, to_json(value));
			return ((bool)true);
		});

		return (arr);
	}

	if (type == XPC_TYPE_DICTIONARY) {
		obj = json_object();
		xpc_dictionary_apply(xo, ^(const char *key, xpc_object_t value) {
			json_object_set_new(obj, key, to_json(value));
			return ((bool)true);
		});

		return (obj);
	}
	
	return (json_null());
}

int
main(int argc, char *argv[])
{
	xpc_connection_t conn;
	xpc_object_t msg;
	json_t *json;
	json_error_t error;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <mach service name>\n", argv[0]);
		return (1);
	}

	conn = xpc_connection_create_mach_service(argv[1], NULL, 0);
	if (conn == NULL) {
		perror("xpc_connection_create_mach_service");
		return (1);
	}
	
	xpc_connection_set_event_handler(conn, ^(xpc_object_t obj) {
		printf("Received message in generic event handler: %p\n", obj);
		printf("%s\n", xpc_copy_description(obj));
	});

	xpc_connection_resume(conn);

	json = json_loadf(stdin, JSON_DECODE_ANY, &error);
	msg = to_xpc(json);

	xpc_connection_send_message_with_reply(conn, msg, NULL, ^(xpc_object_t resp) {
		printf("Received message: %p\n", resp);
		json_t *reply;
		reply = to_json(resp);
		json_dumpf(reply, stdout, JSON_INDENT(4));
		printf("\n");
	});

	dispatch_main();
	return (0);
}
