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

#ifndef	_LIBXPC_XPC_INTERNAL_H
#define	_LIBXPC_XPC_INTERNAL_H

#include <sys/queue.h>
#include <sys/uio.h>
#include <stdatomic.h>
#include <dispatch/dispatch.h>
#include "mpack.h"

#ifdef XPC_DEBUG
#define debugf(...) 				\
    do { 					\
    	fprintf(stderr, "%s: ", __func__);	\
    	fprintf(stderr, __VA_ARGS__);		\
    	fprintf(stderr, "\n");			\
    } while(0);
#else
#define debugf(...)
#endif

#define _XPC_TYPE_INVALID		0
#define _XPC_TYPE_DICTIONARY		1
#define _XPC_TYPE_ARRAY			2
#define _XPC_TYPE_BOOL			3
#define _XPC_TYPE_CONNECTION		4
#define _XPC_TYPE_ENDPOINT		5
#define	_XPC_TYPE_NULL			6
#define _XPC_TYPE_INT64			8
#define _XPC_TYPE_UINT64		9
#define _XPC_TYPE_DATE			10
#define _XPC_TYPE_DATA			11
#define _XPC_TYPE_STRING		12
#define _XPC_TYPE_UUID			13
#define _XPC_TYPE_FD			14
#define _XPC_TYPE_SHMEM			15
#define _XPC_TYPE_ERROR			16
#define _XPC_TYPE_DOUBLE		17
#define _XPC_TYPE_MAX			_XPC_TYPE_DOUBLE

#define	XPC_SEQID		"XPC sequence number"
#define	XPC_PROTOCOL_VERSION	1

struct xpc_object;
struct xpc_dict_pair;
struct xpc_resource;
struct xpc_credentials;

TAILQ_HEAD(xpc_dict_head, xpc_dict_pair);
TAILQ_HEAD(xpc_array_head, xpc_object);

typedef void *xpc_port_t;
typedef void (*xpc_transport_init_t)();
typedef int (*xpc_transport_listen_t)(const char *, xpc_port_t *);
typedef int (*xpc_transport_lookup)(const char *, xpc_port_t *, xpc_port_t *);
typedef char *(*xpc_transport_port_to_string)(xpc_port_t);
typedef int (*xpc_transport_port_compare)(xpc_port_t, xpc_port_t);
typedef int (*xpc_transport_release)(xpc_port_t);
typedef int (*xpc_transport_send)(xpc_port_t, xpc_port_t, void *buf,
    size_t len, struct xpc_resource *, size_t);
typedef int(*xpc_transport_recv)(xpc_port_t, xpc_port_t*, void *buf,
    size_t len, struct xpc_resource **, size_t *, struct xpc_credentials *);
typedef dispatch_source_t (*xpc_transport_create_source)(xpc_port_t,
    void *, dispatch_queue_t);

typedef union {
	struct xpc_dict_head dict;
	struct xpc_array_head array;
	uint64_t ui;
	int64_t i;
	char *str;
	bool b;
	double d;
	uintptr_t ptr;
	int fd;
	uuid_t uuid;
#ifdef __MACH__
	mach_port_t port;
#endif
} xpc_u;

struct xpc_frame_header {
    uint64_t version;
    uint64_t id;
    uint64_t length;
    uint64_t spare[4];
};

#define _XPC_FROM_WIRE 0x1
struct xpc_object {
	uint8_t			xo_xpc_type;
	uint16_t		xo_flags;
	volatile _Atomic(uint32_t)	xo_refcnt;
	size_t			xo_size;
	xpc_u			xo_u;
#ifdef __MACH__
	audit_token_t *		xo_audit_token;
#endif
	TAILQ_ENTRY(xpc_object) xo_link;
};

struct xpc_dict_pair {
	const char *		key;
	struct xpc_object *	value;
	TAILQ_ENTRY(xpc_dict_pair) xo_link;
};

struct xpc_pending_call {
	uint64_t		xp_id;
	xpc_object_t		xp_response;
	dispatch_queue_t	xp_queue;
	xpc_handler_t		xp_handler;
	TAILQ_ENTRY(xpc_pending_call) xp_link;
};

struct xpc_credentials {
    uid_t			xc_remote_euid;
    gid_t			xc_remote_guid;
    pid_t			xc_remote_pid;
#ifdef __MACH__
    au_asid_t			xc_remote_asid;
    audit_token_t		xc_audit_token;
#endif
};

struct xpc_connection {
	const char *		xc_name;
	xpc_port_t		xc_local_port;
    xpc_port_t		xc_remote_port;
	xpc_handler_t		xc_handler;
	dispatch_source_t	xc_recv_source;
	dispatch_queue_t	xc_send_queue;
	dispatch_queue_t	xc_recv_queue;
	dispatch_queue_t	xc_target_queue;
	int			xc_suspend_count;
	int			xc_transaction_count;
	uint64_t		xc_flags;
	volatile _Atomic(uint64_t)	xc_last_id;
	void *			xc_context;
	struct xpc_connection * xc_parent;
    	struct xpc_credentials	xc_creds;
	TAILQ_HEAD(, xpc_pending_call) xc_pending;
	TAILQ_HEAD(, xpc_connection) xc_peers;
	TAILQ_ENTRY(xpc_connection) xc_link;
};

struct xpc_resource {
    	int			xr_type;
#define XPC_RESOURCE_FD		0x01
#define XPC_RESOURCE_SHMEM	0x02
    	union {
	    int 		xr_fd;
	};
};

struct xpc_transport {
    	const char *		xt_name;
    	pthread_once_t		xt_initialized;
    	xpc_transport_init_t 	xt_init;
    	xpc_transport_listen_t 	xt_listen;
    	xpc_transport_lookup 	xt_lookup;
    	xpc_transport_port_to_string xt_port_to_string;
    	xpc_transport_port_compare xt_port_compare;
    	xpc_transport_release 	xt_release;
    	xpc_transport_send 	xt_send;
    	xpc_transport_recv	xt_recv;
    	xpc_transport_create_source xt_create_server_source;
    	xpc_transport_create_source xt_create_client_source;
};

struct xpc_service {
	xpc_port_t		xs_pipe;
	TAILQ_HEAD(, xpc_connection) xs_connections;
};

#define xo_str xo_u.str
#define xo_bool xo_u.b
#define xo_uint xo_u.ui
#define xo_int xo_u.i
#define xo_ptr xo_u.ptr
#define xo_d xo_u.d
#define xo_fd xo_u.fd
#define xo_uuid xo_u.uuid
#define xo_port xo_u.port
#define xo_array xo_u.array
#define xo_dict xo_u.dict

__private_extern__ struct xpc_transport *xpc_get_transport();
__private_extern__ void xpc_set_transport(struct xpc_transport *);
__private_extern__ struct xpc_object *_xpc_prim_create(int type, xpc_u value,
    size_t size);
__private_extern__ struct xpc_object *_xpc_prim_create_flags(int type,
    xpc_u value, size_t size, uint16_t flags);
__private_extern__ const char *_xpc_get_type_name(xpc_object_t obj);
__private_extern__ struct xpc_object *mpack2xpc(mpack_node_t node);
__private_extern__ void xpc2mpack(mpack_writer_t *writer, xpc_object_t xo);
__private_extern__ void xpc_object_destroy(struct xpc_object *xo);
__private_extern__ void xpc_connection_recv_message(void *);
__private_extern__ void xpc_connection_recv_mach_message(void *);
__private_extern__ void *xpc_connection_new_peer(void *context,
    xpc_port_t local, xpc_port_t remote, dispatch_source_t src);
__private_extern__ void xpc_connection_destroy_peer(void *context);
__private_extern__ int xpc_pipe_send(xpc_object_t obj, uint64_t id,
    xpc_port_t local, xpc_port_t remote);
__private_extern__ int xpc_pipe_receive(xpc_port_t local, xpc_port_t *remote,
    xpc_object_t *result, uint64_t *id, struct xpc_credentials *creds);

#endif	/* _LIBXPC_XPC_INTERNAL_H */
