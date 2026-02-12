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

#include "dnssd_private.h"

#include "dnssd_object.h"
#include "dnssd_xpc.h"

#include <CoreUtils/CoreUtils.h>
#include <os/object_private.h>
#include <xpc/private.h>

#if 0
//======================================================================================================================
#pragma mark - Kind Declarations
#endif

#define DNSSD_STRUCT(NAME)	struct dnssd_ ## NAME ## _s
#define DNSSD_TYPE(NAME)	dnssd_ ## NAME ## _t

#define DNSSD_KIND_DECLARE(NAME)																\
	static DNSSD_TYPE(NAME)																		\
	_dnssd_ ## NAME ## _alloc(void);															\
																								\
	static char *																				\
	_dnssd_ ## NAME ## _copy_description(DNSSD_TYPE(NAME) object, bool debug, bool privacy);	\
																								\
	static void																					\
	_dnssd_ ## NAME ## _finalize(DNSSD_TYPE(NAME) object)

// Note: The last check checks if the base's type is equal to that of the superkind. If it's not, then the pointer
// comparison used as the argument to sizeof will cause a "comparison of distinct pointer types" warning, so long as
// the warning hasn't been disabled.

#define DNSSD_BASE_CHECK(NAME, SUPER)																	\
	check_compile_time(offsetof(DNSSD_STRUCT(NAME), base) == 0);										\
	check_compile_time(sizeof_field(DNSSD_STRUCT(NAME), base) == sizeof(DNSSD_STRUCT(SUPER)));			\
	extern int _dnssd_base_type_check[sizeof(&(((DNSSD_TYPE(NAME))0)->base) == ((DNSSD_TYPE(SUPER))0))]

#define DNSSD_KIND_DEFINE(NAME, SUPER) 											\
	static const struct dnssd_kind_s _dnssd_ ## NAME ## _kind = {				\
		&_dnssd_ ## SUPER ## _kind,												\
		# NAME,																	\
		_dnssd_ ## NAME ## _copy_description,									\
		_dnssd_ ## NAME ## _finalize,											\
	};																			\
																				\
	static DNSSD_TYPE(NAME)														\
	_dnssd_ ## NAME ## _alloc(void)												\
	{																			\
		DNSSD_TYPE(NAME) obj = dnssd_object_ ## NAME ## _alloc(sizeof(*obj));	\
		require_quiet(obj, exit);												\
																				\
		const dnssd_object_t base = (dnssd_object_t)obj;						\
		base->kind = &_dnssd_ ## NAME ## _kind;									\
																				\
	exit:																		\
		return obj;																\
	}																			\
	DNSSD_BASE_CHECK(NAME, SUPER)

DNSSD_KIND_DECLARE(getaddrinfo);
DNSSD_KIND_DECLARE(getaddrinfo_result);

typedef char *	(*dnssd_copy_description_f)(dnssd_any_t object, bool debug, bool privacy);
typedef void	(*dnssd_finalize_f)(dnssd_any_t object);

typedef const struct dnssd_kind_s *	dnssd_kind_t;
struct dnssd_kind_s {
	dnssd_kind_t				superkind;			// This kind's superkind.
	const char *				name;				// Name of this kind.
	dnssd_copy_description_f	copy_description;	// Creates a textual description of object.
	dnssd_finalize_f			finalize;			// Releases object's resources right before the object is freed.
};

#if 0
//======================================================================================================================
#pragma mark - dnssd_object Kind Definition
#endif

struct dnssd_object_s {
	_OS_OBJECT_HEADER(const void *_os_obj_isa, _os_obj_refcnt, _os_obj_xref_cnt);
	dnssd_kind_t	kind;	// Pointer to an object's kind.
};

static const struct dnssd_kind_s _dnssd_object_kind = {
	NULL,		// No superkind.
	"object",
	NULL,		// No copy_description method.
	NULL,		// No finalize method.
};

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo Kind Definition
#endif

typedef enum {
	dnssd_getaddrinfo_state_nascent		= 0,
	dnssd_getaddrinfo_state_starting	= 1,
	dnssd_getaddrinfo_state_started		= 2,
	dnssd_getaddrinfo_state_failed		= 3,
	dnssd_getaddrinfo_state_invalidated	= 4,
} dnssd_getaddrinfo_state_t;

struct dnssd_getaddrinfo_s {
	struct dnssd_object_s				base;			// Object base.
	dnssd_getaddrinfo_t					next;			// Next getaddrinfo object in list.
	uint64_t							command_id;		// Command ID.
	dispatch_queue_t					user_queue;		// User's dispatch queue for invoking result and event handlers.
	dispatch_queue_t					mutex_queue;	// Mutex for accessing result_list from different queues.
	xpc_object_t						params;			// Parameters dictionary for getaddrinfo command.
	xpc_object_t						hostname;		// Reference to hostname from parameters dictionary.
	dispatch_source_t					event_source;	// Data source for triggering result and event handlers.
	dnssd_getaddrinfo_result_t			result_list;	// List of getaddrinfo results.
	dnssd_getaddrinfo_result_handler_t	result_handler;	// User's result handler.
	dnssd_event_handler_t				event_handler;	// User's event handler.
	dnssd_getaddrinfo_state_t			state;			// Internal state.
	OSStatus							error;			// Pending error.
	bool								user_activated;	// True if the object has been activated by user.
};

DNSSD_KIND_DEFINE(getaddrinfo, object);

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo_result Kind Definition
#endif

struct dnssd_getaddrinfo_result_s {
	struct dnssd_object_s			base;				// Object base.
	sockaddr_ip						addr;				// IPv4 or IPv6 address of hostname.
	xpc_object_t					hostname;			// Requested hostname to resolve.
	xpc_object_t					actual_hostname;	// The actual/canonical hostname of the requested hostname.
	xpc_object_t					auth_tag;			// Authentication tag.
	uint32_t						interface_index;	// Interface index to which the result pertains.
	dnssd_getaddrinfo_result_type_t	type;				// Type of getaddrinfo result.
	dnssd_getaddrinfo_result_t		next;				// Next getaddrinfo result in list.
};

DNSSD_KIND_DEFINE(getaddrinfo_result, object);

#if 0
//======================================================================================================================
#pragma mark - Constants
#endif

#define DNSSD_EVENT_HAVE_RESULTS	(1U << 0)	// Results are available.
#define DNSSD_EVENT_REMOVE_ALL		(1U << 1)	// Previously delivered results are no longer valid.
#define DNSSD_EVENT_ERROR			(1U << 2)	// An error was encountered.

// Strings for redacted description items.

#define DNSSD_REDACTED_HOSTNAME_STR			"<redacted hostname>"
#define DNSSD_REDACTED_ACTUAL_HOSTNAME_STR	"<redacted actual hostname>"
#define DNSSD_REDACTED_IPv4_ADDRESS_STR		"<redacted IPv4 address>"
#define DNSSD_REDACTED_IPv6_ADDRESS_STR		"<redacted IPv6 address>"

#if 0
//======================================================================================================================
#pragma mark - Local Prototypes
#endif

static dispatch_queue_t
_dnssd_client_queue(void);

static xpc_connection_t
_dnssd_client_connection(void);

static uint64_t
_dnssd_client_get_new_id(void);

static void
_dnssd_client_activate_getaddrinfo_async(dnssd_getaddrinfo_t gai);

static void
_dnssd_client_register_getaddrinfo(dnssd_getaddrinfo_t gai);

static void
_dnssd_client_deregister_getaddrinfo(dnssd_getaddrinfo_t gai);

static OSStatus
_dnssd_client_send_getaddrinfo_command(dnssd_getaddrinfo_t gai);

static void
_dnssd_client_fail_getaddrinfo(dnssd_getaddrinfo_t gai, OSStatus error);

static void
_dnssd_getaddrinfo_append_results(dnssd_getaddrinfo_t gai, dnssd_getaddrinfo_result_t result_list);

static void
_dnssd_getaddrinfo_remove_all_results(dnssd_getaddrinfo_t gai);

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_take_results(dnssd_getaddrinfo_t gai);

static void
_dnssd_getaddrinfo_post_error_event(dnssd_getaddrinfo_t gai, OSStatus error);

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_result_create_from_dictionary(xpc_object_t hostname, xpc_object_t result_dict, OSStatus *out_error);

static DNSServiceErrorType
_dnssd_osstatus_to_dns_service_error(OSStatus status);

#if !defined(dnssd_release_null_safe)
	#define dnssd_release_null_safe(X)	\
		do {							\
			if (X) {					\
				dnssd_release(X);		\
			}							\
		} while(0)
#endif

#if !defined(dnssd_forget)
	#define dnssd_forget(X)	ForgetCustom(X, dnssd_release)
#endif

#if 0
//======================================================================================================================
#pragma mark - dnssd_object Public Methods
#endif

void
dnssd_retain(dnssd_any_t object)
{
	os_retain(object.base);
}

//======================================================================================================================

void
dnssd_release(dnssd_any_t object)
{
	os_release(object.base);
}

//======================================================================================================================

char *
dnssd_copy_description(dnssd_any_t object)
{
	return dnssd_object_copy_description(object, false, false);
}

#if 0
//======================================================================================================================
#pragma mark - dnssd_object Private Methods
#endif

char *
dnssd_object_copy_description(dnssd_any_t object, bool debug, bool privacy)
{
	for (dnssd_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
		if (kind->copy_description) {
			char *desc = kind->copy_description(object, debug, privacy);
			return desc;
		}
	}
	return NULL;
}

//======================================================================================================================

void
dnssd_object_finalize(dnssd_any_t object)
{
	for (dnssd_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
		if (kind->finalize) {
			kind->finalize(object);
		}
	}
}

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo Public Methods
#endif

dnssd_getaddrinfo_t
dnssd_getaddrinfo_create(void)
{
	dnssd_getaddrinfo_t	gai = NULL;
	dnssd_getaddrinfo_t	obj = _dnssd_getaddrinfo_alloc();
	require_quiet(obj, exit);

	obj->params = xpc_dictionary_create(NULL, NULL, 0);
	require_quiet(obj->params, exit);

	obj->mutex_queue = dispatch_queue_create("com.apple.dnssd.getaddrinfo.mutex", DISPATCH_QUEUE_SERIAL);
	require_quiet(obj->mutex_queue, exit);

	gai = obj;
	obj = NULL;

exit:
	dnssd_release_null_safe(obj);
	return gai;
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_queue(dnssd_getaddrinfo_t me, dispatch_queue_t queue)
{
	if (!me->user_activated) {
		dispatch_retain(queue);
		dispatch_release_null_safe(me->user_queue);
		me->user_queue = queue;
	} else if (!me->user_queue) {
		me->user_queue = queue;
		dispatch_retain(me->user_queue);
		_dnssd_client_activate_getaddrinfo_async(me);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_flags(dnssd_getaddrinfo_t me, DNSServiceFlags flags)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_flags(me->params, flags);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_hostname(dnssd_getaddrinfo_t me, const char *hostname)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_hostname(me->params, hostname);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_interface_index(dnssd_getaddrinfo_t me, uint32_t interface_index)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_interface_index(me->params, interface_index);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_protocols(dnssd_getaddrinfo_t me, DNSServiceProtocol protocols)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_protocols(me->params, protocols);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_delegate_pid(dnssd_getaddrinfo_t me, pid_t pid)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_delegate_pid(me->params, pid);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_delegate_uuid(dnssd_getaddrinfo_t me, uuid_t uuid)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_delegate_uuid(me->params, uuid);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_result_handler(dnssd_getaddrinfo_t me, dnssd_getaddrinfo_result_handler_t handler)
{
	dnssd_getaddrinfo_result_handler_t const new_handler = handler ? Block_copy(handler) : NULL;
	if (me->result_handler) {
		Block_release(me->result_handler);
	}
	me->result_handler = new_handler;
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_event_handler(dnssd_getaddrinfo_t me, dnssd_event_handler_t handler)
{
	dnssd_event_handler_t const new_handler = handler ? Block_copy(handler) : NULL;
	if (me->event_handler) {
		Block_release(me->event_handler);
	}
	me->event_handler = new_handler;
}

//======================================================================================================================

void
dnssd_getaddrinfo_set_need_authenticated_results(dnssd_getaddrinfo_t me, bool need)
{
	if (!me->user_activated) {
		dnssd_xpc_parameters_set_need_authentication_tags(me->params, need);
	}
}

//======================================================================================================================

void
dnssd_getaddrinfo_activate(dnssd_getaddrinfo_t me)
{
	if (!me->user_activated) {
		if (me->user_queue) {
			_dnssd_client_activate_getaddrinfo_async(me);
		}
		me->user_activated = true;
	}
}

//======================================================================================================================

static void
_dnssd_client_invalidate_getaddrinfo(dnssd_getaddrinfo_t gai);

static void
_dnssd_getaddrinfo_invalidate(dnssd_getaddrinfo_t me);

void
dnssd_getaddrinfo_invalidate(dnssd_getaddrinfo_t me)
{
	dnssd_retain(me);
	dispatch_async(_dnssd_client_queue(),
	^{
		_dnssd_client_invalidate_getaddrinfo(me);
		dnssd_release(me);
	});
}

static void
_dnssd_client_invalidate_getaddrinfo(dnssd_getaddrinfo_t gai)
{
	require_quiet(gai->state != dnssd_getaddrinfo_state_invalidated, exit);

	_dnssd_client_deregister_getaddrinfo(gai);
	if ((gai->state == dnssd_getaddrinfo_state_starting) || (gai->state == dnssd_getaddrinfo_state_started)) {
		xpc_object_t const msg = xpc_dictionary_create(NULL, NULL, 0);
		if (msg) {
			dnssd_xpc_message_set_id(msg, gai->command_id);
			dnssd_xpc_message_set_command(msg, DNSSD_COMMAND_STOP);
			xpc_connection_send_message_with_reply(_dnssd_client_connection(), msg, _dnssd_client_queue(),
			^(xpc_object_t reply)
			{
				(void)reply;
			});
			xpc_release(msg);
		}
	}
	_dnssd_getaddrinfo_invalidate(gai);
	gai->state = dnssd_getaddrinfo_state_invalidated;

exit:
	return;
}

static void
_dnssd_getaddrinfo_invalidate(dnssd_getaddrinfo_t me)
{
	dispatch_source_forget(&me->event_source);
	_dnssd_getaddrinfo_remove_all_results(me);

	if (me->user_queue) {
		dnssd_retain(me);
		dispatch_async(me->user_queue,
		^{
			if (me->event_handler) {
				me->event_handler(dnssd_event_invalidated, kDNSServiceErr_NoError);
			}
			dnssd_release(me);
		});
	}
}

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo Private Methods
#endif

static char *
_dnssd_getaddrinfo_copy_description(dnssd_getaddrinfo_t me, const bool debug, const bool privacy)
{
	const char *hostname;
	if (me->hostname) {
		hostname = xpc_string_get_string_ptr(me->hostname);
		if (privacy && hostname) {
			hostname = DNSSD_REDACTED_HOSTNAME_STR;
		}
	} else {
		hostname = NULL;
	}

	char *	desc	= NULL;
	char *	buf_ptr	= NULL;
	size_t	buf_len	= 0;
	for (;;)
	{
		size_t			need	= 0;
		char *			dst		= buf_ptr;
		char * const	end		= &buf_ptr[buf_len];
		int				n;

		if (debug) {
			const size_t len = (size_t)(end - dst);
			n = snprintf(dst, len, "dnssd_%s (%p): ", me->base.kind->name, (void *)me);
			require_quiet(n >= 0, exit);

			if (buf_ptr) {
				dst = (((size_t)n) < len) ? &dst[n] : end;
			} else {
				need += (size_t)n;
			}
		}
		n = snprintf(dst, (size_t)(end - dst), "hostname = %s", hostname ? hostname : "<NO HOSTNAME>");
		require_quiet(n >= 0, exit);

		if (!buf_ptr) {
			need += (size_t)n;
			buf_len = need + 1;
			buf_ptr = malloc(buf_len);
			require_quiet(buf_ptr, exit);
		} else {
			break;
		}
	}
	desc	= buf_ptr;
	buf_ptr	= NULL;

exit:
	FreeNullSafe(buf_ptr);
	return desc;
}

//======================================================================================================================

static void
_dnssd_getaddrinfo_finalize(dnssd_getaddrinfo_t me)
{
	dispatch_forget(&me->user_queue);
	dispatch_forget(&me->mutex_queue);
	xpc_forget(&me->params);
	xpc_forget(&me->hostname);
	BlockForget(&me->result_handler);
	BlockForget(&me->event_handler);
}

//======================================================================================================================

static void
_dnssd_getaddrinfo_append_results(dnssd_getaddrinfo_t me, dnssd_getaddrinfo_result_t result_list)
{
	dispatch_sync(me->mutex_queue,
	^{
		dnssd_getaddrinfo_result_t *ptr = &me->result_list;
		while (*ptr) {
			ptr = &(*ptr)->next;
		}
		*ptr = result_list;
	});
	dispatch_source_merge_data(me->event_source, DNSSD_EVENT_HAVE_RESULTS);
}

//======================================================================================================================

static void
_dnssd_getaddrinfo_remove_all_results(dnssd_getaddrinfo_t me)
{
	dnssd_getaddrinfo_result_t result_list = _dnssd_getaddrinfo_take_results(me);
	if (me->event_source) {
		dispatch_source_merge_data(me->event_source, DNSSD_EVENT_REMOVE_ALL);
	}

	dnssd_getaddrinfo_result_t result;
	while ((result = result_list) != NULL) {
		result_list = result->next;
		dnssd_release(result);
	}
}

//======================================================================================================================

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_take_results(dnssd_getaddrinfo_t me)
{
	__block dnssd_getaddrinfo_result_t list;
	dispatch_sync(me->mutex_queue,
	^{
		list = me->result_list;
		me->result_list = NULL;
	});
	return list;
}

//======================================================================================================================

static void
_dnssd_getaddrinfo_post_error_event(dnssd_getaddrinfo_t me, OSStatus error)
{
	dispatch_sync(me->mutex_queue,
	^{
		me->error = error;
	});
	dispatch_source_merge_data(me->event_source, DNSSD_EVENT_ERROR);
}

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo_result Public Methods
#endif

dnssd_getaddrinfo_result_type_t
dnssd_getaddrinfo_result_get_type(dnssd_getaddrinfo_result_t me)
{
	return me->type;
}

//======================================================================================================================

const char *
dnssd_getaddrinfo_result_get_actual_hostname(dnssd_getaddrinfo_result_t me)
{
	return xpc_string_get_string_ptr(me->actual_hostname);
}

//======================================================================================================================

const struct sockaddr *
dnssd_getaddrinfo_result_get_address(dnssd_getaddrinfo_result_t me)
{
	return &me->addr.sa;
}

//======================================================================================================================

const char *
dnssd_getaddrinfo_result_get_hostname(dnssd_getaddrinfo_result_t me)
{
	return xpc_string_get_string_ptr(me->hostname);
}

//======================================================================================================================

uint32_t
dnssd_getaddrinfo_result_get_interface_index(dnssd_getaddrinfo_result_t me)
{
	return me->interface_index;
}

//======================================================================================================================

const void *
dnssd_getaddrinfo_result_get_authentication_tag(dnssd_getaddrinfo_result_t me, size_t *out_length)
{
	const void *	auth_tag_ptr;
	size_t			auth_tag_len;

	if (me->auth_tag) {
		auth_tag_ptr = xpc_data_get_bytes_ptr(me->auth_tag);
		auth_tag_len = xpc_data_get_length(me->auth_tag);
	} else {
		auth_tag_ptr = NULL;
		auth_tag_len = 0;
	}
	if (out_length) {
		*out_length = auth_tag_len;
	}
	return auth_tag_ptr;
}

#if 0
//======================================================================================================================
#pragma mark - dnssd_getaddrinfo_result Private Methods
#endif

static char *
_dnssd_getaddrinfo_result_copy_description(dnssd_getaddrinfo_result_t me, const bool debug, const bool privacy)
{
	const char *hostname;
	if (me->hostname) {
		hostname = xpc_string_get_string_ptr(me->hostname);
		if (privacy && hostname) {
			hostname = DNSSD_REDACTED_HOSTNAME_STR;
		}
	} else {
		hostname = NULL;
	}

	const char *actual_hostname;
	if (me->actual_hostname) {
		actual_hostname = xpc_string_get_string_ptr(me->actual_hostname);
		if (privacy && actual_hostname) {
			actual_hostname = DNSSD_REDACTED_ACTUAL_HOSTNAME_STR;
		}
	} else {
		actual_hostname = NULL;
	}
	char addr_buf[INET6_ADDRSTRLEN];
	check_compile_time_code(sizeof(addr_buf) >= INET_ADDRSTRLEN);
	check_compile_time_code(sizeof(addr_buf) >= INET6_ADDRSTRLEN);

	const char *addr;
	if (me->addr.sa.sa_family == AF_INET) {
		if (privacy) {
			addr = DNSSD_REDACTED_IPv4_ADDRESS_STR;
		} else {
			addr = inet_ntop(AF_INET, &me->addr.v4.sin_addr.s_addr, addr_buf, (socklen_t)sizeof(addr_buf));
		}
	} else if (me->addr.sa.sa_family == AF_INET6) {
		if (privacy) {
			addr = DNSSD_REDACTED_IPv6_ADDRESS_STR;
		} else {
			addr = inet_ntop(AF_INET6, me->addr.v6.sin6_addr.s6_addr, addr_buf, (socklen_t)sizeof(addr_buf));
		}
	} else {
		addr = NULL;
	}

	char *	desc	= NULL;
	char *	buf_ptr	= NULL;
	size_t	buf_len	= 0;
	for (;;)
	{
		char *			dst		= buf_ptr;
		char * const	end		= &buf_ptr[buf_len];
		size_t			need	= 0;
		int				n;

		if (debug) {
			const size_t len = (size_t)(end - dst);
			n = snprintf(dst, len, "dnssd_%s (%p): ", me->base.kind->name, (void *)me);
			require_quiet(n >= 0, exit);

			if (buf_ptr) {
				dst = (((size_t)n) < len) ? &dst[n] : end;
			} else {
				need += (size_t)n;
			}
		}
		n = snprintf(dst, (size_t)(end - dst), "[%s] %s (%s) -> %s (interface index %lu)",
			dnssd_getaddrinfo_result_type_to_string(me->type), hostname ? hostname : "<NO HOSTNAME>",
			actual_hostname ? actual_hostname : "<NO ACTUAL HOSTNAME>", addr ? addr : "<NO IP ADDRESS>",
			(unsigned long)me->interface_index);
		require_quiet(n >= 0, exit);

		if (!buf_ptr) {
			need += (size_t)n;
			buf_len = need + 1;
			buf_ptr = malloc(buf_len);
			require_quiet(buf_ptr, exit);
		} else {
			break;
		}
	}
	desc	= buf_ptr;
	buf_ptr	= NULL;

exit:
	FreeNullSafe(buf_ptr);
	return desc;
}

//======================================================================================================================

void
_dnssd_getaddrinfo_result_finalize(dnssd_getaddrinfo_result_t me)
{
	xpc_forget(&me->hostname);
	xpc_forget(&me->actual_hostname);
	xpc_forget(&me->auth_tag);
}

#if 0
//======================================================================================================================
#pragma mark - dnssd Client
#endif

static dnssd_getaddrinfo_t g_gai_list = NULL;

static dispatch_queue_t
_dnssd_client_queue(void)
{
	static dispatch_once_t	once	= 0;
	static dispatch_queue_t	queue	= NULL;

	dispatch_once(&once,
	^{
		dispatch_queue_attr_t const attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
			QOS_CLASS_UTILITY, 0);
		queue = dispatch_queue_create("com.apple.dnssd.client", attr);
	});
	return queue;
}

//======================================================================================================================

static void
_dnssd_client_handle_message(xpc_object_t msg);
static void
_dnssd_client_handle_interruption(void);

static xpc_connection_t
_dnssd_client_connection(void)
{
	static dispatch_once_t	once		= 0;
	static xpc_connection_t	connection	= NULL;

	dispatch_once(&once,
	^{
		connection = xpc_connection_create_mach_service(DNSSD_MACH_SERVICE_NAME, _dnssd_client_queue(),
			XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
		xpc_connection_set_event_handler(connection,
		^(xpc_object_t event)
		{
			if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
				_dnssd_client_handle_message(event);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				_dnssd_client_handle_interruption();
			}
		});
		xpc_connection_activate(connection);
	});
	return connection;
}

static void
_dnssd_client_handle_message(xpc_object_t msg)
{
	const uint64_t command_id = dnssd_xpc_message_get_id(msg, NULL);
	dnssd_getaddrinfo_t gai;
	for (gai = g_gai_list; gai; gai = gai->next) {
		if (gai->command_id == command_id) {
			break;
		}
	}
	require_quiet(gai, exit);

	const OSStatus error = dnssd_xpc_message_get_error(msg, NULL);
	if (!error) {
		xpc_object_t const result_array = dnssd_xpc_message_get_results(msg);
		require_quiet(result_array, exit);

		dnssd_getaddrinfo_result_t				result_list	= NULL;
		__block dnssd_getaddrinfo_result_t *	result_ptr	= &result_list;
		xpc_array_apply(result_array,
		^bool(__unused size_t index, xpc_object_t result_dict)
		{
			dnssd_getaddrinfo_result_t result = _dnssd_getaddrinfo_result_create_from_dictionary(gai->hostname,
				result_dict, NULL);
			if (result) {
				*result_ptr	= result;
				result_ptr	= &result->next;
			}
			return true;
		});
		require_quiet(result_list, exit);

		_dnssd_getaddrinfo_append_results(gai, result_list);
		result_list = NULL;
	} else {
		_dnssd_client_fail_getaddrinfo(gai, error);
	}

exit:
	return;
}

static void
_dnssd_client_handle_interruption(void)
{
	dnssd_getaddrinfo_t next_gai;
	for (dnssd_getaddrinfo_t gai = g_gai_list; gai; gai = next_gai) {
		next_gai = gai->next;
		gai->state = dnssd_getaddrinfo_state_starting;
		const OSStatus err = _dnssd_client_send_getaddrinfo_command(gai);
		if (!err) {
			_dnssd_getaddrinfo_remove_all_results(gai);
		} else {
			_dnssd_client_fail_getaddrinfo(gai, err);
		}
	}
}

//======================================================================================================================

static uint64_t
_dnssd_client_get_new_id(void)
{
	static uint64_t last_id = 0;
	return ++last_id;
}

//======================================================================================================================

static void
_dnssd_client_activate_getaddrinfo(dnssd_getaddrinfo_t gai);

static OSStatus
_dnssd_getaddrinfo_activate(dnssd_getaddrinfo_t gai);

static void
_dnssd_client_activate_getaddrinfo_async(dnssd_getaddrinfo_t gai)
{
	dnssd_retain(gai);
	dispatch_async(_dnssd_client_queue(),
	^{
		_dnssd_client_activate_getaddrinfo(gai);
		dnssd_release(gai);
	});
}

static void
_dnssd_client_activate_getaddrinfo(dnssd_getaddrinfo_t gai)
{
	OSStatus err;
	require_action_quiet(gai->state == dnssd_getaddrinfo_state_nascent, exit, err = kNoErr);

	err = _dnssd_getaddrinfo_activate(gai);
	if (err) {
		gai->state = dnssd_getaddrinfo_state_failed;
		goto exit;
	}

	gai->command_id = _dnssd_client_get_new_id();
	gai->state = dnssd_getaddrinfo_state_starting;

	_dnssd_client_register_getaddrinfo(gai);

	err = _dnssd_client_send_getaddrinfo_command(gai);
	if (err) {
		_dnssd_client_fail_getaddrinfo(gai, err);
	}

exit:
	return;
}

static void
_dnssd_getaddrinfo_process_events(dnssd_getaddrinfo_t gai, unsigned long events);

static OSStatus
_dnssd_getaddrinfo_activate(dnssd_getaddrinfo_t me)
{
	OSStatus err;
	xpc_object_t const hostname = dnssd_xpc_parameters_get_hostname_object(me->params);
	require_action_quiet(hostname, exit, err = kParamErr);

	me->hostname = xpc_copy(hostname);
	require_action_quiet(me->hostname, exit, err = kNoResourcesErr);

	me->event_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_OR, 0, 0, me->user_queue);
	require_action_quiet(me->event_source, exit, err = kNoResourcesErr);

	dnssd_retain(me);
	dispatch_source_t const event_source = me->event_source;
	dispatch_source_set_event_handler(me->event_source,
	^{
		_dnssd_getaddrinfo_process_events(me, dispatch_source_get_data(event_source));
	});
	dispatch_source_set_cancel_handler(me->event_source,
	^{
		dnssd_release(me);
	});
	dispatch_activate(me->event_source);
	err = kNoErr;

exit:
	if (err) {
		dnssd_retain(me);
		dispatch_async(me->user_queue,
		^{
			if (me->event_handler) {
				me->event_handler(dnssd_event_error, _dnssd_osstatus_to_dns_service_error(err));
			}
			dnssd_release(me);
		});
	}
	return err;
}

static void
_dnssd_getaddrinfo_process_events(dnssd_getaddrinfo_t me, unsigned long events)
{
	if (events & DNSSD_EVENT_REMOVE_ALL) {
		if (me->event_handler) {
			me->event_handler(dnssd_event_remove_all, kDNSServiceErr_NoError);
		}
	}

	if (events & DNSSD_EVENT_HAVE_RESULTS) {
		dnssd_getaddrinfo_result_t result;
		dnssd_getaddrinfo_result_t result_array[32];
		dnssd_getaddrinfo_result_t result_list = _dnssd_getaddrinfo_take_results(me);

		size_t result_count = 0;
		while ((result = result_list) != NULL) {
			result_list		= result->next;
			result->next	= NULL;
			result_array[result_count++] = result;

			if ((result_count == countof(result_array)) || !result_list) {
				if (me->result_handler) {
					me->result_handler(result_array, result_count);
				}
				for (size_t i = 0; i < result_count; ++i) {
					dnssd_release(result_array[i]);
				}
				result_count = 0;
			}
		}
	}

	if (events & DNSSD_EVENT_ERROR) {
		__block OSStatus error;
		dispatch_sync(me->mutex_queue,
		^{
			error = me->error;
			me->error = kNoErr;
		});
		if (me->event_handler && error) {
			me->event_handler(dnssd_event_error, _dnssd_osstatus_to_dns_service_error(error));
		}
	}
}

//======================================================================================================================

static void
_dnssd_client_register_getaddrinfo(dnssd_getaddrinfo_t gai)
{
	gai->next	= g_gai_list;
	g_gai_list	= gai;
	dnssd_retain(gai);
}

//======================================================================================================================

static void
_dnssd_client_deregister_getaddrinfo(dnssd_getaddrinfo_t gai)
{
	dnssd_getaddrinfo_t *ptr;
	for (ptr = &g_gai_list; *ptr; ptr = &(*ptr)->next)
	{
		if (*ptr == gai) {
			break;
		}
	}
	if (*ptr) {
		*ptr = gai->next;
		gai->next = NULL;
		dnssd_release(gai);
	}
}

//======================================================================================================================

static void
_dnssd_client_handle_getaddrinfo_reply(dnssd_getaddrinfo_t gai, xpc_object_t reply);

static OSStatus
_dnssd_client_send_getaddrinfo_command(dnssd_getaddrinfo_t gai)
{
	OSStatus err;
	xpc_object_t const msg = xpc_dictionary_create(NULL, NULL, 0);
	require_action_quiet(msg, exit, err = kNoResourcesErr);

	dnssd_xpc_message_set_id(msg, gai->command_id);
	dnssd_xpc_message_set_command(msg, DNSSD_COMMAND_GETADDRINFO);
	dnssd_xpc_message_set_parameters(msg, gai->params);

	dnssd_retain(gai);
	xpc_connection_send_message_with_reply(_dnssd_client_connection(), msg, _dnssd_client_queue(),
	^(xpc_object_t reply)
	{
		_dnssd_client_handle_getaddrinfo_reply(gai, reply);
		dnssd_release(gai);
	});
	xpc_release(msg);
	err = kNoErr;

exit:
	return err;
}

static void
_dnssd_client_handle_getaddrinfo_reply(dnssd_getaddrinfo_t gai, xpc_object_t reply)
{
	require_quiet(gai->state == dnssd_getaddrinfo_state_starting, exit);

	if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
		const OSStatus error = dnssd_xpc_message_get_error(reply, NULL);
		if (error) {
			_dnssd_client_fail_getaddrinfo(gai, error);
		} else {
			gai->state = dnssd_getaddrinfo_state_started;
		}
	} else if (reply != XPC_ERROR_CONNECTION_INTERRUPTED) {
		OSStatus error;
		if (reply == XPC_ERROR_CONNECTION_INVALID) {
			error = kDNSServiceErr_ServiceNotRunning;
		} else {
			error = kDNSServiceErr_Unknown;
		}
		_dnssd_client_fail_getaddrinfo(gai, error);
	}

exit:
	return;
}

//======================================================================================================================

static void
_dnssd_client_fail_getaddrinfo(dnssd_getaddrinfo_t gai, OSStatus error)
{
	_dnssd_client_deregister_getaddrinfo(gai);
	gai->state = dnssd_getaddrinfo_state_failed;
	_dnssd_getaddrinfo_post_error_event(gai, error);
}

//======================================================================================================================
//	_dnssd_getaddrinfo_result_create_from_dictionary
//======================================================================================================================

static bool
_dnssd_extract_result_dict_values(xpc_object_t result, xpc_object_t *out_hostname, DNSServiceErrorType *out_error,
	DNSServiceFlags *out_flags, uint32_t *out_interface_index, uint16_t *out_type, uint16_t *out_class,
	xpc_object_t *out_rdata, xpc_object_t *out_auth_tag);

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_result_create(dnssd_getaddrinfo_result_type_t type, xpc_object_t hostname,
	xpc_object_t actual_hostname, int addr_family, const void *addr_data, uint32_t interface_index,
	xpc_object_t auth_tag, OSStatus *out_error);

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_result_create_from_dictionary(xpc_object_t hostname, xpc_object_t result_dict, OSStatus *out_error)
{
	OSStatus					err;
	xpc_object_t				actual_hostname, rdata, auth_tag;
	DNSServiceErrorType			error;
	DNSServiceFlags				flags;
	uint32_t					interface_index;
	uint16_t					rtype;
	dnssd_getaddrinfo_result_t	result = NULL;

	const bool success = _dnssd_extract_result_dict_values(result_dict, &actual_hostname, &error, &flags,
		&interface_index, &rtype, NULL, &rdata, &auth_tag);
	require_action_quiet(success, exit, err = kMalformedErr);

	if ((error != kDNSServiceErr_NoError) &&
		(error != kDNSServiceErr_NoSuchRecord)) {
		err = kUnexpectedErr;
		goto exit;
	}
	require_action_quiet((rtype == kDNSServiceType_A) || (rtype == kDNSServiceType_AAAA), exit, err = kTypeErr);

	dnssd_getaddrinfo_result_type_t result_type;
	if (error == kDNSServiceErr_NoSuchRecord) {
		result_type = dnssd_getaddrinfo_result_type_no_address;
	} else {
		if (flags & kDNSServiceFlagsAdd) {
			if (flags & kDNSServiceFlagsExpiredAnswer) {
				result_type = dnssd_getaddrinfo_result_type_expired;
			} else {
				result_type = dnssd_getaddrinfo_result_type_add;
			}
		} else {
			result_type = dnssd_getaddrinfo_result_type_remove;
		}
		if (rtype == kDNSServiceType_A) {
			require_action_quiet(xpc_data_get_length(rdata) == 4, exit, err = kMalformedErr);
		} else {
			require_action_quiet(xpc_data_get_length(rdata) == 16, exit, err = kMalformedErr);
		}
	}
	const int addr_family = (rtype == kDNSServiceType_A) ? AF_INET : AF_INET6;
	result = _dnssd_getaddrinfo_result_create(result_type, hostname, actual_hostname, addr_family,
		xpc_data_get_bytes_ptr(rdata), interface_index, auth_tag, &err);
	require_noerr_quiet(err, exit);

exit:
	if (err) {
		dnssd_forget(&result);
	}
	if (out_error) {
		*out_error = err;
	}
	return result;
}

static bool
_dnssd_extract_result_dict_values(xpc_object_t result, xpc_object_t *out_hostname, DNSServiceErrorType *out_error,
	DNSServiceFlags *out_flags, uint32_t *out_interface_index, uint16_t *out_type, uint16_t *out_class,
	xpc_object_t *out_rdata, xpc_object_t *out_auth_tag)
{
	bool result_is_valid = false;
	xpc_object_t const hostname = dnssd_xpc_result_get_record_name_object(result);
	require_quiet(hostname, exit);

	xpc_object_t const rdata = dnssd_xpc_result_get_record_data_object(result);
	require_quiet(rdata, exit);

	if (out_hostname) {
		*out_hostname = hostname;
	}
	if (out_error) {
		*out_error = dnssd_xpc_result_get_error(result, NULL);
	}
	if (out_flags) {
		*out_flags = dnssd_xpc_result_get_flags(result, NULL);
	}
	if (out_interface_index) {
		*out_interface_index = dnssd_xpc_result_get_interface_index(result, NULL);
	}
	if (out_type) {
		*out_type = dnssd_xpc_result_get_record_type(result, NULL);
	}
	if (out_class) {
		*out_class = dnssd_xpc_result_get_record_class(result, NULL);
	}
	if (out_rdata) {
		*out_rdata = rdata;
	}
	if (out_auth_tag) {
		*out_auth_tag = dnssd_xpc_result_get_authentication_tag_object(result);
	}
	result_is_valid = true;

exit:
	return result_is_valid;
}

static dnssd_getaddrinfo_result_t
_dnssd_getaddrinfo_result_create(dnssd_getaddrinfo_result_type_t type, xpc_object_t hostname,
	xpc_object_t actual_hostname, int addr_family, const void *addr_data, uint32_t interface_index,
	xpc_object_t auth_tag, OSStatus *out_error)
{
	OSStatus err;
	dnssd_getaddrinfo_result_t result = NULL;
	dnssd_getaddrinfo_result_t obj = _dnssd_getaddrinfo_result_alloc();
	require_action_quiet(obj, exit, err = kNoMemoryErr);

	switch (type) {
		case dnssd_getaddrinfo_result_type_add:
		case dnssd_getaddrinfo_result_type_remove:
		case dnssd_getaddrinfo_result_type_no_address:
		case dnssd_getaddrinfo_result_type_expired:
			break;

		default:
			err = kTypeErr;
			goto exit;
	}
	obj->type				= type;
	obj->interface_index	= interface_index;

	require_action_quiet(xpc_get_type(hostname) == XPC_TYPE_STRING, exit, err = kTypeErr);

	obj->hostname = xpc_copy(hostname);
	require_action_quiet(obj->hostname, exit, err = kNoResourcesErr);

	require_action_quiet(xpc_get_type(actual_hostname) == XPC_TYPE_STRING, exit, err = kTypeErr);

	obj->actual_hostname = xpc_copy(actual_hostname);
	require_action_quiet(obj->actual_hostname, exit, err = kNoResourcesErr);

	require_action_quiet((addr_family == AF_INET) || (addr_family == AF_INET6), exit, err = kTypeErr);

	if (addr_family == AF_INET) {
		obj->addr.sa.sa_family	= AF_INET;
		obj->addr.v4.sin_len	= sizeof(struct sockaddr_in);
		if (obj->type != dnssd_getaddrinfo_result_type_no_address) {
			memcpy(&obj->addr.v4.sin_addr.s_addr, addr_data, 4);
		}
	} else {
		obj->addr.sa.sa_family	= AF_INET6;
		obj->addr.v6.sin6_len	= sizeof(struct sockaddr_in6);
		if (obj->type != dnssd_getaddrinfo_result_type_no_address) {
			memcpy(&obj->addr.v6.sin6_addr.s6_addr, addr_data, 16);
		}
	}

	if (auth_tag) {
		require_action_quiet(xpc_get_type(auth_tag) == XPC_TYPE_DATA, exit, err = kTypeErr);

		obj->auth_tag = xpc_copy(auth_tag);
		require_action_quiet(obj->auth_tag, exit, err = kNoResourcesErr);
	}
	result	= obj;
	obj		= NULL;
	err = kNoErr;

exit:
	if (out_error) {
		*out_error = err;
	}
	dnssd_release_null_safe(obj);
	return result;
}

#if 0
//======================================================================================================================
#pragma mark - Misc. Helpers
#endif

static DNSServiceErrorType
_dnssd_osstatus_to_dns_service_error(OSStatus error)
{
	switch (error) {
		case kNoMemoryErr:
		case kNoResourcesErr:
			error = kDNSServiceErr_NoMemory;
			break;

		case kParamErr:
			error = kDNSServiceErr_BadParam;
			break;

		default:
			if ((error >= kGenericErrorBase) && (error <= kGenericErrorEnd)) {
				error = kDNSServiceErr_Unknown;
			}
			break;
	}
	return error;
}
