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
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

int
main(int argc, char *argv[])
{
	xpc_connection_t conn;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <mach service name>\n", argv[0]);
		return (1);
	}

	conn = xpc_connection_create_mach_service(argv[1], NULL,
	    XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(conn, ^(xpc_object_t peer) {
		xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
			printf("Message received: %p\n", event);
			printf("Client UID: %d\n", xpc_connection_get_euid(peer));
			printf("Client GID: %d\n", xpc_connection_get_guid(peer));
			printf("Client PID: %d\n", xpc_connection_get_pid(peer));

			xpc_object_t resp = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(resp, "foo", "bar");
			xpc_connection_send_message(peer, resp);
		});

		xpc_connection_resume(peer);
	});

	xpc_connection_resume(conn);
	dispatch_main();
}
