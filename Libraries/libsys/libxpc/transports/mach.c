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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mach/mach.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <xpc/xpc.h>
#include <servers/bootstrap.h>

#include "../xpc_internal.h"

#define MAX_RECV 8192

struct xpc_message {
    mach_msg_header_t header;
    char body[];
};

static int
mach_lookup(const char *name, xpc_port_t *local, xpc_port_t *remote)
{
	mach_port_t mp, rmp;
	kern_return_t kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
	    &mp);
	if (kr != KERN_SUCCESS) {
		errno = EPERM;
		return (-1);
	}

	kr = mach_port_insert_right(mach_task_self(), mp, mp,
	    MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS) {
		errno = EPERM;
		return (-1);
	}

	kr = bootstrap_look_up(bootstrap_port, name, &rmp);
	if (kr != KERN_SUCCESS) {
		debugf("bootstrap_look_up failed kr=%d", kr);
		errno = EBUSY;
		return (-1);
	}

	*local = (xpc_port_t)mp;
	*remote = (xpc_port_t)rmp;
	return (0);
}

static int
mach_listen(const char *name, xpc_port_t *port)
{
	mach_port_t mp;
	kern_return_t kr;

	kr = bootstrap_check_in(bootstrap_port, name, &mp);
	if (kr != KERN_SUCCESS) {
		debugf("bootstrap_check_in failed kr=%d", kr);
		errno = EBUSY;
		return (-1);
	}

	*(mach_port_t *)port = mp;
	return (0);
}

static char *
mach_port_to_string(xpc_port_t port)
{
	mach_port_t p = (mach_port_t)port;
	char *ret;

	asprintf(&ret, "<%d>", p);
	return (ret);
}

static int
mach_port_compare(xpc_port_t p1, xpc_port_t p2)
{
	return (mach_port_t)p1 == (mach_port_t)p2;
}

static dispatch_source_t
mach_create_client_source(xpc_port_t port, void *context, dispatch_queue_t tq)
{
	mach_port_t mp = (mach_port_t)port;
	dispatch_source_t ret;

	ret = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV,
	    (uintptr_t)mp, 0, tq);

	dispatch_set_context(ret, context);
	dispatch_source_set_event_handler_f(ret, xpc_connection_recv_message);
	dispatch_source_set_cancel_handler(ret, ^{
	    xpc_connection_destroy_peer(dispatch_get_context(ret));
	});

	return (ret);
}

static dispatch_source_t
mach_create_server_source(xpc_port_t port, void *context, dispatch_queue_t tq)
{
	mach_port_t mp = (mach_port_t)port;
	void *client_ctx;
	dispatch_source_t ret;

	ret = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV,
	    (uintptr_t)mp, 0, tq);

	dispatch_set_context(ret, context);
	dispatch_source_set_event_handler_f(ret,
	    xpc_connection_recv_mach_message);
	
	return (ret);
}

#ifdef __x86_64__
#define _ALIGN(x) (((uint64_t)x + 63) & ~63) /* align to register size */
#else
#error Unsupported platform!
#endif

static int
mach_send(xpc_port_t local, xpc_port_t remote, void *buf, size_t len,
    struct xpc_resource *res, size_t nres)
{
	mach_port_t src, dst;
	struct xpc_object *xo;
	size_t size, msg_size;
	struct xpc_message *message;
	kern_return_t kr;
	int err = 0;

	message = malloc(sizeof(struct xpc_message) + len);
	src = (mach_port_t)local;
	dst = (mach_port_t)remote;

	debugf("local=%d remote=%d", src, dst);

	msg_size = _ALIGN(len + sizeof(struct xpc_message));
	message->header.msgh_size = msg_size;
	message->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
	    MACH_MSG_TYPE_MAKE_SEND);
	message->header.msgh_remote_port = dst;
	message->header.msgh_local_port = src;
	memcpy(&message->body, buf, len);
	kr = mach_msg_send(&message->header);

	if (kr != KERN_SUCCESS) {
		debugf("failed, kr=%d", kr);
		err = (kr == KERN_INVALID_TASK) ? EPIPE : EINVAL;
	}

	free(message);
	return (err);
}

static int
mach_recv(xpc_port_t local, xpc_port_t *remote, void *buf, size_t len,
    struct xpc_resource **res, size_t *nres, struct xpc_credentials *creds)
{
	struct xpc_message *message;
	mach_port_t src;
	mach_msg_header_t *request;
	kern_return_t kr;
	mig_reply_error_t response;
	mach_msg_trailer_t *tr;
	int data_size;
	audit_token_t *auditp;

	message = malloc(sizeof(struct xpc_message) + sizeof(mach_msg_trailer_t) + len);
	src = (mach_port_t)local;

	request = &message->header;
	request->msgh_size = MAX_RECV;
	request->msgh_local_port = src;

	kr = mach_msg(request, MACH_RCV_MSG |
	    MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
	    MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT), 0,
	    request->msgh_size, request->msgh_local_port,
	    MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (kr != 0) {
		debugf("failed, kr=%d", kr);
		return (-1);
	}

	debugf("msgh_size=%d", message->header.msgh_size);

	*remote = (xpc_port_t)request->msgh_remote_port;
	memcpy(buf, &message->body, message->header.msgh_size);

	tr = (mach_msg_trailer_t *)(((char *)&message) + request->msgh_size);
	auditp = &((mach_msg_audit_trailer_t *)tr)->msgh_audit;

	return (message->header.msgh_size - sizeof(struct xpc_message));
}

struct xpc_transport mach_transport = {
    	.xt_name = "mach",
	.xt_listen = mach_listen,
	.xt_lookup = mach_lookup,
    	.xt_port_to_string = mach_port_to_string,
    	.xt_port_compare = mach_port_compare,
    	.xt_create_client_source = mach_create_client_source,
    	.xt_create_server_source = mach_create_server_source,
	.xt_send = mach_send,
	.xt_recv = mach_recv
};
