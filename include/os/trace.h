/*
 * Copyright (c) 2013-2014 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef os_trace_h
#define os_trace_h

#include <inttypes.h>
#include <os/base.h>
#include <sys/types.h>
#include <stdbool.h>
#if __has_include(<xpc/xpc.h>)
#include <xpc/xpc.h>
#else 
typedef void *xpc_object_t;
#endif

#if !__GNUC__
#error "must be GNU C compatible"
#endif

extern void *__dso_handle;

OS_ALWAYS_INLINE __attribute__((format(printf,1,2)))
static inline void _os_trace_verify_printf(const char *msg, ...) { (void) msg; }

#if !defined OS_COUNT_ARGS
#define OS_COUNT_ARGS(...) OS_COUNT_ARGS1(,##__VA_ARGS__,_8,_7,_6,_5,_4,_3,_2,_1,_0)
#define OS_COUNT_ARGS1(z,a,b,c,d,e,f,g,h,cnt,...) cnt
#endif

#define _os_trace_0(_m, _t) __extension__({ \
	_os_trace_verify_printf(_m); \
	_os_trace_with_buffer(&__dso_handle, _m, _t, NULL, 0, NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_1(_m, _t, _1) __extension__({ \
	const typeof(_1) _c1 = _1; \
	_os_trace_verify_printf(_m, _c1); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
	} _buf = { \
		._f1 = _c1, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_2(_m, _t, _1, _2) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	_os_trace_verify_printf(_m, _c1, _c2); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_3(_m, _t, _1, _2, _3) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_4(_m, _t, _1, _2, _3, _4) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_5(_m, _t, _1, _2, _3, _4, _5) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_5) _c5 = _5; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_6(_m, _t, _1, _2, _3, _4, _5, _6) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_5) _c5 = _5; \
	const typeof(_6) _c6 = _6; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5, _c6); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
		typeof(_c6) _f6; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
		._f6 = _c6, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_7(_m, _t, _1, _2, _3, _4, _5, _6, _7) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_5) _c5 = _5; \
	const typeof(_6) _c6 = _6; \
	const typeof(_7) _c7 = _7; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5, _c6, _c7); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
		typeof(_c6) _f6; \
		typeof(_c7) _f7; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
		._f6 = _c6, \
		._f7 = _c7, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_1(_m, _t, _payload) __extension__({ \
	_os_trace_with_buffer(&__dso_handle, _m, _t, NULL, 0, _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_2(_m, _t, _1, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	_os_trace_verify_printf(_m, _c1); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
	} _buf = { \
		._f1 = _c1, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_3(_m, _t, _1, _2, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	_os_trace_verify_printf(_m, _c1, _c2); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_4(_m, _t, _1, _2, _3, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_5(_m, _t, _1, _2, _3, _4, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_6(_m, _t, _1, _2, _3, _4, _5, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_4) _c5 = _5; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_7(_m, _t, _1, _2, _3, _4, _5, _6, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_5) _c5 = _5; \
	const typeof(_6) _c6 = _6; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5, _c6); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
		typeof(_c6) _f6; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
		._f6 = _c6, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_8(_m, _t, _1, _2, _3, _4, _5, _6, _7, _payload) __extension__({ \
	const typeof(_1) _c1 = _1; \
	const typeof(_2) _c2 = _2; \
	const typeof(_3) _c3 = _3; \
	const typeof(_4) _c4 = _4; \
	const typeof(_5) _c5 = _5; \
	const typeof(_6) _c6 = _6; \
	const typeof(_7) _c7 = _7; \
	_os_trace_verify_printf(_m, _c1, _c2, _c3, _c4, _c5, _c6, _c7); \
	const struct __attribute__((packed)) { \
		typeof(_c1) _f1; \
		typeof(_c2) _f2; \
		typeof(_c3) _f3; \
		typeof(_c4) _f4; \
		typeof(_c5) _f5; \
		typeof(_c6) _f6; \
		typeof(_c7) _f7; \
	} _buf = { \
		._f1 = _c1, \
		._f2 = _c2, \
		._f3 = _c3, \
		._f4 = _c4, \
		._f5 = _c5, \
		._f6 = _c6, \
		._f7 = _c7, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	asm(""); /* avoid tailcall */ \
})

#pragma mark - Other defines

/*!
 * @define OS_TRACE_TYPE_RELEASE
 * Trace messages to be recorded on a typical user install.  These should be
 * limited to things which improve diagnosis of a failure/crash/hang. Trace
 * buffers are generally smaller on a production system.
 */
#define OS_TRACE_TYPE_RELEASE (1u << 0)

/*!
 * @define OS_TRACE_TYPE_DEBUG
 * Trace messages to be recorded while debugger or other development tool is
 * attached to the originator.  This is transported interprocess.
 */
#define OS_TRACE_TYPE_DEBUG (1u << 1)

/*!
 * @define OS_TRACE_TYPE_ERROR
 * Trace the message as an error and force a collection as a failure may be
 * imminent.
 */
#define OS_TRACE_TYPE_ERROR ((1u << 6) | (1u << 0))

/*!
 * @define OS_TRACE_TYPE_FAULT
 * Trace the message as a fatal error which forces a collection and a diagnostic
 * to be initiated.
 */
#define OS_TRACE_TYPE_FAULT ((1u << 7) | (1u << 6) | (1u << 0))

__BEGIN_DECLS

/*!
 * @typedef os_trace_payload_t
 * A block that populates an xpc_object_t of type XPC_TYPE_DICTIONARY to represent
 * complex data. This block will only be invoked under conditions where tools
 * have attached to the process. The payload can be used to send arbitrary data 
 * via the trace call. Tools may use the data to validate state for integration 
 * tests or provide other introspection services. No assumptions are made about 
 * the format or structure of the data.
 */
typedef void (^os_trace_payload_t)(xpc_object_t xdict);

#pragma mark - function declarations

/*!
 * @function os_trace
 *
 * @abstract
 * Insert a trace message into a ring-buffer for later decoding.
 *
 * @discussion
 * Trace message that will be recorded on a typical user install. These should
 * be limited to things which help diagnose a failure during postmortem
 * analysis. Trace buffers are generally smaller on a production system.
 *
 * @param logmsg
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace("network event: %ld, last seen: %ld, avg: %g", event_id, last_seen, avg);
 */
#define os_trace(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_RELEASE, ##__VA_ARGS__); \
} while (0)

/*!
 * @function os_trace_debug
 *
 * @abstract
 * Insert a trace message into a ring-buffer for later decoding.
 *
 * @discussion
 * Trace message to be recorded while debugger or other development tool is
 * attached to the originator.  This is transported interprocess to help
 * diagnose the entire call chain including external helpers.
 *
 * @param logmsg
 * A printf-style format string that represents a human-readable message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_debug("network interface status %ld", status);
 */
#define os_trace_debug(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace_debug"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_DEBUG, ##__VA_ARGS__); \
} while (0)

/*!
 * @function os_trace_debug_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking if debug level is enabled.
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking if debug level is enabled.
 * Generally trace points should not involve expensive operations, but some
 * circumstances warrant it.  Use this function to avoid doing the work unless
 * debug level trace messages are requested.
 *
 *	if (os_trace_debug_enabled()) {
 *		os_trace_debug("value = %d, average = %d",
 *				[[dict objectForKey: @"myKey"] intValue],
 *				(int) [self getAverage: dict]);
 *	}
 *
 * @result
 * Returns true if debug mode is enabled.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_trace_debug_enabled(void);

/*!
 * @function os_trace_error
 *
 * @abstract
 * Trace the message as an error and force a collection of the trace buffer as a
 * failure may be imminent.
 *
 * @discussion
 * Trace the message as an error and force a collection of the trace buffer as a
 * failure may be imminent.
 *
 * @param logmsg
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_error("socket %d connection timeout %ld", fd, secs);
 */
#define os_trace_error(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_ERROR, ##__VA_ARGS__); \
} while (0)

/*!
 * @function os_trace_fault
 *
 * @abstract
 * Trace the message as a fault which forces a collection of the trace buffer
 * and diagnostic of the activity.
 *
 * @discussion
 * Trace the message as a fault which forces a collection of the trace buffer
 * and diagnostic of the activity.
 *
 * @param logmsg
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_fault("failed to lookup uid %d - aborting", uid);
 */
#define os_trace_fault(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_FAULT, ##__VA_ARGS__); \
} while (0)

#if __has_include(<xpc/xpc.h>)
/*!
 * @function os_trace_with_payload
 *
 * @abstract
 * Add a trace entry containing the provided values and call the block if
 * appropriate.
 *
 * @discussion
 * Will insert a trace entry into a limited ring buffer for an activity or
 * process.  Trace points are for recording interesting data that would improve
 * diagnosis of unexpected crashes, failures and hangs.  The block will only be
 * called under the required conditions.
 *
 * @param trace_msg
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported. Attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 * The final parameter must be a block of type os_trace_payload_t.
 *
 *   os_trace_with_payload("network event %ld", event, ^(xpc_object_t xdict) {
 *
 *		// validate the network interface and address where what was expected
 *		xpc_dictionary_set_string(xdict, "network", ifp->ifa_name);
 *		xpc_dictionary_set_string(xdict, "ip_address", _get_address(ifp));
 *   });
 */
#define os_trace_with_payload(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace_with_payload,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_RELEASE, ##__VA_ARGS__); \
} while (0)

#define os_trace_debug_with_payload(logmsg, ...) do { \
__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
OS_CONCAT(_os_trace_with_payload,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_DEBUG, ##__VA_ARGS__); \
} while (0)

#define os_trace_error_with_payload(logmsg, ...) do { \
__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
OS_CONCAT(_os_trace_with_payload,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_ERROR, ##__VA_ARGS__); \
} while (0)

#define os_trace_fault_with_payload(logmsg, ...) do { \
	__attribute__((unused)) char _verify_const_msg[__builtin_constant_p(logmsg) ? 1 : -1]; \
	__attribute__((section("__TEXT,__os_trace"))) static const char _m[] = logmsg; \
	OS_CONCAT(_os_trace_with_payload,OS_COUNT_ARGS(__VA_ARGS__))(_m, OS_TRACE_TYPE_FAULT, ##__VA_ARGS__); \
} while (0)

#endif // __has_include(<xpc/xpc.h>)

/*!
 * @function _os_trace_with_buffer
 *
 * @abstract
 * Internal function to support pre-encoded buffer.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0)
OS_EXPORT OS_NOTHROW
void
_os_trace_with_buffer(void *dso, const char *message, uint8_t type, const void *buffer, size_t buffer_size, os_trace_payload_t payload);

__END_DECLS

#endif
