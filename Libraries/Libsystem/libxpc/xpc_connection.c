/*
 * Original code Copyright 2014-2015 iXsystems, Inc.
 * Changes for ravynOS Copyright (C) 2026 Zoe Knox
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

#include <string.h>
#include <sys/bsm/audit.h>
#include <bsm/libbsm.h>
#include <errno.h>
#include <xpc/xpc.h>
#include <stdatomic.h>
#include <Block.h>
#include "xpc_internal.h"

#define XPC_CONNECTION_NEXT_ID(conn) (atomic_fetch_add(&conn->xc_last_id, 1))

static void xpc_send(xpc_connection_t xconn, xpc_object_t message, uint64_t id);
char *strerror(int);

xpc_connection_t
xpc_connection_create(const char *name, dispatch_queue_t targetq)
{
	char *qname;
	struct xpc_transport *transport = xpc_get_transport();
	struct xpc_connection *conn;

	if ((conn = malloc(sizeof(struct xpc_connection))) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}

	memset(conn, 0, sizeof(struct xpc_connection));
	conn->xc_last_id = 1;
	TAILQ_INIT(&conn->xc_peers);
	TAILQ_INIT(&conn->xc_pending);

	/* Create send queue */
	asprintf(&qname, "com.ravynos.xpc.connection.sendq.%p", conn);
	conn->xc_send_queue = dispatch_queue_create(qname, NULL);

	/* Create recv queue */
	asprintf(&qname, "com.ravynos.xpc.connection.recvq.%p", conn);
	conn->xc_recv_queue = dispatch_queue_create(qname, NULL);

	/* Create target queue */
	conn->xc_target_queue = targetq ? targetq : dispatch_get_main_queue();

	/* Receive queue is initially suspended */
	dispatch_suspend(conn->xc_recv_queue);

	return ((xpc_connection_t)conn);
}

xpc_connection_t
xpc_connection_create_mach_service(const char *name, dispatch_queue_t targetq,
    uint64_t flags)
{
	struct xpc_transport *transport = xpc_get_transport();
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xpc_connection_create(name, targetq);
	if (conn == NULL)
		return (NULL);

	conn->xc_flags = flags;

	if (flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		if (transport->xt_listen(name, &conn->xc_local_port) != 0) {
			debugf("Cannot create local port: %s", strerror(errno));
			return (NULL);
		}

		return ((xpc_connection_t)conn);
	}

	if (transport->xt_lookup(name, &conn->xc_local_port, &conn->xc_remote_port) != 0) {
		return (NULL);
	}

	return ((xpc_connection_t)conn);
}

xpc_connection_t
xpc_connection_create_from_endpoint(xpc_endpoint_t endpoint)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xpc_connection_create("anonymous", NULL);
	if (conn == NULL)
		return (NULL);

	conn->xc_remote_port = (xpc_port_t)endpoint;
	return ((xpc_connection_t)conn);
}

void
xpc_connection_set_target_queue(xpc_connection_t xconn,
    dispatch_queue_t targetq)
{
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = (struct xpc_connection *)xconn;
	conn->xc_target_queue = targetq;	
}

void
xpc_connection_set_event_handler(xpc_connection_t xconn,
    xpc_handler_t handler)
{
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = (struct xpc_connection *)xconn;
	conn->xc_handler = (xpc_handler_t)Block_copy(handler);
}

void
xpc_connection_suspend(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xconn;
	dispatch_suspend(conn->xc_recv_source);
}

void
xpc_connection_resume(xpc_connection_t xconn)
{
	struct xpc_transport *transport = xpc_get_transport();
	struct xpc_connection *conn;

	debugf("connection=%p", xconn);
	conn = (struct xpc_connection *)xconn;

	/* Create dispatch source for top-level connection */
	if (conn->xc_flags & XPC_CONNECTION_MACH_SERVICE_LISTENER) {
		conn->xc_recv_source = transport->xt_create_server_source(
		    conn->xc_local_port, conn, conn->xc_recv_queue);
		dispatch_resume(conn->xc_recv_source);
	} else {
		if (conn->xc_parent == NULL) {
			conn->xc_recv_source = transport->xt_create_client_source(
			    conn->xc_local_port, conn, conn->xc_recv_queue);
			dispatch_resume(conn->xc_recv_source);
		}
	}

	dispatch_resume(conn->xc_recv_queue);
}

void
xpc_connection_send_message(xpc_connection_t xconn,
    xpc_object_t message)
{
	struct xpc_connection *conn;
	uint64_t id;

	conn = (struct xpc_connection *)xconn;
	id = xpc_dictionary_get_uint64(message, XPC_SEQID);

	if (id == 0)
		id = XPC_CONNECTION_NEXT_ID(conn);

	dispatch_async(conn->xc_send_queue, ^{
		xpc_send(xconn, message, id);
	});
}

void
xpc_connection_send_message_with_reply(xpc_connection_t xconn,
    xpc_object_t message, dispatch_queue_t targetq, xpc_handler_t handler)
{
	struct xpc_connection *conn;
	struct xpc_pending_call *call;

	conn = (struct xpc_connection *)xconn;
	call = malloc(sizeof(struct xpc_pending_call));
	call->xp_id = XPC_CONNECTION_NEXT_ID(conn);
	call->xp_handler = handler;
	call->xp_queue = targetq;
	TAILQ_INSERT_TAIL(&conn->xc_pending, call, xp_link);

	dispatch_async(conn->xc_send_queue, ^{
		xpc_send(xconn, message, call->xp_id);
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

	conn = (struct xpc_connection *)xconn;
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

	conn = (struct xpc_connection *)xconn;
	return (conn->xc_creds.xc_remote_euid);
}

gid_t
xpc_connection_get_egid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xconn;
	return (conn->xc_creds.xc_remote_guid);
}

pid_t
xpc_connection_get_pid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xconn;
	return (conn->xc_creds.xc_remote_pid);
}

au_asid_t
xpc_connection_get_asid(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = xconn;
	return (conn->xc_creds.xc_remote_asid);
}

void
xpc_connection_set_context(xpc_connection_t xconn, void *ctx)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xconn;
	conn->xc_context = ctx;
}

void *
xpc_connection_get_context(xpc_connection_t xconn)
{
	struct xpc_connection *conn;

	conn = (struct xpc_connection *)xconn;
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
	return (NULL);
}

void
xpc_main(xpc_connection_handler_t handler)
{

	dispatch_main();
}

void
xpc_transaction_begin(void)
{

}

void
xpc_transaction_end(void)
{

}

static void
xpc_send(xpc_connection_t xconn, xpc_object_t message, uint64_t id)
{
	struct xpc_connection *conn;
	int err;

	debugf("connection=%p, message=%p, id=%lu", xconn, message, id);

	conn = (struct xpc_connection *)xconn;
	if (xpc_pipe_send(message, id, conn->xc_local_port,
	    conn->xc_remote_port) != 0)
		debugf("send failed: %s", strerror(errno));
}

#ifdef MACH
static void
xpc_connection_set_credentials(struct xpc_connection *conn, audit_token_t *tok)
{
	uid_t uid;
	gid_t gid;
	pid_t pid;
	au_asid_t asid;

	if (tok == NULL)
		return;

	audit_token_to_au32(*tok, NULL, &uid, &gid, NULL, NULL, &pid, &asid,
	    NULL);

	conn->xc_creds.xc_remote_euid = uid;
	conn->xc_creds.xc_remote_guid = gid;
	conn->xc_creds.xc_remote_pid = pid;
	conn->xc_creds.xc_remote_asid = asid;
}
#endif

struct xpc_connection *
xpc_connection_get_peer(void *context, xpc_port_t port)
{
	struct xpc_transport *transport = xpc_get_transport();
	struct xpc_connection *conn, *peer;

	conn = context;
	TAILQ_FOREACH(peer, &conn->xc_peers, xc_link) {
		if (transport->xt_port_compare(port,
		    peer->xc_remote_port)) {
			return (peer);
		}
	}

	return (NULL);
}

void *
xpc_connection_new_peer(void *context, xpc_port_t local, xpc_port_t remote, dispatch_source_t src)
{
	struct xpc_transport *transport = xpc_get_transport();
	struct xpc_connection *conn, *peer;

	conn = context;
	peer = (struct xpc_connection *)xpc_connection_create(NULL, NULL);
	peer->xc_parent = conn;
	peer->xc_local_port = local;
	peer->xc_remote_port = remote;
	peer->xc_recv_source = src;

	TAILQ_INSERT_TAIL(&conn->xc_peers, peer, xc_link);

	if (src) {
		dispatch_set_context(src, peer);
		dispatch_resume(src);
		dispatch_async(conn->xc_target_queue, ^{
		    conn->xc_handler(peer);
		});
	}

	return (peer);
}

void
xpc_connection_destroy_peer(void *context)
{
	struct xpc_connection *conn, *parent;

	conn = context;
	parent = conn->xc_parent;

	if (conn->xc_parent != NULL) {
		dispatch_async(parent->xc_target_queue, ^{
		    conn->xc_handler((xpc_object_t)XPC_ERROR_CONNECTION_INVALID);
		});

		TAILQ_REMOVE(&parent->xc_peers, conn, xc_link);
	}

	dispatch_release(conn->xc_recv_source);
}

static void
xpc_connection_dispatch_callback(struct xpc_connection *conn,
    xpc_object_t result, uint64_t id)
{
	struct xpc_pending_call *call;

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
		debugf("yes");
		dispatch_async(conn->xc_target_queue, ^{
		    debugf("calling handler=%p", conn->xc_handler);
		    conn->xc_handler(result);
		});
	}
}

void
xpc_connection_recv_message(void *context)
{
	struct xpc_pending_call *call;
	struct xpc_connection *conn;
	struct xpc_credentials creds;
	xpc_object_t result;
	xpc_port_t remote;
	uint64_t id;
	int err;

	debugf("connection=%p", context);

	conn = context;
	err = xpc_pipe_receive(conn->xc_local_port, &remote, &result, &id,
	    &creds);

	if (err < 0)
		return;

	if (err == 0) {
		dispatch_source_cancel(conn->xc_recv_source);
		return;
	}

	debugf("msg=%p, id=%lu", result, id);

	conn->xc_creds = creds;

	xpc_connection_dispatch_callback(conn, result, id);
}

void
xpc_connection_recv_mach_message(void *context)
{
	struct xpc_transport *transport = xpc_get_transport();

	struct xpc_connection *conn, *peer;
	struct xpc_credentials creds;
	xpc_object_t result;
	xpc_port_t remote;
	uint64_t id;
	int err;

	debugf("connection=%p", context);

	conn = context;
	if (xpc_pipe_receive(conn->xc_local_port, &remote, &result, &id,
	    &creds) < 0)
		return;

	debugf("message=%p, id=%lu, remote=%s", result, id,
	    transport->xt_port_to_string(remote));

	peer = xpc_connection_get_peer(context, remote);
	if (!peer) {
		debugf("new peer on port %s",
		    transport->xt_port_to_string(remote));
		peer = xpc_connection_new_peer(context, conn->xc_local_port, remote, NULL);

		dispatch_async(conn->xc_target_queue, ^{
		    conn->xc_handler(peer);
		    xpc_connection_dispatch_callback(peer, result, id);
		});
	} else
		xpc_connection_dispatch_callback(peer, result, id);
}