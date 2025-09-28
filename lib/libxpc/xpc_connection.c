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

#include <errno.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <xpc/xpc.h>
#include <machine/atomic.h>
#include <Block.h>
#include "xpc_internal.h"

#define XPC_CONNECTION_NEXT_ID(conn) (atomic_fetchadd_int(&conn->xc_last_id, 1))

static void xpc_connection_recv_message();
static void xpc_send(xpc_connection_t xconn, xpc_object_t message, uint64_t id);

static inline struct xpc_connection *conn_extract(xpc_connection_t object)
{
	struct xpc_object *o = (struct xpc_object *)object;
	return &o->xo_connection;
}

xpc_connection_t
xpc_connection_create(const char *name, dispatch_queue_t targetq)
{
	kern_return_t kr;
	char *qname;
	struct xpc_connection *conn;
	xpc_connection_t rv;

	xpc_u val;
	rv = _xpc_prim_create(_XPC_TYPE_CONNECTION, val, 0);
	if (!rv)
		return NULL;
	conn = conn_extract(rv);

	memset(conn, 0, sizeof(struct xpc_connection));
	conn->xc_last_id = 1;
	TAILQ_INIT(&conn->xc_peers);
	TAILQ_INIT(&conn->xc_pending);

	/* Create send queue */
	asprintf(&qname, "com.ixsystems.xpc.connection.sendq.%p", conn);
	conn->xc_send_queue = dispatch_queue_create(qname, NULL);
	free(qname);

	/* Create recv queue */
	asprintf(&qname, "com.ixsystems.xpc.connection.recvq.%p", conn);
	conn->xc_recv_queue = dispatch_queue_create(qname, NULL);
	free(qname);

	/* Create target queue */
	conn->xc_target_queue = targetq ? targetq : dispatch_get_main_queue();
			/* FIXME: should this be dispatch_get_global_queue(DISPATCH_TARGET_QUEUE_DEFAULT, 0) ?? */

	/* Receive queue is initially suspended */
	dispatch_suspend(conn->xc_recv_queue);

	/* Create local port */
	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
	    &conn->xc_local_port);
	if (kr != KERN_SUCCESS) {
		errno = EPERM;
		return (NULL);
	}

	kr = mach_port_insert_right(mach_task_self(), conn->xc_local_port,
	    conn->xc_local_port, MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS) {
		errno = EPERM;
		return (NULL);
	}

	return (rv);
}

void
xpc_connection_destroy(xpc_connection_t object)
{
	struct xpc_connection *conn = conn_extract(object);
	dispatch_release(conn->xc_send_queue);
	dispatch_release(conn->xc_recv_queue);
	mach_port_deallocate(mach_task_self(), conn->xc_local_port);

	if(conn->xc_handler)
		Block_release(conn->xc_handler);
}

xpc_connection_t
xpc_connection_create_mach_service(const char *name, dispatch_queue_t targetq,
    uint64_t flags)
{
	kern_return_t kr;
	struct xpc_connection *conn;
	xpc_connection_t rv;

	rv = xpc_connection_create(name, targetq);
	if (rv == NULL)
		return (NULL);

	conn = conn_extract(rv);
	conn->xc_flags = flags;

	if (flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		kr = bootstrap_check_in(bootstrap_port, name,
		    &conn->xc_local_port);
		if (kr != KERN_SUCCESS) {
			errno = EBUSY;
			xpc_release(rv);
			return (NULL);
		}

		return (rv);
	}

	if (!strcmp(name, "bootstrap")) {
		conn->xc_remote_port = bootstrap_port;
		return (rv);
	}

	/* Look up named mach service */
	kr = bootstrap_look_up(bootstrap_port, name, &conn->xc_remote_port);
	if (kr != KERN_SUCCESS) {
		errno = ENOENT;
		xpc_release(rv);
		return (NULL);
	}

	return (rv);
}

xpc_connection_t
xpc_connection_create_from_endpoint(xpc_endpoint_t endpoint)
{
	kern_return_t kr;
	struct xpc_connection *conn;
	xpc_connection_t rv;

	rv = xpc_connection_create("anonymous", NULL);
	if (rv == NULL)
		return (NULL);

	conn = conn_extract(rv);
	conn->xc_remote_port = (mach_port_t)endpoint;
	return (rv);
}

void
xpc_connection_set_target_queue(xpc_connection_t xconn,
    dispatch_queue_t targetq)
{
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = conn_extract(xconn);
	conn->xc_target_queue = targetq;	
}

void
xpc_connection_set_event_handler(xpc_connection_t xconn,
    xpc_handler_t handler)
{
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = conn_extract(xconn);
	conn->xc_handler = (xpc_handler_t)Block_copy(handler);
}

void
xpc_connection_suspend(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	dispatch_suspend(conn->xc_recv_source);
}

void
xpc_connection_resume(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = conn_extract(xconn);

	/* Create dispatch source for top-level connection */
	if (conn->xc_parent == NULL) {
		conn->xc_recv_source = dispatch_source_create(
		    DISPATCH_SOURCE_TYPE_MACH_RECV, conn->xc_local_port, 0,
		    conn->xc_recv_queue);
		dispatch_set_context(conn->xc_recv_source, conn);
		dispatch_source_set_event_handler_f(conn->xc_recv_source,
		    xpc_connection_recv_message);
		dispatch_resume(conn->xc_recv_source);
	}

	dispatch_resume(conn->xc_recv_queue);
}

void
xpc_connection_send_message(xpc_connection_t xconn,
    xpc_object_t message)
{
	struct xpc_connection *conn;
	uint64_t id;

	conn = conn_extract(xconn);
	id = xpc_dictionary_get_uint64(message, XPC_SEQID);

	if (id == 0)
		id = XPC_CONNECTION_NEXT_ID(conn);

	xpc_retain(message);
	dispatch_async(conn->xc_send_queue, ^{
		xpc_send(xconn, message, id);
		xpc_release(message);
	});
}

void
xpc_connection_send_message_with_reply(xpc_connection_t xconn,
    xpc_object_t message, dispatch_queue_t targetq, xpc_handler_t handler)
{
	struct xpc_connection *conn;
	struct xpc_pending_call *call;

	conn = conn_extract(xconn);
	call = malloc(sizeof(struct xpc_pending_call));
	call->xp_id = XPC_CONNECTION_NEXT_ID(conn);
	call->xp_handler = handler;
	call->xp_queue = targetq;
	TAILQ_INSERT_TAIL(&conn->xc_pending, call, xp_link);

	xpc_retain(message);
	dispatch_async(conn->xc_send_queue, ^{
		xpc_send(xconn, message, call->xp_id);
		xpc_release(message);
	});

}

xpc_object_t
xpc_connection_send_message_with_reply_sync(xpc_connection_t conn,
    xpc_object_t message)
{
	__block xpc_object_t result;
	dispatch_semaphore_t sem = dispatch_semaphore_create(0);

	xpc_connection_send_message_with_reply(conn, message, NULL,
	    ^(xpc_object_t o) {
		result = o;
		dispatch_semaphore_signal(sem);
	});

	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
	return (result);
}

void
xpc_connection_send_barrier(xpc_connection_t xconn, dispatch_block_t barrier)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	dispatch_sync(conn->xc_send_queue, barrier);
}

void
xpc_connection_cancel(xpc_connection_t connection)
{

}

const char *
xpc_connection_get_name(xpc_connection_t connection)
{

	return ("unknown"); /* ??? */
}

uid_t
xpc_connection_get_euid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	return (conn->xc_remote_euid);
}

gid_t
xpc_connection_get_guid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	return (conn->xc_remote_guid);
}

pid_t
xpc_connection_get_pid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	return (conn->xc_remote_pid);
}

au_asid_t
xpc_connection_get_asid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	return (conn->xc_remote_asid);
}

void
xpc_connection_set_context(xpc_connection_t xconn, void *ctx)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	conn->xc_context = ctx;
}

void *
xpc_connection_get_context(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = conn_extract(xconn);
	return (conn->xc_context);
}

void
xpc_connection_set_finalizer_f(xpc_connection_t connection,
    xpc_finalizer_t finalizer)
{

}

xpc_endpoint_t
xpc_endpoint_create(xpc_connection_t connection)
{

}

void
xpc_main(xpc_connection_handler_t handler)
{

	dispatch_main();
}

void
xpc_transaction_begin(void)
{
	vproc_transaction_begin(NULL);
}

void
xpc_transaction_end(void)
{
	vproc_transaction_end(NULL, NULL);
}

static void
xpc_send(xpc_connection_t xconn, xpc_object_t message, uint64_t id)
{
	struct xpc_connection *conn;
	kern_return_t kr;

	debugf("connection=%p, message=%p, id=%lu", xconn, message, id);

	conn = conn_extract(xconn);
	kr = xpc_pipe_send(message, conn->xc_remote_port,
	    conn->xc_local_port, id);

	if (kr != KERN_SUCCESS)
		debugf("send failed, kr=%d", kr);
}

static void
xpc_connection_set_credentials(struct xpc_connection *conn, audit_token_t *tok)
{
	uid_t uid;
	gid_t gid;
	pid_t pid;
	au_asid_t asid;

	if (tok == NULL)
		return;

#ifdef AUDIT_BSM
	audit_token_to_au32(*tok, NULL, &uid, &gid, NULL, NULL, &pid, &asid,
	    NULL);
#endif

	conn->xc_remote_euid = uid;
	conn->xc_remote_guid = gid;
	conn->xc_remote_pid = pid;
	conn->xc_remote_asid = asid;
}

static void
xpc_connection_recv_message(void *context)
{
	struct xpc_pending_call *call;
	struct xpc_connection *conn, *peer;
	xpc_object_t result;
	mach_port_t remote;
	kern_return_t kr;
	uint64_t id;

	debugf("connection=%p", context);

	conn = context;
	kr = xpc_pipe_receive(conn->xc_local_port, &remote, &result, &id);
	if (kr != KERN_SUCCESS)
		return;

	debugf("message=%p, id=%lu, remote=<%d>", result, id, remote);

	if (conn->xc_flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		TAILQ_FOREACH(peer, &conn->xc_peers, xc_link) {
			if (remote == peer->xc_remote_port) {
				dispatch_async(peer->xc_target_queue, ^{
					peer->xc_handler(result);
				});
				return;
			}
		}

		debugf("new peer on port <%u>", remote);

		/* New peer */
		xpc_object_t xpeer = xpc_connection_create(NULL, NULL);
		peer = conn_extract(xpeer);
		peer->xc_parent = conn;
		peer->xc_remote_port = remote;
		xpc_connection_set_credentials(peer,
		    ((struct xpc_object *)result)->xo_audit_token);

		TAILQ_INSERT_TAIL(&conn->xc_peers, peer, xc_link);

		dispatch_async(conn->xc_target_queue, ^{
			conn->xc_handler(xpeer);
		});

		dispatch_async(peer->xc_target_queue, ^{
			peer->xc_handler(result);
		});

	} else {
		xpc_connection_set_credentials(conn,
		    ((struct xpc_object *)result)->xo_audit_token);

		TAILQ_FOREACH(call, &conn->xc_pending, xp_link) {
			if (call->xp_id == id) {
				dispatch_async(conn->xc_target_queue, ^{
					call->xp_handler(result);
					TAILQ_REMOVE(&conn->xc_pending, call,
					    xp_link);
					free(call);
				});
				return;
			}
		}

		if (conn->xc_handler) {
			dispatch_async(conn->xc_target_queue, ^{
			    conn->xc_handler(result);
			});
		}
	}
}
