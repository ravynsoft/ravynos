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

#include <nv.h>

#define debugf(...) 				\
    do { 					\
    	fprintf(stderr, "%s: ", __func__);	\
    	fprintf(stderr, __VA_ARGS__);		\
    	fprintf(stderr, "\n");			\
    } while(0);

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

#define	XPC_SEQID	"XPC sequence number"

struct xpc_object;
struct xpc_dict_pair;

TAILQ_HEAD(xpc_dict_head, xpc_dict_pair);
TAILQ_HEAD(xpc_array_head, xpc_object);

struct xpc_connection {
	const char *		xc_name;
	mach_port_t		xc_remote_port;
	mach_port_t		xc_local_port;
	xpc_handler_t		xc_handler;
	dispatch_source_t	xc_recv_source;
	dispatch_queue_t	xc_send_queue;
	dispatch_queue_t	xc_recv_queue;
	dispatch_queue_t	xc_target_queue;
	int			xc_suspend_count;
	int			xc_transaction_count;
	int 			xc_flags;
	volatile uint64_t	xc_last_id;
	void *			xc_context;
	struct xpc_connection * xc_parent;
	uid_t			xc_remote_euid;
	gid_t			xc_remote_guid;
	pid_t			xc_remote_pid;
	au_asid_t		xc_remote_asid;
	TAILQ_HEAD(, xpc_pending_call) xc_pending;
	TAILQ_HEAD(, xpc_connection) xc_peers;
	TAILQ_ENTRY(xpc_connection) xc_link;
};

typedef union {
	struct xpc_dict_head dict;
	struct xpc_array_head array;
	uint64_t ui;
	int64_t i;
	const char *str;
	bool b;
	double d;
	uintptr_t ptr;
	int fd;
	uuid_t uuid;
	mach_port_t port;
	struct xpc_connection connection;
} xpc_u;	


#define _XPC_FROM_WIRE 0x1
struct xpc_object {
	uint8_t			xo_xpc_type;
	uint16_t		xo_flags;
	volatile uint32_t	xo_refcnt;
	size_t			xo_size;
	xpc_u			xo_u;
	audit_token_t *		xo_audit_token;
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


struct xpc_service {
	mach_port_t		xs_remote_port;
	TAILQ_HEAD(, xpc_connection) xs_connections;
};

#define xo_nv xo_u.nv
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
#define xo_connection xo_u.connection

__private_extern__ struct xpc_object *_xpc_prim_create(int type, xpc_u value,
    size_t size);
__private_extern__ struct xpc_object *_xpc_prim_create_flags(int type,
    xpc_u value, size_t size, uint16_t flags);
__private_extern__ const char *_xpc_get_type_name(xpc_object_t obj);
__private_extern__ struct xpc_object *nv2xpc(const nvlist_t *nv);
__private_extern__ nvlist_t *xpc2nv(struct xpc_object *xo);
__private_extern__ void xpc_object_destroy(struct xpc_object *xo);
__private_extern__ int xpc_pipe_send(xpc_object_t obj, mach_port_t dst,
    mach_port_t local, uint64_t id);
__private_extern__ int xpc_pipe_receive(mach_port_t local, mach_port_t *remote,
    xpc_object_t *result, uint64_t *id);

void fail_log(const char *) __dead2;

#endif	/* _LIBXPC_XPC_INTERNAL_H */
