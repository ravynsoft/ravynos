/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mdns_private.h"

#include "mdns_object.h"

#include <CoreUtils/CoreUtils.h>
#include <network_information.h>
#include <notify.h>
#include <os/log.h>
#include <os/object_private.h>

//======================================================================================================================
// MARK: - Kind Declarations

#define MDNS_STRUCT(NAME)	struct mdns_ ## NAME ## _s

// Note: The last check checks if the base's type is equal to that of the superkind. If it's not, then the pointer
// comparison used as the argument to sizeof will cause a "comparison of distinct pointer types" warning, so long as
// the warning hasn't been disabled.

#define MDNS_BASE_CHECK(NAME, SUPER)															\
	check_compile_time(offsetof(MDNS_STRUCT(NAME), base) == 0);									\
	check_compile_time(sizeof_field(MDNS_STRUCT(NAME), base) == sizeof(MDNS_STRUCT(SUPER)));	\
	extern int _mdns_base_type_check[sizeof(&(((mdns_ ## NAME ## _t)0)->base) == ((mdns_ ## SUPER ## _t)0))]

#define MDNS_OBJECT_SUBKIND_DEFINE(NAME)														\
	static void																					\
	_mdns_ ## NAME ## _finalize(mdns_ ## NAME ## _t object);									\
																								\
	static char *																				\
	_mdns_ ## NAME ## _copy_description(mdns_ ## NAME ## _t object, bool debug, bool privacy);	\
																								\
	static const struct mdns_kind_s _mdns_ ## NAME ## _kind = {									\
		&_mdns_object_kind,																		\
		# NAME,																					\
		_mdns_ ## NAME ## _copy_description,													\
		_mdns_ ## NAME ## _finalize																\
	};																							\
																								\
	static mdns_ ## NAME ## _t																	\
	_mdns_ ## NAME ## _alloc(void)																\
	{																							\
		mdns_ ## NAME ## _t obj = mdns_object_ ## NAME ## _alloc(sizeof(*obj));					\
		require_quiet(obj, exit);																\
																								\
		const mdns_object_t base = (mdns_object_t)obj;											\
		base->kind = &_mdns_ ## NAME ## _kind;													\
																								\
	exit:																						\
		return obj;																				\
	}																							\
	MDNS_BASE_CHECK(NAME, object)

typedef char *	(*mdns_copy_description_f)(mdns_any_t object, bool debug, bool privacy);
typedef void	(*mdns_finalize_f)(mdns_any_t object);

typedef const struct mdns_kind_s *	mdns_kind_t;
struct mdns_kind_s {
	mdns_kind_t				superkind;			// This kind's superkind.
	const char *			name;				// Name of this kind.
	mdns_copy_description_f	copy_description;	// Creates a textual description of object.
	mdns_finalize_f			finalize;			// Releases object's resources right before the object is freed.
};

//======================================================================================================================
// MARK: - mdns_object Kind Definition

struct mdns_object_s {
	_OS_OBJECT_HEADER(const void *_os_obj_isa, _os_obj_refcnt, _os_obj_xref_cnt);
	mdns_kind_t	kind;	// Pointer to an object's kind.
};

static const struct mdns_kind_s _mdns_object_kind = {
	NULL,		// No superkind.
	"object",	// Name.
	NULL,		// No copy_description method.
	NULL		// No finalize method.
};

static const void *
_mdns_cf_collection_callback_retain(CFAllocatorRef allocator, const void *object);

static void
_mdns_cf_collection_callback_release(CFAllocatorRef allocator, const void *object);

static CFStringRef
_mdns_cf_collection_callback_copy_description(const void *object);

const CFArrayCallBacks mdns_cfarray_callbacks = {
	0,												// version
	_mdns_cf_collection_callback_retain,			// retain
	_mdns_cf_collection_callback_release,			// release
	_mdns_cf_collection_callback_copy_description,	// copy description
	NULL											// equal (NULL for pointer equality)
};

//======================================================================================================================
// MARK: - mdns_interface_monitor Kind Definition

struct mdns_interface_monitor_s {
	struct mdns_object_s					base;					// Object base.
	mdns_interface_monitor_t				next;					// Next monitor in list.
	dispatch_queue_t						user_queue;				// User's queue for invoking handlers.
	nw_path_evaluator_t						path_evaluator;			// Path evaluator for interface properties.
	dispatch_source_t						update_source;			// Data source for triggering user's update handler.
	mdns_interface_monitor_update_handler_t	update_handler;			// User's update handler.
	mdns_event_handler_t					event_handler;			// User's event handler.
	char *									ifname;					// Name of monitored interface.
	uint32_t								ifindex;				// Index of monitored interface.
	mdns_interface_flags_t					pending_flags;			// The latest interface flags from path updates.
	mdns_interface_flags_t					flags;					// The current interface flags made known to user.
	bool									user_activated;			// True if user called activate method.
	bool									activated;				// True if the monitor has been activated.
	bool									invalidated;			// True if the monitor has been invalidated.
	bool									path_evaluator_started;	// True if the path evaluator has been started.
};

MDNS_OBJECT_SUBKIND_DEFINE(interface_monitor);

//======================================================================================================================
// MARK: - Local Prototypes

static dispatch_queue_t
_mdns_internal_queue(void);

static dispatch_queue_t
_mdns_nwi_state_mutex_queue(void);

static void
_mdns_interface_monitor_activate_async(mdns_interface_monitor_t monitor);

static void
_mdns_interface_monitor_terminate(mdns_interface_monitor_t me, const OSStatus error);

static mdns_interface_flags_t
_mdns_get_interface_flags_from_nw_path(nw_path_t path, mdns_interface_flags_t current_flags);

static mdns_interface_flags_t
_mdns_get_interface_flags_from_nwi_state(const char *ifname, mdns_interface_flags_t current_flags);

static int
_mdns_snprintf_add(char **ptr, const char *lim, const char *fmt, ...);

static void
_mdns_start_nwi_state_monitoring(void);

#if !defined(nw_forget)
	#define nw_forget(X)	ForgetCustom(X, nw_release)
#endif

#if !defined(nw_release_null_safe)
	#define nw_release_null_safe(X)	do { if (X) { nw_release(X); } } while (0)
#endif

#if !defined(nwi_state_release_null_safe)
	#define nwi_state_release_null_safe(X)	do { if (X) { nwi_state_release(X); } } while (0)
#endif

//======================================================================================================================
// MARK: - Globals

static mdns_interface_monitor_t	g_monitor_list	= NULL;
static nwi_state_t				g_nwi_state		= NULL;

//======================================================================================================================
// MARK: - Internals

static dispatch_queue_t
_mdns_internal_queue(void)
{
	static dispatch_once_t	s_once	= 0;
	static dispatch_queue_t	s_queue	= NULL;
	dispatch_once(&s_once,
	^{
		s_queue = dispatch_queue_create("com.apple.mdns.internal_queue", DISPATCH_QUEUE_SERIAL);
	});
	return s_queue;	
}

//======================================================================================================================

static dispatch_queue_t
_mdns_nwi_state_mutex_queue(void)
{
	static dispatch_once_t	s_once	= 0;
	static dispatch_queue_t	s_queue	= NULL;
	dispatch_once(&s_once,
	^{
		s_queue = dispatch_queue_create("com.apple.mdns.nwi_state_mutex", DISPATCH_QUEUE_SERIAL);
	});
	return s_queue;
}

//======================================================================================================================

#define MDNS_LOG_CATEGORY_DEFINE(SHORT_NAME, CATEGORY_STR)			\
	static os_log_t													\
	_mdns_ ## SHORT_NAME ## _log(void)								\
	{																\
		static dispatch_once_t	s_once	= 0;						\
		static os_log_t			s_log	= NULL;						\
		dispatch_once(&s_once,										\
		^{															\
			s_log = os_log_create("com.apple.mdns", CATEGORY_STR);	\
		});															\
		return s_log;												\
	}																\
	extern int _mdns_dummy_variable

MDNS_LOG_CATEGORY_DEFINE(ifmon, "interface_monitor");
MDNS_LOG_CATEGORY_DEFINE(nwi,   "NWI");

//======================================================================================================================
// MARK: - mdns_object Public Methods

void
mdns_retain(mdns_any_t object)
{
	os_retain(object.base);
}

//======================================================================================================================

void
mdns_release(mdns_any_t object)
{
	os_release(object.base);
}

//======================================================================================================================

char *
mdns_copy_description(mdns_any_t object)
{
	return mdns_object_copy_description(object, false, false);
}

//======================================================================================================================
// MARK: - mdns_object Private Methods

char *
mdns_object_copy_description(mdns_any_t object, bool debug, bool privacy)
{
	for (mdns_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
		if (kind->copy_description) {
			return kind->copy_description(object, debug, privacy);
		}
	}
	return NULL;
}

//======================================================================================================================

CFStringRef
mdns_object_copy_description_as_cfstring(mdns_any_t object, bool debug, bool privacy)
{
	CFStringRef description = NULL;
	char *cstring = mdns_object_copy_description(object, debug, privacy);
	require_quiet(cstring, exit);

	description = CFStringCreateWithCStringNoCopy(NULL, cstring, kCFStringEncodingUTF8, kCFAllocatorMalloc);
	require_quiet(description, exit);
	cstring = NULL;

exit:
	FreeNullSafe(cstring);
	return description;
}

//======================================================================================================================

void
mdns_object_finalize(mdns_any_t object)
{
	for (mdns_kind_t kind = object.base->kind; kind; kind = kind->superkind) {
		if (kind->finalize) {
			kind->finalize(object);
		}
	}
}

//======================================================================================================================

static const void *
_mdns_cf_collection_callback_retain(__unused CFAllocatorRef allocator, const void *object)
{
	mdns_retain((mdns_object_t)object);
	return object;
}

//======================================================================================================================

static void
_mdns_cf_collection_callback_release(__unused CFAllocatorRef allocator, const void *object)
{
	mdns_release((mdns_object_t)object);
}

//======================================================================================================================

static CFStringRef
_mdns_cf_collection_callback_copy_description(const void *object)
{
	return mdns_object_copy_description_as_cfstring((mdns_object_t)object, false, false);
}

//======================================================================================================================
// MARK: - mdns_interface_monitor Public Methods

mdns_interface_monitor_t
mdns_interface_monitor_create(uint32_t interface_index)
{
	mdns_interface_monitor_t	monitor		= NULL;
	nw_interface_t				interface	= NULL;
	nw_parameters_t				params		= NULL;

	mdns_interface_monitor_t obj = _mdns_interface_monitor_alloc();
	require_quiet(obj, exit);

	obj->ifindex = interface_index;
	char ifname[IF_NAMESIZE + 1];
	if (if_indextoname(obj->ifindex, ifname) == NULL) {
		os_log_error(_mdns_ifmon_log(), "if_indextoname returned NULL for index %u", obj->ifindex);
		goto exit;
	}
	obj->ifname = strdup(ifname);
	require_quiet(obj->ifname, exit);

	interface = nw_interface_create_with_index(obj->ifindex);
	if (!interface) {
		os_log_error(_mdns_ifmon_log(), "nw_interface_create_with_index returned NULL for index %u", obj->ifindex);
		goto exit;
	}

	params = nw_parameters_create();
	require_quiet(params, exit);

	nw_parameters_require_interface(params, interface);
	obj->path_evaluator = nw_path_create_evaluator_for_endpoint(NULL, params);
	if (!obj->path_evaluator) {
		os_log_error(_mdns_ifmon_log(), "nw_path_create_evaluator_for_endpoint returned NULL for params: %@", params);
		goto exit;
	}

	nw_path_t path = nw_path_evaluator_copy_path(obj->path_evaluator);
	require_quiet(path, exit);

	obj->pending_flags = _mdns_get_interface_flags_from_nw_path(path, mdns_interface_flag_null);
	obj->pending_flags = _mdns_get_interface_flags_from_nwi_state(obj->ifname, obj->pending_flags);
	obj->flags = obj->pending_flags;
	nw_forget(&path);

	monitor = obj;
	obj = NULL;

exit:
	if (obj) {
		mdns_release(obj);
	}
	nw_release_null_safe(interface);
	nw_release_null_safe(params);
	return monitor;
}

//======================================================================================================================

void
mdns_interface_monitor_activate(mdns_interface_monitor_t me)
{
	if (!me->user_activated) {
		if (me->user_queue) {
			_mdns_interface_monitor_activate_async(me);
		}
		me->user_activated = true;
	}
}

//======================================================================================================================

void
mdns_interface_monitor_invalidate(mdns_interface_monitor_t me)
{
	mdns_retain(me);
	dispatch_async(_mdns_internal_queue(),
	^{
		if (!me->invalidated) {
			_mdns_interface_monitor_terminate(me, kNoErr);
			me->invalidated = true;
		}
		mdns_release(me);
	});
}

//======================================================================================================================

void
mdns_interface_monitor_set_queue(mdns_interface_monitor_t me, dispatch_queue_t queue)
{
	if (!me->user_activated) {
		dispatch_retain(queue);
		dispatch_release_null_safe(me->user_queue);
		me->user_queue = queue;
	} else if (!me->user_queue) {
		me->user_queue = queue;
		dispatch_retain(me->user_queue);
		_mdns_interface_monitor_activate_async(me);
	}
}

//======================================================================================================================

void
mdns_interface_monitor_set_event_handler(mdns_interface_monitor_t me, mdns_event_handler_t handler)
{
	mdns_event_handler_t const new_handler = handler ? Block_copy(handler) : NULL;
	if (me->event_handler) {
		Block_release(me->event_handler);
	}
	me->event_handler = new_handler;
}

//======================================================================================================================

void
mdns_interface_monitor_set_update_handler(mdns_interface_monitor_t me, mdns_interface_monitor_update_handler_t handler)
{
	mdns_interface_monitor_update_handler_t const new_handler = handler ? Block_copy(handler) : NULL;
	if (me->update_handler) {
		Block_release(me->update_handler);
	}
	me->update_handler = new_handler;
}

//======================================================================================================================

uint32_t
mdns_interface_monitor_get_interface_index(mdns_interface_monitor_t me)
{
	return me->ifindex;
}

//======================================================================================================================

bool
mdns_interface_monitor_has_ipv4_connectivity(mdns_interface_monitor_t me)
{
	return ((me->flags & mdns_interface_flag_ipv4_connectivity) ? true : false);
}

//======================================================================================================================

bool
mdns_interface_monitor_has_ipv6_connectivity(mdns_interface_monitor_t me)
{
	return ((me->flags & mdns_interface_flag_ipv6_connectivity) ? true : false);
}

//======================================================================================================================

bool
mdns_interface_monitor_is_expensive(mdns_interface_monitor_t me)
{
	return ((me->flags & mdns_interface_flag_expensive) ? true : false);
}

//======================================================================================================================

bool
mdns_interface_monitor_is_constrained(mdns_interface_monitor_t me)
{
	return ((me->flags & mdns_interface_flag_constrained) ? true : false);
}

//======================================================================================================================

bool
mdns_interface_monitor_is_clat46(mdns_interface_monitor_t me)
{
	return ((me->flags & mdns_interface_flag_clat46) ? true : false);
}

//======================================================================================================================
// MARK: - mdns_interface_monitor Private Methods

typedef struct {
	mdns_interface_flags_t	flag;
	const char *			desc;
} mdns_interface_flag_description_t;

const mdns_interface_flag_description_t	mdns_interface_flag_descriptions[] = {
	{ mdns_interface_flag_ipv4_connectivity, "IPv4" },
	{ mdns_interface_flag_ipv6_connectivity, "IPv6" },
	{ mdns_interface_flag_expensive,         "expensive" },
	{ mdns_interface_flag_constrained,       "constrained" },
	{ mdns_interface_flag_clat46,            "CLAT46" }
};

static char *
_mdns_interface_monitor_copy_description(mdns_interface_monitor_t me, const bool debug, __unused const bool privacy)
{
	char *				description	= NULL;
	char				buffer[128];
	char *				dst			= buffer;
	const char * const	lim			= &buffer[countof(buffer)];
	int					n;

	*dst = '\0';
	if (debug) {
		n = _mdns_snprintf_add(&dst, lim, "mdns_%s (%p): ", me->base.kind->name, me);
		require_quiet(n >= 0, exit);
	}
	n = _mdns_snprintf_add(&dst, lim, "interface %s (%u): ", me->ifname, me->ifindex);
	require_quiet(n >= 0, exit);

	const char *separator = "";
	for (size_t i = 0; i < countof(mdns_interface_flag_descriptions); ++i) {
		const mdns_interface_flag_description_t * const flag_desc = &mdns_interface_flag_descriptions[i];
		if (me->flags & flag_desc->flag) {
			n = _mdns_snprintf_add(&dst, lim, "%s%s", separator, flag_desc->desc);
			require_quiet(n >= 0, exit);
			separator = ", ";
		}
	}
	description = strdup(buffer);

exit:
	return description;
}

//======================================================================================================================

static void
_mdns_interface_monitor_finalize(mdns_interface_monitor_t me)
{
	dispatch_forget(&me->user_queue);
	nw_forget(&me->path_evaluator);
	BlockForget(&me->update_handler);
	BlockForget(&me->event_handler);
	ForgetMem(&me->ifname);
}

//======================================================================================================================

static void
_mdns_interface_monitor_activate_internal(mdns_interface_monitor_t monitor);

static void
_mdns_interface_monitor_activate_async(mdns_interface_monitor_t me)
{
	mdns_retain(me);
	dispatch_async(_mdns_internal_queue(),
	^{
		_mdns_interface_monitor_activate_internal(me);
		mdns_release(me);		
	});
}

static void
_mdns_interface_monitor_activate_internal(mdns_interface_monitor_t me)
{
	OSStatus err;
	require_action_quiet(!me->activated && !me->invalidated, exit, err = kNoErr);
	me->activated = true;

	me->update_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_REPLACE, 0, 0, me->user_queue);
	require_action_quiet(me->update_source, exit, err = kNoResourcesErr);

	mdns_retain(me);
	const dispatch_source_t update_source = me->update_source;
	dispatch_source_set_event_handler(me->update_source,
	^{
		const unsigned long data = dispatch_source_get_data(update_source);
		const mdns_interface_flags_t new_flags = ((mdns_interface_flags_t)data) & ~mdns_interface_flag_reserved;
		const mdns_interface_flags_t changed_flags = me->flags ^ new_flags;
		if (changed_flags != 0) {
			me->flags = new_flags;
			if (me->update_handler) {
				me->update_handler(changed_flags);
			}
		}
	});
	dispatch_source_set_cancel_handler(me->update_source,
	^{
		mdns_release(me);
	});
	dispatch_activate(me->update_source);

	mdns_retain(me);
	nw_path_evaluator_set_update_handler(me->path_evaluator, _mdns_internal_queue(),
	^(nw_path_t path)
	{
		const mdns_interface_flags_t new_flags = _mdns_get_interface_flags_from_nw_path(path, me->pending_flags);
		if (new_flags != me->pending_flags) {
			me->pending_flags = new_flags;
			if (me->update_source) {
				// Note: mdns_interface_flag_reserved is used to ensure that the data is non-zero. According to the
				// dispatch_source_create(3) man page, if the data value is zero, the source handler won't be invoked.
				dispatch_source_merge_data(me->update_source, me->pending_flags | mdns_interface_flag_reserved);
			}
		}
	});
	nw_path_evaluator_set_cancel_handler(me->path_evaluator,
	^{
		mdns_release(me);
	});
	nw_path_evaluator_start(me->path_evaluator);
	me->path_evaluator_started = true;

	mdns_interface_monitor_t *p = &g_monitor_list;
	while (*p != NULL) {
		p = &(*p)->next;
	}
	mdns_retain(me);
	*p = me;

	// This is called after adding the monitor to the global list to ensure that the initial NWI state check is aware
	// that the interface monitor exists.
	_mdns_start_nwi_state_monitoring();
	err = kNoErr;

exit:
	if (err) {
		_mdns_interface_monitor_terminate(me, err);
	}
}

//======================================================================================================================

static void
_mdns_interface_monitor_terminate(mdns_interface_monitor_t me, const OSStatus error)
{
	dispatch_source_forget(&me->update_source);
	if (me->path_evaluator) {
		if (me->path_evaluator_started) {
			nw_path_evaluator_cancel(me->path_evaluator);
		}
		nw_forget(&me->path_evaluator);
	}
	for (mdns_interface_monitor_t *p = &g_monitor_list; *p; p = &(*p)->next) {
		if (*p == me) {
			*p = me->next;
			me->next = NULL;
			mdns_release(me);
			break;
		}
	}
	mdns_retain(me);
	dispatch_async(me->user_queue,
	^{
		if (me->event_handler) {
			me->event_handler(error ? mdns_event_error : mdns_event_invalidated, error);
		}
		mdns_release(me);
	});
}

//======================================================================================================================
// MARK: - NW Path Helpers

#define MDNS_INTERFACE_FLAGS_FROM_NWPATH		\
	(mdns_interface_flag_ipv4_connectivity |	\
	 mdns_interface_flag_ipv6_connectivity |	\
	 mdns_interface_flag_expensive         |	\
	 mdns_interface_flag_constrained)

static mdns_interface_flags_t
_mdns_get_interface_flags_from_nw_path(nw_path_t path, mdns_interface_flags_t current_flags)
{
	mdns_interface_flags_t flags = current_flags & ~MDNS_INTERFACE_FLAGS_FROM_NWPATH;
	if (nw_path_has_ipv4(path)) {
		flags |= mdns_interface_flag_ipv4_connectivity;
	}
	if (nw_path_has_ipv6(path)) {
		flags |= mdns_interface_flag_ipv6_connectivity;
	}
	if (nw_path_is_expensive(path)) {
		flags |= mdns_interface_flag_expensive;
	}
	if (__builtin_available(macOS 10.15, *)) {
		if (nw_path_is_constrained(path)) {
			flags |= mdns_interface_flag_constrained;
		}
	}
	return flags;
}

//======================================================================================================================
// MARK: - NWI Helpers

#if !defined(NWI_IFSTATE_FLAGS_HAS_CLAT46)
	#define NWI_IFSTATE_FLAGS_HAS_CLAT46	0x0040
#endif

#define MDNS_INTERFACE_FLAGS_FROM_NWI_STATE		mdns_interface_flag_clat46

static mdns_interface_flags_t
_mdns_get_interface_flags_from_nwi_state(const char *ifname, mdns_interface_flags_t current_flags)
{
	__block nwi_ifstate_flags ifstate_flags = 0;
	dispatch_sync(_mdns_nwi_state_mutex_queue(),
	^{
		if (g_nwi_state) {
			const nwi_ifstate_t ifstate = nwi_state_get_ifstate(g_nwi_state, ifname);
			if (ifstate) {
				ifstate_flags = nwi_ifstate_get_flags(ifstate);
			}
		}
	});
	mdns_interface_flags_t flags = current_flags & ~MDNS_INTERFACE_FLAGS_FROM_NWI_STATE;
	if (ifstate_flags & NWI_IFSTATE_FLAGS_HAS_CLAT46) {
		flags |= mdns_interface_flag_clat46;
	}
	return flags;
}

//======================================================================================================================

static void
_mdns_nwi_state_update(void);

static void
_mdns_start_nwi_state_monitoring(void)
{
	static int s_nwi_notify_token = NOTIFY_TOKEN_INVALID;
	if (s_nwi_notify_token == NOTIFY_TOKEN_INVALID) {
		const uint32_t status = notify_register_dispatch(nwi_state_get_notify_key(), &s_nwi_notify_token,
			_mdns_internal_queue(),
		^(__unused int token)
		{
			_mdns_nwi_state_update();
		});
		if (s_nwi_notify_token == NOTIFY_TOKEN_INVALID) {
			os_log_error(_mdns_nwi_log(), "Failed to register for NWI state notifications (status %u)", status);
		} else {
			_mdns_nwi_state_update();
		}
	}
}

static void
_mdns_nwi_state_update(void)
{
	nwi_state_t new_state = nwi_state_copy();
	if (!new_state) {
		os_log_error(_mdns_nwi_log(), "Failed to copy NWI state");
	}
	__block nwi_state_t old_state;
	dispatch_sync(_mdns_nwi_state_mutex_queue(),
	^{
		old_state	= g_nwi_state;
		g_nwi_state	= new_state;
	});
	nwi_state_release_null_safe(old_state);
	for (mdns_interface_monitor_t m = g_monitor_list; m; m = m->next) {
		const mdns_interface_flags_t new_flags = _mdns_get_interface_flags_from_nwi_state(m->ifname, m->pending_flags);
		if (new_flags != m->pending_flags) {
			m->pending_flags = new_flags;
			if (m->update_source) {
				// Note: mdns_interface_flag_reserved is used to ensure that the data is non-zero. According to the
				// dispatch_source_create(3) man page, if the data value is zero, the source handler won't be invoked.
				dispatch_source_merge_data(m->update_source, m->pending_flags | mdns_interface_flag_reserved);
			}
		}
	}
}

//======================================================================================================================
// MARK: - General Helpers

static int
_mdns_snprintf_add(char **ptr, const char *lim, const char *fmt, ...)
{
	char * const	dst = *ptr;
	const size_t	len = (size_t)(lim - dst);
	int				n;

	require_action_quiet(len > 0, exit, n = 0);

	va_list args;
	va_start(args, fmt);
	n = vsnprintf(dst, len, fmt, args);
	va_end(args);
	require_quiet(n >= 0, exit);

	if (((size_t)n) > len) {
		n = (int)len;
	}
	*ptr = dst + n;

exit:
	return n;
}
