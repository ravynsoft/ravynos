/*
 * Copyright (c) 2008-2016 Apple Inc. All rights reserved.
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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_PRIORITY__
#define __DISPATCH_SHIMS_PRIORITY__

#if HAVE_PTHREAD_QOS_H && __has_include(<pthread/qos_private.h>)
#include <pthread/qos.h>
#include <pthread/qos_private.h>
#ifndef _PTHREAD_PRIORITY_OVERCOMMIT_FLAG
#define _PTHREAD_PRIORITY_OVERCOMMIT_FLAG 0x80000000
#endif
#ifndef _PTHREAD_PRIORITY_SCHED_PRI_FLAG
#define _PTHREAD_PRIORITY_SCHED_PRI_FLAG 0x20000000
#endif
#ifndef _PTHREAD_PRIORITY_FALLBACK_FLAG
#define _PTHREAD_PRIORITY_FALLBACK_FLAG 0x04000000
#endif
#ifndef _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG
#define _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG 0x02000000
#endif
#ifndef _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG
#define _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG 0x01000000
#endif
#else // HAVE_PTHREAD_QOS_H
OS_ENUM(qos_class, unsigned int,
	QOS_CLASS_USER_INTERACTIVE = 0x21,
	QOS_CLASS_USER_INITIATED = 0x19,
	QOS_CLASS_DEFAULT = 0x15,
	QOS_CLASS_UTILITY = 0x11,
	QOS_CLASS_BACKGROUND = 0x09,
	QOS_CLASS_MAINTENANCE = 0x05,
	QOS_CLASS_UNSPECIFIED = 0x00,
);
typedef unsigned long pthread_priority_t;
#define QOS_MIN_RELATIVE_PRIORITY (-15)
#define _PTHREAD_PRIORITY_FLAGS_MASK (~0xffffff)
#define _PTHREAD_PRIORITY_QOS_CLASS_MASK 0x00ffff00
#define _PTHREAD_PRIORITY_QOS_CLASS_SHIFT (8ull)
#define _PTHREAD_PRIORITY_PRIORITY_MASK 0x000000ff
#define _PTHREAD_PRIORITY_OVERCOMMIT_FLAG 0x80000000
#define _PTHREAD_PRIORITY_SCHED_PRI_FLAG 0x20000000
#define _PTHREAD_PRIORITY_FALLBACK_FLAG 0x04000000
#define _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG 0x02000000
#define _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG 0x01000000
#define _PTHREAD_PRIORITY_ENFORCE_FLAG  0x10000000

#endif // HAVE_PTHREAD_QOS_H

typedef uint32_t dispatch_qos_t;
typedef uint32_t dispatch_priority_t;

#define DISPATCH_QOS_UNSPECIFIED        ((dispatch_qos_t)0)
#define DISPATCH_QOS_MAINTENANCE        ((dispatch_qos_t)1)
#define DISPATCH_QOS_BACKGROUND         ((dispatch_qos_t)2)
#define DISPATCH_QOS_UTILITY            ((dispatch_qos_t)3)
#define DISPATCH_QOS_DEFAULT            ((dispatch_qos_t)4)
#define DISPATCH_QOS_USER_INITIATED     ((dispatch_qos_t)5)
#define DISPATCH_QOS_USER_INTERACTIVE   ((dispatch_qos_t)6)
#define DISPATCH_QOS_MIN                DISPATCH_QOS_MAINTENANCE
#define DISPATCH_QOS_MAX                DISPATCH_QOS_USER_INTERACTIVE
#define DISPATCH_QOS_SATURATED          ((dispatch_qos_t)15)

#define DISPATCH_QOS_NBUCKETS           (DISPATCH_QOS_MAX - DISPATCH_QOS_MIN + 1)
#define DISPATCH_QOS_BUCKET(qos)        ((qos) - DISPATCH_QOS_MIN)

#define DISPATCH_PRIORITY_RELPRI_MASK        ((dispatch_priority_t)0x000000ff)
#define DISPATCH_PRIORITY_RELPRI_SHIFT       0
#define DISPATCH_PRIORITY_QOS_MASK           ((dispatch_priority_t)0x00000f00)
#define DISPATCH_PRIORITY_QOS_SHIFT          8
#define DISPATCH_PRIORITY_REQUESTED_MASK     ((dispatch_priority_t)0x00000fff)
#define DISPATCH_PRIORITY_FALLBACK_QOS_MASK  ((dispatch_priority_t)0x0000f000)
#define DISPATCH_PRIORITY_FALLBACK_QOS_SHIFT 12
#define DISPATCH_PRIORITY_OVERRIDE_MASK      ((dispatch_priority_t)0x000f0000)
#define DISPATCH_PRIORITY_OVERRIDE_SHIFT     16
#define DISPATCH_PRIORITY_FLAGS_MASK         ((dispatch_priority_t)0xff000000)

#define DISPATCH_PRIORITY_SATURATED_OVERRIDE DISPATCH_PRIORITY_OVERRIDE_MASK

#define DISPATCH_PRIORITY_FLAG_OVERCOMMIT    ((dispatch_priority_t)0x80000000) // _PTHREAD_PRIORITY_OVERCOMMIT_FLAG
#define DISPATCH_PRIORITY_FLAG_FALLBACK      ((dispatch_priority_t)0x04000000) // _PTHREAD_PRIORITY_FALLBACK_FLAG
#define DISPATCH_PRIORITY_FLAG_MANAGER       ((dispatch_priority_t)0x02000000) // _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG
#define DISPATCH_PRIORITY_PTHREAD_PRIORITY_FLAGS_MASK \
		(DISPATCH_PRIORITY_FLAG_OVERCOMMIT | DISPATCH_PRIORITY_FLAG_FALLBACK | \
		DISPATCH_PRIORITY_FLAG_MANAGER)

// not passed to pthread
#define DISPATCH_PRIORITY_FLAG_FLOOR         ((dispatch_priority_t)0x40000000) // _PTHREAD_PRIORITY_INHERIT_FLAG
#define DISPATCH_PRIORITY_FLAG_ENFORCE       ((dispatch_priority_t)0x10000000) // _PTHREAD_PRIORITY_ENFORCE_FLAG
#define DISPATCH_PRIORITY_FLAG_INHERITED     ((dispatch_priority_t)0x20000000)

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_qos_class_valid(qos_class_t cls, int relpri)
{
	switch ((unsigned int)cls) {
	case QOS_CLASS_MAINTENANCE:
	case QOS_CLASS_BACKGROUND:
	case QOS_CLASS_UTILITY:
	case QOS_CLASS_DEFAULT:
	case QOS_CLASS_USER_INITIATED:
	case QOS_CLASS_USER_INTERACTIVE:
	case QOS_CLASS_UNSPECIFIED:
		break;
	default:
		return false;
	}
	return QOS_MIN_RELATIVE_PRIORITY <= relpri && relpri <= 0;
}

#pragma mark dispatch_qos

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_qos_from_qos_class(qos_class_t cls)
{
	switch ((unsigned int)cls) {
	case QOS_CLASS_USER_INTERACTIVE: return DISPATCH_QOS_USER_INTERACTIVE;
	case QOS_CLASS_USER_INITIATED:   return DISPATCH_QOS_USER_INITIATED;
	case QOS_CLASS_DEFAULT:          return DISPATCH_QOS_DEFAULT;
	case QOS_CLASS_UTILITY:          return DISPATCH_QOS_UTILITY;
	case QOS_CLASS_BACKGROUND:       return DISPATCH_QOS_BACKGROUND;
	case QOS_CLASS_MAINTENANCE:      return DISPATCH_QOS_MAINTENANCE;
	default: return DISPATCH_QOS_UNSPECIFIED;
	}
}

DISPATCH_ALWAYS_INLINE
static inline qos_class_t
_dispatch_qos_to_qos_class(dispatch_qos_t qos)
{
	switch (qos) {
	case DISPATCH_QOS_USER_INTERACTIVE: return QOS_CLASS_USER_INTERACTIVE;
	case DISPATCH_QOS_USER_INITIATED:   return QOS_CLASS_USER_INITIATED;
	case DISPATCH_QOS_DEFAULT:          return QOS_CLASS_DEFAULT;
	case DISPATCH_QOS_UTILITY:          return QOS_CLASS_UTILITY;
	case DISPATCH_QOS_BACKGROUND:       return QOS_CLASS_BACKGROUND;
	case DISPATCH_QOS_MAINTENANCE:      return (qos_class_t)QOS_CLASS_MAINTENANCE;
	default: return QOS_CLASS_UNSPECIFIED;
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_qos_from_queue_priority(long priority)
{
	switch (priority) {
	case DISPATCH_QUEUE_PRIORITY_BACKGROUND:      return DISPATCH_QOS_BACKGROUND;
	case DISPATCH_QUEUE_PRIORITY_NON_INTERACTIVE: return DISPATCH_QOS_UTILITY;
	case DISPATCH_QUEUE_PRIORITY_LOW:             return DISPATCH_QOS_UTILITY;
	case DISPATCH_QUEUE_PRIORITY_DEFAULT:         return DISPATCH_QOS_DEFAULT;
	case DISPATCH_QUEUE_PRIORITY_HIGH:            return DISPATCH_QOS_USER_INITIATED;
	default: return _dispatch_qos_from_qos_class((qos_class_t)priority);
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_qos_from_pp(pthread_priority_t pp)
{
	pp &= _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	pp >>= _PTHREAD_PRIORITY_QOS_CLASS_SHIFT;
	return (dispatch_qos_t)__builtin_ffs((int)pp);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_qos_from_pp_unsafe(pthread_priority_t pp)
{
	// this assumes we know there is a QOS and pp has been masked off properly
	pp >>= _PTHREAD_PRIORITY_QOS_CLASS_SHIFT;
	DISPATCH_COMPILER_CAN_ASSUME(pp);
	return (dispatch_qos_t)__builtin_ffs((int)pp);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_qos_to_pp(dispatch_qos_t qos)
{
	pthread_priority_t pp;
	pp = 1ul << ((qos - 1) + _PTHREAD_PRIORITY_QOS_CLASS_SHIFT);
	return pp | _PTHREAD_PRIORITY_PRIORITY_MASK;
}

// including maintenance
DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_qos_is_background(dispatch_qos_t qos)
{
	return qos && qos <= DISPATCH_QOS_BACKGROUND;
}

#pragma mark dispatch_priority

#define _dispatch_priority_make(qos, relpri) \
	(qos ? ((((qos) << DISPATCH_PRIORITY_QOS_SHIFT) & DISPATCH_PRIORITY_QOS_MASK) | \
	 ((dispatch_priority_t)(relpri - 1) & DISPATCH_PRIORITY_RELPRI_MASK)) : 0)

#define _dispatch_priority_make_override(qos) \
	(((qos) << DISPATCH_PRIORITY_OVERRIDE_SHIFT) & \
	 DISPATCH_PRIORITY_OVERRIDE_MASK)

#define _dispatch_priority_make_floor(qos) \
	(qos ? (_dispatch_priority_make(qos) | DISPATCH_PRIORITY_FLAG_FLOOR) : 0)

#define _dispatch_priority_make_fallback(qos) \
	(qos ? ((((qos) << DISPATCH_PRIORITY_FALLBACK_QOS_SHIFT) & \
	 DISPATCH_PRIORITY_FALLBACK_QOS_MASK) | DISPATCH_PRIORITY_FLAG_FALLBACK) : 0)

DISPATCH_ALWAYS_INLINE
static inline int
_dispatch_priority_relpri(dispatch_priority_t dbp)
{
	if (dbp & DISPATCH_PRIORITY_QOS_MASK) {
		return (int8_t)(dbp & DISPATCH_PRIORITY_RELPRI_MASK) + 1;
	}
	return 0;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_priority_qos(dispatch_priority_t dbp)
{
	dbp &= DISPATCH_PRIORITY_QOS_MASK;
	return dbp >> DISPATCH_PRIORITY_QOS_SHIFT;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_priority_fallback_qos(dispatch_priority_t dbp)
{
	dbp &= DISPATCH_PRIORITY_FALLBACK_QOS_MASK;
	return dbp >> DISPATCH_PRIORITY_FALLBACK_QOS_SHIFT;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_priority_override_qos(dispatch_priority_t dbp)
{
	dbp &= DISPATCH_PRIORITY_OVERRIDE_MASK;
	return dbp >> DISPATCH_PRIORITY_OVERRIDE_SHIFT;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_priority_manually_selected(dispatch_priority_t pri)
{
	return !(pri & DISPATCH_PRIORITY_FLAG_INHERITED) &&
			(pri & (DISPATCH_PRIORITY_FLAG_FALLBACK |
			DISPATCH_PRIORITY_FLAG_FLOOR |
			DISPATCH_PRIORITY_REQUESTED_MASK));
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_priority_t
_dispatch_priority_from_pp_impl(pthread_priority_t pp, bool keep_flags)
{
	dispatch_assert(!(pp & _PTHREAD_PRIORITY_SCHED_PRI_FLAG));

	dispatch_priority_t dbp;
	if (keep_flags) {
		dbp = pp & (DISPATCH_PRIORITY_PTHREAD_PRIORITY_FLAGS_MASK |
				DISPATCH_PRIORITY_RELPRI_MASK);
	} else {
		dbp = pp & DISPATCH_PRIORITY_RELPRI_MASK;
	}

	dbp |= _dispatch_qos_from_pp(pp) << DISPATCH_PRIORITY_QOS_SHIFT;
	return dbp;
}
#define _dispatch_priority_from_pp(pp) \
		_dispatch_priority_from_pp_impl(pp, true)
#define _dispatch_priority_from_pp_strip_flags(pp) \
		_dispatch_priority_from_pp_impl(pp, false)

#define DISPATCH_PRIORITY_TO_PP_STRIP_FLAGS     0x1
#define DISPATCH_PRIORITY_TO_PP_PREFER_FALLBACK 0x2

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_priority_to_pp_strip_flags(dispatch_priority_t dbp)
{
	pthread_priority_t pp = dbp & DISPATCH_PRIORITY_RELPRI_MASK;
	dispatch_qos_t qos = _dispatch_priority_qos(dbp);
	if (qos) {
		pp |= (1ul << ((qos - 1) + _PTHREAD_PRIORITY_QOS_CLASS_SHIFT));
	}
	return pp;
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_priority_to_pp_prefer_fallback(dispatch_priority_t dbp)
{
	pthread_priority_t pp;
	dispatch_qos_t qos;

	if (dbp & DISPATCH_PRIORITY_FLAG_FALLBACK) {
		pp = dbp & DISPATCH_PRIORITY_PTHREAD_PRIORITY_FLAGS_MASK;
		pp |= _PTHREAD_PRIORITY_PRIORITY_MASK;
		qos = _dispatch_priority_fallback_qos(dbp);
	} else {
		pp = dbp & (DISPATCH_PRIORITY_PTHREAD_PRIORITY_FLAGS_MASK |
				DISPATCH_PRIORITY_RELPRI_MASK);
		qos = _dispatch_priority_qos(dbp);
		if (unlikely(!qos)) return pp;
	}

	return pp | (1ul << ((qos - 1) + _PTHREAD_PRIORITY_QOS_CLASS_SHIFT));
}

#endif // __DISPATCH_SHIMS_PRIORITY__
