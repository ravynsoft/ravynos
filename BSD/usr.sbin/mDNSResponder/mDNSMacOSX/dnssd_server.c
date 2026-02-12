/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dnssd_server.h"

#include "ClientRequests.h"
#include "dnssd_xpc.h"
#include "mDNSMacOSX.h"

#include <CoreUtils/CommonServices.h>
#include <CoreUtils/DebugServices.h>
#include <libproc.h>
#include <net/necp.h>
#include <sys/proc_info.h>
#include <xpc/private.h>

#if 0
//======================================================================================================================
#pragma mark - Kind Declarations
#endif

#define DX_STRUCT(NAME)			struct dx_ ## NAME ## _s
#define DX_TYPE(NAME)			dx_ ## NAME ## _t
#define DX_KIND_DECLARE(NAME)	typedef DX_STRUCT(NAME) *	DX_TYPE(NAME)

#define DX_KIND_DECLARE_FULL(NAME)						\
	DX_KIND_DECLARE(NAME);								\
														\
	static void											\
	_dx_ ## NAME ## _invalidate(DX_TYPE(NAME) object);	\
														\
	static void											\
	_dx_ ## NAME ## _finalize(DX_TYPE(NAME) object);	\
														\
	static DX_TYPE(NAME)								\
	_dx_ ## NAME ## _alloc(void)

// Note: The last check checks if the base's type is equal to that of the superkind. If it's not, then the pointer
// comparison used as the argument to sizeof will cause a "comparison of distinct pointer types" warning, so long as
// the warning hasn't been disabled.

#define DX_BASE_CHECK(NAME, SUPER)																\
	check_compile_time(offsetof(DX_STRUCT(NAME), base) == 0);									\
	check_compile_time(sizeof_field(DX_STRUCT(NAME), base) == sizeof(DX_STRUCT(SUPER)));		\
	extern int _dx_base_type_check[sizeof(&(((DX_TYPE(NAME))0)->base) == ((DX_TYPE(SUPER))0))]

#define DX_KIND_DEFINE(NAME, SUPER)											\
	static const struct dx_kind_s _dx_ ## NAME ## _kind = {					\
		&_dx_ ## SUPER ##_kind,												\
		_dx_ ## NAME ## _invalidate,										\
		_dx_ ## NAME ## _finalize,											\
	};																		\
																			\
	static DX_TYPE(NAME)													\
	_dx_ ## NAME ## _alloc(void)											\
	{																		\
		const DX_TYPE(NAME) obj = (DX_TYPE(NAME))calloc(1, sizeof(*obj));	\
		require_quiet(obj, exit);											\
																			\
		const dx_base_t base = (dx_base_t)obj;								\
		base->ref_count	= 1;												\
		base->kind		= &_dx_ ## NAME ## _kind;							\
																			\
	exit:																	\
		return obj;															\
	}																		\
	DX_BASE_CHECK(NAME, SUPER)

DX_KIND_DECLARE(base);
DX_KIND_DECLARE(request);
DX_KIND_DECLARE_FULL(session);
DX_KIND_DECLARE_FULL(getaddrinfo_request);

typedef union {
	dx_base_t					base;
	dx_session_t				session;
	dx_request_t				request;
	dx_getaddrinfo_request_t	getaddrinfo_request;
} dx_any_t __attribute__((__transparent_union__));

typedef void (*dx_invalidate_f)(dx_any_t object);
typedef void (*dx_finalize_f)(dx_any_t object);

typedef const struct dx_kind_s *	dx_kind_t;
struct dx_kind_s {
	dx_kind_t		superkind;	// This kind's superkind. All kinds have a superkind, except the base kind.
	dx_invalidate_f	invalidate;	// Stops an object's outstanding operations, if any.
	dx_finalize_f	finalize;	// Releases object's resources right before the object is freed.
};

#if 0
//======================================================================================================================
#pragma mark - Base Kind Definition
#endif

struct dx_base_s {
	dx_kind_t	kind;		// The object's kind.
	int32_t		ref_count;	// Reference count.
};

static const struct dx_kind_s _dx_base_kind = {
	NULL,	// No superkind.
	NULL,	// No invalidate method.
	NULL,	// No finalize method.
};

#if 0
//======================================================================================================================
#pragma mark - Request Kind Definition
#endif

struct dx_request_s {
	struct dx_base_s	base;			// Object base.
	dx_request_t		next;			// Next request in list.
	xpc_object_t		result_array;	// Array of pending results.
	uint64_t			command_id;		// ID to distinquish multiple commands during a session.
	uint32_t			request_id;		// Request ID, used for logging purposes.
	DNSServiceErrorType	error;			// Pending error.
	bool				sent_error;		// True if the pending error has been sent to client.
};

static void
_dx_request_finalize(dx_any_t request);

static const struct dx_kind_s _dx_request_kind = {
	&_dx_base_kind,
	NULL,					// No invalidate method.
	_dx_request_finalize,
};
DX_BASE_CHECK(request, base);

#if 0
//======================================================================================================================
#pragma mark - Session Kind Definition
#endif

struct dx_session_s {
	struct dx_base_s	base;						// Object base;
	dx_session_t		next;						// Next session in list.
	dx_request_t		request_list;				// List of outstanding requests.
	xpc_connection_t	connection;					// Underlying XPC connection.
	bool				has_delegate_entitlement;	// True if the client is entitled to be a delegate.
};

DX_KIND_DEFINE(session, base);

#if 0
//======================================================================================================================
#pragma mark - GetAddrInfo Request Kind Definition
#endif

struct dx_getaddrinfo_request_s {
	struct dx_request_s			base;			// Request object base.
	GetAddrInfoClientRequest	gai;			// Underlying GetAddrInfoClientRequest.
	xpc_object_t				hostname;		// Hostname to be resolved for getaddrinfo requests.
	uuid_t						client_uuid;	// Client's UUID for authenticating results.
	bool						need_auth;		// True if results need to be authenticated.
	bool						active;			// True if the GetAddrInfoClientRequest is currently active.
};

DX_KIND_DEFINE(getaddrinfo_request, request);

#if 0
//======================================================================================================================
#pragma mark - Local Prototypes
#endif

static dispatch_queue_t
_dx_server_queue(void);

static void
_dx_server_register_session(dx_session_t session);

static void
_dx_server_deregister_session(dx_session_t session);

static void
_dx_retain(dx_any_t object);

static void
_dx_release(dx_any_t object);
#define _dx_release_null_safe(X)	do { if (X) { _dx_release(X); } } while (0)

static void
_dx_invalidate(dx_any_t object);

static dx_session_t
_dx_session_create(xpc_connection_t connection);

static void
_dx_session_activate(dx_session_t me);

static DNSServiceErrorType
_dx_session_handle_getaddrinfo_command(dx_session_t session, xpc_object_t msg);

static DNSServiceErrorType
_dx_session_handle_stop_command(dx_session_t session, xpc_object_t msg);

static void
_dx_session_send_results(dx_session_t session);

static dx_getaddrinfo_request_t
_dx_getaddrinfo_request_create(uint64_t command_id, uint32_t request_id);

static DNSServiceErrorType
_dx_getaddrinfo_request_set_hostname(dx_getaddrinfo_request_t request, xpc_object_t hostname);

static void
_dx_getaddrinfo_request_set_need_authenticaed_results(dx_getaddrinfo_request_t request, bool need,
	const uuid_t client_uuid);

static DNSServiceErrorType
_dx_getaddrinfo_request_activate(dx_getaddrinfo_request_t request, uint32_t interface_index, DNSServiceFlags flags,
	DNSServiceProtocol protocols, pid_t pid, const uuid_t uuid, uid_t uid);

static void
_dx_getaddrinfo_request_result_handler(mDNS *m, DNSQuestion *question, const ResourceRecord *answer,
	QC_result qc_result, DNSServiceErrorType error, void *context);

#if 0
//======================================================================================================================
#pragma mark - Server Functions
#endif

static void
_dx_server_handle_new_connection(xpc_connection_t connection);

mDNSexport void
dnssd_server_init(void)
{
	static xpc_connection_t	listener = NULL;

	listener = xpc_connection_create_mach_service(DNSSD_MACH_SERVICE_NAME, _dx_server_queue(),
		XPC_CONNECTION_MACH_SERVICE_LISTENER);
    xpc_connection_set_event_handler(listener,
	^(xpc_object_t event)
	{
		if (xpc_get_type(event) == XPC_TYPE_CONNECTION) {
			_dx_server_handle_new_connection((xpc_connection_t)event);
		}
	});
	xpc_connection_activate(listener);
}

static void
_dx_server_handle_new_connection(xpc_connection_t connection)
{
	const dx_session_t session = _dx_session_create(connection);
	if (session) {
		_dx_session_activate(session);
		_dx_server_register_session(session);
		_dx_release(session);
	} else {
		xpc_connection_cancel(connection);
	}
}


//======================================================================================================================

static dx_session_t	g_session_list	= NULL;

mDNSexport void
dnssd_server_idle(void)
{
	for (dx_session_t session = g_session_list; session; session = session->next) {
		_dx_session_send_results(session);
	}
}

//======================================================================================================================

static dispatch_queue_t
_dx_server_queue(void)
{
	static dispatch_once_t	once	= 0;
	static dispatch_queue_t	queue	= NULL;

	dispatch_once(&once,
	^{
		const dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
			QOS_CLASS_UTILITY, 0);
		queue = dispatch_queue_create("com.apple.dnssd.server", attr);
	});
	return queue;
}

//======================================================================================================================

static void
_dx_server_register_session(dx_session_t session)
{
	session->next	= g_session_list;
	g_session_list	= session;
	_dx_retain(session);
}

//======================================================================================================================

static void
_dx_server_deregister_session(dx_session_t session)
{
	dx_session_t *ptr;
	for (ptr = &g_session_list; *ptr; ptr = &(*ptr)->next) {
		if (*ptr == session) {
			break;
		}
	}
	if (*ptr) {
		*ptr			= session->next;
		session->next	= NULL;
		_dx_release(session);
	}
}

#if 0
//======================================================================================================================
#pragma mark - Base Methods
#endif

static void
_dx_retain(dx_any_t object)
{
	++object.base->ref_count;
}

//======================================================================================================================

static void
_dx_release(dx_any_t object)
{
	if (--object.base->ref_count == 0) {
		for (dx_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
			if (kind->finalize) {
				kind->finalize(object);
			}
		}
		free(object.base);
	}
}

//======================================================================================================================

static void
_dx_invalidate(dx_any_t object)
{
	for (dx_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
		if (kind->invalidate) {
			kind->invalidate(object);
			return;
		}
	}
}

#if 0
//======================================================================================================================
#pragma mark - Session Methods
#endif

static dx_session_t
_dx_session_create(xpc_connection_t connection)
{
	const dx_session_t obj = _dx_session_alloc();
	require_quiet(obj, exit);

	obj->connection = connection;
	xpc_retain(obj->connection);

exit:
	return obj;
}

//======================================================================================================================

static void
_dx_session_handle_message(dx_session_t session, xpc_object_t msg);

#define DNSSD_DELEGATE_ENTITLEMENT "com.apple.private.network.socket-delegate"

static void
_dx_session_activate(dx_session_t me)
{
	const xpc_object_t value = xpc_connection_copy_entitlement_value(me->connection, DNSSD_DELEGATE_ENTITLEMENT);
	if (value) {
		if (value == XPC_BOOL_TRUE) {
			me->has_delegate_entitlement = true;
		}
		xpc_release(value);
	}
	_dx_retain(me);
	xpc_connection_set_target_queue(me->connection, _dx_server_queue());
	xpc_connection_set_event_handler(me->connection,
	^(xpc_object_t event) {
		const xpc_type_t type = xpc_get_type(event);
		if (type == XPC_TYPE_DICTIONARY) {
    		KQueueLock();
			_dx_session_handle_message(me, event);
    		KQueueUnlock("_dx_session_handle_message");
		} else if (event == XPC_ERROR_CONNECTION_INVALID) {
    		KQueueLock();
			_dx_server_deregister_session(me);
    		KQueueUnlock("_dx_server_deregister_session");
			_dx_session_invalidate(me);
			_dx_release(me);
		} else {
			xpc_connection_cancel(me->connection);
		}
	});
	xpc_connection_activate(me->connection);
}

static void
_dx_session_handle_message(dx_session_t me, xpc_object_t msg)
{
	DNSServiceErrorType error;
	const char * const command = dnssd_xpc_message_get_command(msg);
	require_action_quiet(command, exit, error = kDNSServiceErr_BadParam);

	if (strcmp(command, DNSSD_COMMAND_GETADDRINFO) == 0) {
		error = _dx_session_handle_getaddrinfo_command(me, msg);
	} else if (strcmp(command, DNSSD_COMMAND_STOP) == 0) {
		error = _dx_session_handle_stop_command(me, msg);
	} else {
		error = kDNSServiceErr_BadParam;
	}

exit:
	{
		const xpc_object_t reply = xpc_dictionary_create_reply(msg);
		if (likely(reply)) {
			dnssd_xpc_message_set_error(reply, error);
			xpc_connection_send_message(me->connection, reply);
			xpc_release(reply);
		} else {
			xpc_connection_cancel(me->connection);
		}
	}
}

//======================================================================================================================

static void
_dx_session_invalidate(dx_session_t me)
{
	xpc_connection_forget(&me->connection);
	dx_request_t req;
	while ((req = me->request_list) != NULL)
	{
		me->request_list = req->next;
		_dx_invalidate(req);
		_dx_release(req);
	}
}

//======================================================================================================================

static void
_dx_session_finalize(dx_session_t me)
{
	(void)me;
}

//======================================================================================================================

static bool
_dx_get_getaddrinfo_params(xpc_object_t msg, uint64_t *out_command_id, xpc_object_t *out_hostname,
	uint32_t *out_interface_index, DNSServiceFlags *out_flags, DNSServiceProtocol *out_protocols,
	pid_t *out_delegate_pid, const uint8_t **out_delegate_uuid, bool *out_need_auth_tags);

extern mDNS	mDNSStorage;
#define g_mdns	mDNSStorage

static DNSServiceErrorType
_dx_session_handle_getaddrinfo_command(dx_session_t me, xpc_object_t msg)
{
	dx_getaddrinfo_request_t	req = NULL;
	DNSServiceErrorType			error;
	uint64_t					command_id;
	xpc_object_t				hostname;
	uint32_t					interface_index;
	DNSServiceFlags				flags;
	DNSServiceProtocol			protocols;
	pid_t						pid;
	const uint8_t *				uuid;
	bool						need_auth;

	const bool valid = _dx_get_getaddrinfo_params(msg, &command_id, &hostname, &interface_index, &flags, &protocols,
		&pid, &uuid, &need_auth);
	require_action_quiet(valid, exit, error = kDNSServiceErr_BadParam);

	if (uuid || (pid != 0)) {
		require_action_quiet(me->has_delegate_entitlement, exit, error = kDNSServiceErr_NoAuth);
	} else {
		pid = xpc_connection_get_pid(me->connection);
	}

	req = _dx_getaddrinfo_request_create(command_id, g_mdns.next_request_id++);
	require_action_quiet(req, exit, error = kDNSServiceErr_NoMemory);

	error = _dx_getaddrinfo_request_set_hostname(req, hostname);
	require_noerr_quiet(error, exit);

	if (need_auth) {
		struct proc_uniqidentifierinfo info;
		const int n = proc_pidinfo(pid, PROC_PIDUNIQIDENTIFIERINFO, 1, &info, sizeof(info));
		if (n == (int)sizeof(info)) {
			_dx_getaddrinfo_request_set_need_authenticaed_results(req, true, info.p_uuid);
		}
	}

	const uid_t euid = xpc_connection_get_euid(me->connection);
	error = _dx_getaddrinfo_request_activate(req, interface_index, flags, protocols, pid, uuid, euid);
	require_noerr_quiet(error, exit);

	req->base.next		= me->request_list;
	me->request_list	= (dx_request_t)req;
	req = NULL;

exit:
	_dx_release_null_safe(req);
	static_analyzer_malloc_freed(req);
	return error;
}

static bool
_dx_get_getaddrinfo_params(xpc_object_t msg, uint64_t *out_command_id, xpc_object_t *out_hostname,
	uint32_t *out_interface_index, DNSServiceFlags *out_flags, DNSServiceProtocol *out_protocols,
	pid_t *out_delegate_pid, const uint8_t **out_delegate_uuid, bool *out_need_auth_tags)
{
	bool params_are_valid = false;
	bool valid;
	const uint64_t command_id = dnssd_xpc_message_get_id(msg, &valid);
	require_quiet(valid, exit);

	const xpc_object_t params = dnssd_xpc_message_get_parameters(msg);
	require_quiet(params, exit);

	xpc_object_t hostname = dnssd_xpc_parameters_get_hostname_object(params);
	require_quiet(hostname, exit);

	const uint32_t interface_index = dnssd_xpc_parameters_get_interface_index(params, &valid);
	require_quiet(valid, exit);

	const DNSServiceFlags flags = dnssd_xpc_parameters_get_flags(params, &valid);
	require_quiet(valid, exit);

	const uint32_t protocols = dnssd_xpc_parameters_get_protocols(params, &valid);
	require_quiet(valid, exit);

	pid_t pid;
	const uint8_t * const uuid = dnssd_xpc_parameters_get_delegate_uuid(params);
	if (uuid) {
		pid = 0;
	} else {
		pid = dnssd_xpc_parameters_get_delegate_pid(params, NULL);
	}

	*out_command_id			= command_id;
	*out_hostname			= hostname;
	*out_interface_index	= interface_index;
	*out_flags				= flags;
	*out_protocols			= protocols;
	*out_delegate_pid		= pid;
	*out_delegate_uuid		= uuid;
	*out_need_auth_tags		= dnssd_xpc_parameters_get_need_authentication_tags(params);
	params_are_valid = true;

exit:
	return params_are_valid;
}

//======================================================================================================================

static DNSServiceErrorType
_dx_session_handle_stop_command(dx_session_t me, xpc_object_t msg)
{
	bool valid;
	DNSServiceErrorType error;
	const uint64_t command_id = dnssd_xpc_message_get_id(msg, &valid);
	require_action_quiet(valid, exit, error = kDNSServiceErr_BadParam);

	dx_request_t * ptr;
	dx_request_t req;
	for (ptr = &me->request_list; (req = *ptr) != NULL; ptr = &req->next) {
		if (req->command_id == command_id) {
			break;
		}
	}
	require_action_quiet(req, exit, error = kDNSServiceErr_BadReference);

	*ptr		= req->next;
	req->next	= NULL;

	_dx_invalidate(req);
	_dx_release(req);
	error = kDNSServiceErr_NoError;

exit:
	return error;
}

//======================================================================================================================

static void
_dx_session_send_results(dx_session_t me)
{
	bool success = false;
	for (dx_request_t req = me->request_list; req; req = req->next) {
		if (req->result_array && (xpc_array_get_count(req->result_array) > 0)) {
			const xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
			require_quiet(msg, exit);

			dnssd_xpc_message_set_id(msg, req->command_id);
			dnssd_xpc_message_set_error(msg, kDNSServiceErr_NoError);
			dnssd_xpc_message_set_results(msg, req->result_array);
			xpc_connection_send_message(me->connection, msg);
			xpc_release(msg);

			xpc_release(req->result_array);
			req->result_array = xpc_array_create(NULL, 0);
			require_quiet(req->result_array, exit);
		}
		if (req->error && !req->sent_error) {
			const xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
			require_quiet(msg, exit);

			dnssd_xpc_message_set_id(msg, req->command_id);
			dnssd_xpc_message_set_error(msg, req->error);
			xpc_connection_send_message(me->connection, msg);
			xpc_release(msg);
			req->sent_error = true;
		}
	}
	success = true;

exit:
	if (unlikely(!success)) {
		xpc_connection_cancel(me->connection);
	}
}

#if 0
//======================================================================================================================
#pragma mark - Request Methods
#endif

static void
_dx_request_finalize(dx_request_t me)
{
	xpc_forget(&me->result_array);
}

#if 0
//======================================================================================================================
#pragma mark - GetAddrInfo Request Methods
#endif

static dx_getaddrinfo_request_t
_dx_getaddrinfo_request_create(uint64_t command_id, uint32_t request_id)
{
	dx_getaddrinfo_request_t req = NULL;
	dx_getaddrinfo_request_t obj = _dx_getaddrinfo_request_alloc();
	require_quiet(obj, exit);

	obj->base.command_id	= command_id;
	obj->base.request_id	= request_id;
	obj->base.result_array	= xpc_array_create(NULL, 0);
	require_quiet(obj->base.result_array, exit);

	req = obj;
	obj = NULL;

exit:
	_dx_release_null_safe(obj);
	return req;
}

//======================================================================================================================

static DNSServiceErrorType
_dx_getaddrinfo_request_set_hostname(dx_getaddrinfo_request_t me, xpc_object_t hostname)
{
	DNSServiceErrorType err;
	require_action_quiet(xpc_string_get_length(hostname) <= MAX_ESCAPED_DOMAIN_NAME, exit,
		err = kDNSServiceErr_BadParam);

	xpc_release_null_safe(me->hostname);
	me->hostname = xpc_copy(hostname);
	require_action_quiet(me->hostname, exit, err = kDNSServiceErr_NoMemory);

	err = kDNSServiceErr_NoError;

exit:
	return err;
}

//======================================================================================================================

static void
_dx_getaddrinfo_request_set_need_authenticaed_results(dx_getaddrinfo_request_t me, bool need, const uuid_t client_uuid)
{
	if (need) {
		uuid_copy(me->client_uuid, client_uuid);
		me->need_auth = true;
	} else {
		uuid_clear(me->client_uuid);
		me->need_auth = false;
	}
}

//======================================================================================================================

static DNSServiceErrorType
_dx_getaddrinfo_request_activate(dx_getaddrinfo_request_t me, uint32_t interface_index, DNSServiceFlags flags,
	DNSServiceProtocol protocols, pid_t pid, const uuid_t uuid, uid_t uid)
{
	DNSServiceErrorType err;
	const char * const hostname_str = xpc_string_get_string_ptr(me->hostname);
	require_action_quiet(hostname_str, exit, err = kDNSServiceErr_Unknown);

	err = GetAddrInfoClientRequestStart(&me->gai, me->base.request_id, hostname_str, interface_index, flags,
		protocols, pid, uuid, uid, _dx_getaddrinfo_request_result_handler, me);
	require_noerr_quiet(err, exit);

	_dx_retain(me);
	me->active = true;

exit:
	return err;
}

//======================================================================================================================

static void
_dx_getaddrinfo_request_invalidate(dx_getaddrinfo_request_t me)
{
	if (me->active) {
		GetAddrInfoClientRequestStop(&me->gai);
		me->active = false;
		_dx_release(me);
	}
}

//======================================================================================================================

static void
_dx_getaddrinfo_request_finalize(dx_getaddrinfo_request_t me)
{
	xpc_forget(&me->hostname);
}

//======================================================================================================================

#if defined(NECP_CLIENT_ACTION_SIGN)
#define DNSSD_AUTHENTICATION_TAG_SIZE	32	// XXX: Defined as a workaround until NECP header defines this length.

static bool
_dx_authenticate_answer(uuid_t client_id, xpc_object_t hostname, int record_type, const void *record_data,
	uint8_t out_auth_tag[STATIC_PARAM DNSSD_AUTHENTICATION_TAG_SIZE]);
#endif

static void
_dx_getaddrinfo_request_result_handler(mDNS *m, DNSQuestion *question, const ResourceRecord *answer,
	QC_result qc_result, DNSServiceErrorType error, void *context)
{
	(void)question;

	const dx_getaddrinfo_request_t me = (dx_getaddrinfo_request_t)context;
	if (error && (error != kDNSServiceErr_NoSuchRecord)) {
		if (!me->base.error) {
			me->base.error = error;
		}
		goto exit;
	}
	require_quiet((answer->rrtype == kDNSServiceType_A) || (answer->rrtype == kDNSServiceType_AAAA), exit);

	const void *	rdata_ptr;
	size_t			rdata_len;
	if (!error) {
		if (answer->rrtype == kDNSServiceType_A) {
			rdata_ptr = answer->rdata->u.ipv4.b;
			rdata_len = 4;
		} else {
			rdata_ptr = answer->rdata->u.ipv6.b;
			rdata_len = 16;
		}
	} else {
		rdata_ptr = NULL;
		rdata_len = 0;
	}

	DNSServiceFlags flags = 0;
	if (qc_result != QC_rmv) {
		flags |= kDNSServiceFlagsAdd;
	}
    if (answer->mortality == Mortality_Ghost) {
		flags |= kDNSServiceFlagsExpiredAnswer;
	}
	if (!question->InitialCacheMiss) {
		flags |= kDNSServiceFlagAnsweredFromCache;
	}
	
	const uint32_t interface_index = mDNSPlatformInterfaceIndexfromInterfaceID(m, answer->InterfaceID, mDNStrue);
	const xpc_object_t result = xpc_dictionary_create(NULL, NULL, 0);
	if (likely(result)) {
    	char name_str[MAX_ESCAPED_DOMAIN_NAME];
    	ConvertDomainNameToCString(answer->name, name_str);

		dnssd_xpc_result_set_error(result, error);
		dnssd_xpc_result_set_flags(result, flags);
		dnssd_xpc_result_set_interface_index(result, interface_index);
		dnssd_xpc_result_set_record_name(result, name_str);
		dnssd_xpc_result_set_record_type(result, answer->rrtype);
		dnssd_xpc_result_set_record_class(result, answer->rrclass);
		dnssd_xpc_result_set_record_data(result, rdata_ptr, rdata_len);

#if defined(NECP_CLIENT_ACTION_SIGN)
		if (me->need_auth && !error && (flags & kDNSServiceFlagsAdd)) {
			uint8_t auth_tag[DNSSD_AUTHENTICATION_TAG_SIZE];
			const bool success = _dx_authenticate_answer(me->client_uuid, me->hostname, answer->rrtype, rdata_ptr,
				auth_tag);
			if (success) {
				dnssd_xpc_result_set_authentication_tag(result, auth_tag, sizeof(auth_tag));
			}
		}
#endif
		xpc_array_append_value(me->base.result_array, result);
		xpc_release(result);
	} else {
		me->base.error = kDNSServiceErr_NoMemory;
	}

exit:
	return;
}

#if defined(NECP_CLIENT_ACTION_SIGN)
typedef struct {
	struct necp_client_resolver_answer	hdr;
	uint8_t								hostname[MAX_ESCAPED_DOMAIN_NAME];
} dx_necp_answer_t;

check_compile_time(offsetof(dx_necp_answer_t, hdr) == 0);
check_compile_time(endof_field(struct necp_client_resolver_answer, hostname_length) == offsetof(dx_necp_answer_t, hostname));

static bool
_dx_authenticate_answer(uuid_t client_id, xpc_object_t hostname, int record_type, const void *record_data,
	uint8_t out_auth_tag[STATIC_PARAM DNSSD_AUTHENTICATION_TAG_SIZE])
{
	static int necp_fd = -1;

	bool success = false;
	if (necp_fd < 0) {
		necp_fd = necp_open(0);
	}
	require_quiet(necp_fd >= 0, exit);

	dx_necp_answer_t answer;
	memset(&answer, 0, sizeof(answer));

	struct necp_client_resolver_answer * const hdr = &answer.hdr;
	uuid_copy(hdr->client_id, client_id);

	hdr->sign_type = NECP_CLIENT_SIGN_TYPE_RESOLVER_ANSWER;

	switch (record_type) {
		case kDNSServiceType_A:
			hdr->address_answer.sa.sa_family	= AF_INET;
			hdr->address_answer.sa.sa_len		= sizeof(struct sockaddr_in);
			memcpy(&hdr->address_answer.sin.sin_addr.s_addr, record_data, 4);
			break;

		case kDNSServiceType_AAAA:
			hdr->address_answer.sa.sa_family	= AF_INET6;
			hdr->address_answer.sa.sa_len		= sizeof(struct sockaddr_in6);
			memcpy(hdr->address_answer.sin6.sin6_addr.s6_addr, record_data, 16);
			break;

		default:
			goto exit;
	}
	const size_t hostname_len = xpc_string_get_length(hostname);
	require_quiet(hostname_len <= sizeof(answer.hostname), exit);

	hdr->hostname_length = (uint32_t)hostname_len;
	memcpy(answer.hostname, xpc_string_get_string_ptr(hostname), hdr->hostname_length);

	const int necp_err = necp_client_action(necp_fd, NECP_CLIENT_ACTION_SIGN, (void *)&answer,
		sizeof(answer.hdr) + hdr->hostname_length, out_auth_tag, DNSSD_AUTHENTICATION_TAG_SIZE);
	require_noerr_quiet(necp_err, exit);

	success = true;

exit:
	return success;
}
#endif	// defined(NECP_CLIENT_ACTION_SIGN)
