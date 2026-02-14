/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
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
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __DISPATCH_EVENT_EVENT_CONFIG__
#define __DISPATCH_EVENT_EVENT_CONFIG__

#if defined(__linux__)
#	include <sys/eventfd.h>
#	define DISPATCH_EVENT_BACKEND_EPOLL 1
#	define DISPATCH_EVENT_BACKEND_KEVENT 0
#	define DISPATCH_EVENT_BACKEND_WINDOWS 0
#elif __has_include(<sys/event.h>)
#	include <sys/event.h>
#	define DISPATCH_EVENT_BACKEND_EPOLL 0
#	define DISPATCH_EVENT_BACKEND_KEVENT 1
#	define DISPATCH_EVENT_BACKEND_WINDOWS 0
#elif defined(_WIN32)
#	define DISPATCH_EVENT_BACKEND_EPOLL 0
#	define DISPATCH_EVENT_BACKEND_KEVENT 0
#	define DISPATCH_EVENT_BACKEND_WINDOWS 1
#else
#	error unsupported event loop
#endif

#if DISPATCH_DEBUG
#define DISPATCH_MGR_QUEUE_DEBUG 1
#define DISPATCH_WLH_DEBUG 1
#endif

#ifndef DISPATCH_MGR_QUEUE_DEBUG
#define DISPATCH_MGR_QUEUE_DEBUG 0
#endif

#ifndef DISPATCH_WLH_DEBUG
#define DISPATCH_WLH_DEBUG 0
#endif

#ifndef DISPATCH_MACHPORT_DEBUG
#define DISPATCH_MACHPORT_DEBUG 0
#endif

#ifndef DISPATCH_TIMER_ASSERTIONS
#if DISPATCH_DEBUG
#define DISPATCH_TIMER_ASSERTIONS 1
#else
#define DISPATCH_TIMER_ASSERTIONS 0
#endif
#endif

#if DISPATCH_TIMER_ASSERTIONS
#define DISPATCH_TIMER_ASSERT(a, op, b, text) ({ \
		__typeof__(a) _a = (a); \
		if (unlikely(!(_a op (b)))) { \
			DISPATCH_CLIENT_CRASH(_a, "Timer: " text); \
		} \
	})
#else
#define DISPATCH_TIMER_ASSERT(a, op, b, text) ((void)0)
#endif

#ifndef EV_VANISHED
#define EV_VANISHED 0x0200
#endif

#if DISPATCH_EVENT_BACKEND_KEVENT
#	if defined(EV_UDATA_SPECIFIC) && EV_UDATA_SPECIFIC
#		define DISPATCH_HAVE_DIRECT_KNOTES 1
#	else
#		define DISPATCH_HAVE_DIRECT_KNOTES 0
#	endif

#	if defined(EV_SET_QOS)
#		define DISPATCH_USE_KEVENT_QOS 1
#	else
#		define DISPATCH_USE_KEVENT_QOS 0
#	endif

#	ifndef KEVENT_FLAG_ERROR_EVENTS
#		define KEVENT_FLAG_ERROR_EVENTS 0x002
#	endif

#	ifdef NOTE_LEEWAY
#		define DISPATCH_HAVE_TIMER_COALESCING 1
#   else
#		define NOTE_LEEWAY 0
#		define DISPATCH_HAVE_TIMER_COALESCING 0
#	endif // !NOTE_LEEWAY
#	if defined(NOTE_CRITICAL) && defined(NOTE_BACKGROUND)
#		define DISPATCH_HAVE_TIMER_QOS 1
#	else
#		undef  NOTE_CRITICAL
#		define NOTE_CRITICAL 0
#		undef  NOTE_BACKGROUND
#		define NOTE_BACKGROUND 0
#		define DISPATCH_HAVE_TIMER_QOS 0
#	endif // !defined(NOTE_CRITICAL) || !defined(NOTE_BACKGROUND)

#	ifndef NOTE_FUNLOCK
#	define NOTE_FUNLOCK 0x00000100
#	endif

// FreeBSD's kevent does not support those
#	ifndef NOTE_ABSOLUTE
#	define NOTE_ABSOLUTE 0
#	endif
#	ifndef NOTE_EXITSTATUS
#	define NOTE_EXITSTATUS 0
#	endif

#	if HAVE_DECL_NOTE_REAP
#	if defined(NOTE_REAP) && defined(__APPLE__)
#	undef NOTE_REAP
#	define NOTE_REAP 0x10000000 // <rdar://problem/13338526>
#	endif
#	endif // HAVE_DECL_NOTE_REAP

#	ifndef VQ_QUOTA
#	undef HAVE_DECL_VQ_QUOTA // rdar://problem/24160982
#	endif // VQ_QUOTA

#	ifndef VQ_NEARLOWDISK
#	undef HAVE_DECL_VQ_NEARLOWDISK
#	endif // VQ_NEARLOWDISK

#	ifndef VQ_DESIRED_DISK
#	undef HAVE_DECL_VQ_DESIRED_DISK
#	endif // VQ_DESIRED_DISK

#	ifndef VQ_FREE_SPACE_CHANGE
#	undef HAVE_DECL_VQ_FREE_SPACE_CHANGE
#	endif // VQ_FREE_SPACE_CHANGE

#	if !defined(EVFILT_NW_CHANNEL) && defined(__APPLE__)
#	define EVFILT_NW_CHANNEL		(-16)
#	define NOTE_FLOW_ADV_UPDATE		0x1
#	define NOTE_CHANNEL_EVENT		0x2
#	define NOTE_IF_ADV_UPD			0x4
#	endif
#else // DISPATCH_EVENT_BACKEND_KEVENT
#	define EV_ADD					0x0001
#	define EV_DELETE				0x0002
#	define EV_ENABLE				0x0004

#	define EV_ONESHOT				0x0010
#	define EV_CLEAR					0x0020
#	define EV_DISPATCH				0x0080

#	define EVFILT_READ				(-1)
#	define EVFILT_WRITE				(-2)
#	define EVFILT_SIGNAL			(-3)
#	define EVFILT_TIMER				(-4)
#	define EVFILT_SYSCOUNT			4

#	define DISPATCH_HAVE_TIMER_QOS 0
#	define DISPATCH_HAVE_TIMER_COALESCING 0
#	define DISPATCH_HAVE_DIRECT_KNOTES 0
#endif // !DISPATCH_EVENT_BACKEND_KEVENT

// These flags are used by dispatch generic code and
// translated back by the various backends to similar semantics
// hence must be defined even on non Darwin platforms
#ifndef KEVENT_FLAG_IMMEDIATE
#	define KEVENT_FLAG_IMMEDIATE 0x001
#endif

#ifdef EV_UDATA_SPECIFIC
#	define DISPATCH_EV_DIRECT		(EV_UDATA_SPECIFIC|EV_DISPATCH)
#else
#	define DISPATCH_EV_DIRECT		0x0000
#	define EV_UDATA_SPECIFIC		0x0000
#	undef  EV_VANISHED
#	define EV_VANISHED				0x0000
#endif

#define DISPATCH_EV_MSG_NEEDS_FREE	0x10000 // mach message needs to be freed()

#define DISPATCH_EVFILT_TIMER				(-EVFILT_SYSCOUNT - 1)
#define DISPATCH_EVFILT_TIMER_WITH_CLOCK	(-EVFILT_SYSCOUNT - 2)
#define DISPATCH_EVFILT_CUSTOM_ADD			(-EVFILT_SYSCOUNT - 3)
#define DISPATCH_EVFILT_CUSTOM_OR			(-EVFILT_SYSCOUNT - 4)
#define DISPATCH_EVFILT_CUSTOM_REPLACE		(-EVFILT_SYSCOUNT - 5)
#define DISPATCH_EVFILT_MACH_NOTIFICATION	(-EVFILT_SYSCOUNT - 6)

#if HAVE_MACH
#	if !EV_UDATA_SPECIFIC
#	error mach support requires EV_UDATA_SPECIFIC
#	endif

#	ifndef MACH_RCV_VOUCHER
#	define MACH_RCV_VOUCHER 0x00000800
#	endif

#	ifndef MACH_NOTIFY_SEND_POSSIBLE
#	undef  MACH_NOTIFY_SEND_POSSIBLE
#	define MACH_NOTIFY_SEND_POSSIBLE MACH_NOTIFY_DEAD_NAME
#	endif

#	ifndef NOTE_MACH_CONTINUOUS_TIME
#	define NOTE_MACH_CONTINUOUS_TIME 0
#	endif // NOTE_MACH_CONTINUOUS_TIME

#	ifndef HOST_NOTIFY_CALENDAR_SET
#	define HOST_NOTIFY_CALENDAR_SET HOST_NOTIFY_CALENDAR_CHANGE
#	endif // HOST_NOTIFY_CALENDAR_SET

#	ifndef HOST_CALENDAR_SET_REPLYID
#	define HOST_CALENDAR_SET_REPLYID 951
#	endif // HOST_CALENDAR_SET_REPLYID

#	ifndef MACH_SEND_OVERRIDE
#	define MACH_SEND_OVERRIDE 0x00000020
typedef unsigned int mach_msg_priority_t;
#	define MACH_MSG_PRIORITY_UNSPECIFIED ((mach_msg_priority_t)0)
#	endif // MACH_SEND_OVERRIDE

#	ifndef MACH_SEND_SYNC_OVERRIDE
#	define MACH_SEND_SYNC_OVERRIDE 0x00100000
#	endif // MACH_SEND_SYNC_OVERRIDE

#	ifndef MACH_MSG_STRICT_REPLY
#	define MACH_MSG_STRICT_REPLY  0x00000200
#	endif

#	ifndef MACH_RCV_SYNC_WAIT
#	define MACH_RCV_SYNC_WAIT 0x00004000
#	endif // MACH_RCV_SYNC_WAIT

#	define DISPATCH_MACH_TRAILER_SIZE sizeof(dispatch_mach_trailer_t)
#	define DISPATCH_MACH_RCV_TRAILER MACH_RCV_TRAILER_CTX
#	define DISPATCH_MACH_RCV_OPTIONS ( \
		MACH_RCV_MSG | MACH_RCV_LARGE | MACH_RCV_LARGE_IDENTITY | \
		MACH_RCV_TRAILER_ELEMENTS(DISPATCH_MACH_RCV_TRAILER) | \
		MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | \
		MACH_RCV_VOUCHER)
#endif // HAVE_MACH

#endif // __DISPATCH_EVENT_EVENT_CONFIG__
