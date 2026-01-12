/*
 * Copyright (c) 2011-2012 Apple Inc. All rights reserved.
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

#ifndef __OS_OBJECT_PRIVATE__
#define __OS_OBJECT_PRIVATE__

#include <os/object.h>
#include <stddef.h>
#include <stdint.h>

#if __GNUC__
#define OS_OBJECT_NOTHROW __attribute__((__nothrow__))
#define OS_OBJECT_NONNULL __attribute__((__nonnull__))
#define OS_OBJECT_WARN_RESULT __attribute__((__warn_unused_result__))
#define OS_OBJECT_MALLOC __attribute__((__malloc__))
#ifndef OS_OBJECT_EXPORT
#define OS_OBJECT_EXPORT extern __attribute__((visibility("default")))
#endif
#else
/*! @parseOnly */
#define OS_OBJECT_NOTHROW
/*! @parseOnly */
#define OS_OBJECT_NONNULL
/*! @parseOnly */
#define OS_OBJECT_WARN_RESULT
/*! @parseOnly */
#define OS_OBJECT_MALLOC
#ifndef OS_OBJECT_EXPORT
/*! @parseOnly */
#define OS_OBJECT_EXPORT extern
#endif
#endif

#if OS_OBJECT_USE_OBJC && __has_feature(objc_arc)
#define _OS_OBJECT_OBJC_ARC 1
#else
#define _OS_OBJECT_OBJC_ARC 0
#endif

#define _OS_OBJECT_GLOBAL_REFCNT INT_MAX

#define _OS_OBJECT_HEADER(isa, ref_cnt, xref_cnt) \
        isa; /* must be pointer-sized */ \
        int volatile ref_cnt; \
        int volatile xref_cnt

#if OS_OBJECT_HAVE_OBJC_SUPPORT
#define OS_OBJECT_CLASS_SYMBOL(name) OS_##name##_class
#if TARGET_OS_MAC && !TARGET_OS_SIMULATOR && defined(__i386__)
#define OS_OBJECT_HAVE_OBJC1 1
#define OS_OBJECT_HAVE_OBJC2 0
#define OS_OBJC_CLASS_RAW_SYMBOL_NAME(name) \
		".objc_class_name_" OS_STRINGIFY(name)
#define _OS_OBJECT_CLASS_HEADER() \
		const void *_os_obj_objc_isa
#else
#define OS_OBJECT_HAVE_OBJC1 0
#define OS_OBJECT_HAVE_OBJC2 1
#define OS_OBJC_CLASS_RAW_SYMBOL_NAME(name) "_OBJC_CLASS_$_" OS_STRINGIFY(name)
// Must match size of compiler-generated OBJC_CLASS structure rdar://10640168
#define _OS_OBJECT_CLASS_HEADER() \
		void *_os_obj_objc_class_t[5]
#endif
#define OS_OBJECT_OBJC_CLASS_DECL(name) \
		extern void *OS_OBJECT_CLASS_SYMBOL(name) \
				__asm__(OS_OBJC_CLASS_RAW_SYMBOL_NAME(OS_OBJECT_CLASS(name)))
#else
#define OS_OBJECT_HAVE_OBJC1 0
#define OS_OBJECT_HAVE_OBJC2 0
#define _OS_OBJECT_CLASS_HEADER() \
		void (*_os_obj_xref_dispose)(_os_object_t); \
		void (*_os_obj_dispose)(_os_object_t)
#endif

#define OS_OBJECT_CLASS(name) OS_##name

#if OS_OBJECT_USE_OBJC
#define OS_OBJECT_USES_XREF_DISPOSE() \
	- (oneway void)release { \
		_os_object_release(self); \
	}
#endif

#if __has_attribute(objc_nonlazy_class)
#define OS_OBJECT_NONLAZY_CLASS        __attribute__((objc_nonlazy_class))
#define OS_OBJECT_NONLAZY_CLASS_LOAD
#else
#define OS_OBJECT_NONLAZY_CLASS
#define OS_OBJECT_NONLAZY_CLASS_LOAD  + (void)load { }
#endif

#if OS_OBJECT_USE_OBJC && OS_OBJECT_SWIFT3
@interface OS_OBJECT_CLASS(object) (OSObjectPrivate)
// Note: objects who want _xref_dispose to be called need
// to use OS_OBJECT_USES_XREF_DISPOSE()
- (void)_xref_dispose;
@end
OS_OBJECT_DECL_PROTOCOL(object, <NSObject>);
typedef OS_OBJECT_CLASS(object) *_os_object_t;
#define _OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super) \
		@interface OS_OBJECT_CLASS(name) : OS_OBJECT_CLASS(super) \
		<OS_OBJECT_CLASS(name)> \
		@end
#define _OS_OBJECT_DECL_PROTOCOL(name, super) \
		OS_OBJECT_DECL_PROTOCOL(name, <OS_OBJECT_CLASS(super)>)
#define _OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(name, super) \
		OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(name, super)
#elif OS_OBJECT_USE_OBJC
API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT
@interface OS_OBJECT_CLASS(object) : NSObject
// Note: objects who want _xref_dispose to be called need
// to use OS_OBJECT_USES_XREF_DISPOSE()
- (void)_xref_dispose;
@end
typedef OS_OBJECT_CLASS(object) *_os_object_t;
#define _OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super) \
		@interface OS_OBJECT_CLASS(name) : OS_OBJECT_CLASS(super) \
		<OS_OBJECT_CLASS(name)> \
		@end
#else
#define _OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super)
#define _OS_OBJECT_DECL_PROTOCOL(name, super)
#define _OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(name, super)
typedef struct _os_object_s *_os_object_t;
#endif

OS_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

#if !_OS_OBJECT_OBJC_ARC

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_MALLOC OS_OBJECT_WARN_RESULT OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_alloc(const void *cls, size_t size);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_MALLOC OS_OBJECT_WARN_RESULT OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_alloc_realized(const void *cls, size_t size);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
void _os_object_dealloc(_os_object_t object);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_retain(_os_object_t object);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_retain_with_resurrect(_os_object_t obj);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
void
_os_object_release(_os_object_t object);

API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
void
_os_object_release_without_xref_dispose(_os_object_t object);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_retain_internal(_os_object_t object);

API_AVAILABLE(macos(10.8), ios(6.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
void
_os_object_release_internal(_os_object_t object);

API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
_os_object_t
_os_object_retain_internal_n(_os_object_t object, uint16_t n);

API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
OS_SWIFT_UNAVAILABLE("Unavailable in Swift")
void
_os_object_release_internal_n(_os_object_t object, uint16_t n);

#endif // !_OS_OBJECT_OBJC_ARC

__END_DECLS

OS_ASSUME_NONNULL_END

#endif
