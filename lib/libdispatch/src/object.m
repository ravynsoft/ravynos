/*
 * Copyright (c) 2011-2014 Apple Inc. All rights reserved.
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

#include "internal.h"

#if USE_OBJC

#if !__OBJC2__
#error "Cannot build with legacy ObjC runtime"
#endif
#if _OS_OBJECT_OBJC_ARC
#error "Cannot build with ARC"
#endif

#include <objc/objc-internal.h>
#include <objc/objc-exception.h>

#pragma mark -
#pragma mark _os_object_gc

#if __OBJC_GC__
#include <objc/objc-auto.h>
#include <auto_zone.h>

static bool _os_object_have_gc;
static malloc_zone_t *_os_object_gc_zone;

static void
_os_object_gc_init(void)
{
	_os_object_have_gc = objc_collectingEnabled();
	if (slowpath(_os_object_have_gc)) {
		_os_object_gc_zone = objc_collectableZone();
		(void)[OS_OBJECT_CLASS(object) class]; // OS_object class realization
	}
}

static _os_object_t
_os_object_make_uncollectable(_os_object_t obj)
{
	if (slowpath(_os_object_have_gc)) {
		auto_zone_retain(_os_object_gc_zone, obj);
	}
	return obj;
}

static _os_object_t
_os_object_make_collectable(_os_object_t obj)
{
	if (slowpath(_os_object_have_gc)) {
		auto_zone_release(_os_object_gc_zone, obj);
	}
	return obj;
}

DISPATCH_NOINLINE
static id
_os_objc_gc_retain(id obj)
{
	if (fastpath(obj)) {
		auto_zone_retain(_os_object_gc_zone, obj);
	}
	return obj;
}

DISPATCH_NOINLINE
static void
_os_objc_gc_release(id obj)
{
	if (fastpath(obj)) {
		(void)auto_zone_release(_os_object_gc_zone, obj);
	}
	asm(""); // prevent tailcall
}

DISPATCH_NOINLINE
static id
_os_object_gc_retain(id obj)
{
	if ([obj isKindOfClass:OS_OBJECT_OBJC_CLASS(object)]) {
		return _os_object_retain(obj);
	} else {
		return _os_objc_gc_retain(obj);
	}
}

DISPATCH_NOINLINE
static void
_os_object_gc_release(id obj)
{
	if ([obj isKindOfClass:OS_OBJECT_OBJC_CLASS(object)]) {
		return _os_object_release(obj);
	} else {
		return _os_objc_gc_release(obj);
	}
}

#else // __OBJC_GC__
#define _os_object_gc_init()
#define _os_object_make_uncollectable(obj) (obj)
#define _os_object_make_collectable(obj) (obj)
#define _os_object_have_gc 0
#define _os_object_gc_retain(obj) (obj)
#define _os_object_gc_release(obj)
#endif // __OBJC_GC__

#pragma mark -
#pragma mark _os_object_t

static inline id
_os_objc_alloc(Class cls, size_t size)
{
	id obj;
	size -= sizeof(((struct _os_object_s *)NULL)->os_obj_isa);
	while (!fastpath(obj = class_createInstance(cls, size))) {
		_dispatch_temporary_resource_shortage();
	}
	return obj;
}

static void*
_os_objc_destructInstance(id obj)
{
	// noop if only Libystem is loaded
	return obj;
}

void
_os_object_init(void)
{
	_objc_init();
	_os_object_gc_init();
	if (slowpath(_os_object_have_gc)) return;
	Block_callbacks_RR callbacks = {
		sizeof(Block_callbacks_RR),
		(void (*)(const void *))&objc_retain,
		(void (*)(const void *))&objc_release,
		(void (*)(const void *))&_os_objc_destructInstance
	};
	_Block_use_RR2(&callbacks);
}

_os_object_t
_os_object_alloc_realized(const void *cls, size_t size)
{
	dispatch_assert(size >= sizeof(struct _os_object_s));
	return _os_object_make_uncollectable(_os_objc_alloc(cls, size));
}

_os_object_t
_os_object_alloc(const void *_cls, size_t size)
{
	dispatch_assert(size >= sizeof(struct _os_object_s));
	Class cls = _cls ? [(id)_cls class] : [OS_OBJECT_CLASS(object) class];
	return _os_object_make_uncollectable(_os_objc_alloc(cls, size));
}

void
_os_object_dealloc(_os_object_t obj)
{
	[_os_object_make_collectable(obj) dealloc];
}

void
_os_object_xref_dispose(_os_object_t obj)
{
	[obj _xref_dispose];
}

void
_os_object_dispose(_os_object_t obj)
{
	[obj _dispose];
}

#undef os_retain
void*
os_retain(void *obj)
{
	if (slowpath(_os_object_have_gc)) return _os_object_gc_retain(obj);
	return objc_retain(obj);
}

#undef os_release
void
os_release(void *obj)
{
	if (slowpath(_os_object_have_gc)) return _os_object_gc_release(obj);
	return objc_release(obj);
}

#pragma mark -
#pragma mark _os_object

@implementation OS_OBJECT_CLASS(object)

-(id)retain {
	return _os_object_retain(self);
}

-(oneway void)release {
	return _os_object_release(self);
}

-(NSUInteger)retainCount {
	return _os_object_retain_count(self);
}

-(BOOL)retainWeakReference {
	return _os_object_retain_weak(self);
}

-(BOOL)allowsWeakReference {
	return _os_object_allows_weak_reference(self);
}

- (void)_xref_dispose {
	return _os_object_release_internal(self);
}

- (void)_dispose {
	return _os_object_dealloc(self);
}

@end

#pragma mark -
#pragma mark _dispatch_objc

#include <Foundation/NSString.h>

id
_dispatch_objc_alloc(Class cls, size_t size)
{
	return _os_objc_alloc(cls, size);
}

void
_dispatch_objc_retain(dispatch_object_t dou)
{
	return (void)os_retain(dou);
}

void
_dispatch_objc_release(dispatch_object_t dou)
{
	return os_release(dou);
}

void
_dispatch_objc_set_context(dispatch_object_t dou, void *context)
{
	return [dou _setContext:context];
}

void *
_dispatch_objc_get_context(dispatch_object_t dou)
{
	return [dou _getContext];
}

void
_dispatch_objc_set_finalizer_f(dispatch_object_t dou,
		dispatch_function_t finalizer)
{
	return [dou _setFinalizer:finalizer];
}

void
_dispatch_objc_set_target_queue(dispatch_object_t dou, dispatch_queue_t queue)
{
	return [dou _setTargetQueue:queue];
}

void
_dispatch_objc_suspend(dispatch_object_t dou)
{
	return [dou _suspend];
}

void
_dispatch_objc_resume(dispatch_object_t dou)
{
	return [dou _resume];
}

size_t
_dispatch_objc_debug(dispatch_object_t dou, char* buf, size_t bufsiz)
{
	NSUInteger offset = 0;
	NSString *desc = [dou debugDescription];
	[desc getBytes:buf maxLength:bufsiz-1 usedLength:&offset
			encoding:NSUTF8StringEncoding options:0
			range:NSMakeRange(0, [desc length]) remainingRange:NULL];
	if (offset) buf[offset] = 0;
	return offset;
}

#pragma mark -
#pragma mark _dispatch_object

// Force non-lazy class realization rdar://10640168
#define DISPATCH_OBJC_LOAD() + (void)load {}

@implementation DISPATCH_CLASS(object)

- (id)init {
	self = [super init];
	[self release];
	self = nil;
	return self;
}

- (void)_xref_dispose {
	_dispatch_xref_dispose(self);
	[super _xref_dispose];
}

- (void)_dispose {
	return _dispatch_dispose(self); // calls _os_object_dealloc()
}

- (NSString *)debugDescription {
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	char buf[2048];
	struct dispatch_object_s *obj = (struct dispatch_object_s *)self;
	if (obj->do_vtable->do_debug) {
		dx_debug(obj, buf, sizeof(buf));
	} else {
		strlcpy(buf, dx_kind(obj), sizeof(buf));
	}
	return [nsstring stringWithFormat:
			[nsstring stringWithUTF8String:"<%s: %s>"],
			class_getName([self class]), buf];
}

@end

@implementation DISPATCH_CLASS(queue)
DISPATCH_OBJC_LOAD()

- (NSString *)description {
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	return [nsstring stringWithFormat:
			[nsstring stringWithUTF8String:"<%s: %s[%p]>"],
			class_getName([self class]), dispatch_queue_get_label(self), self];
}

@end

@implementation DISPATCH_CLASS(source)
DISPATCH_OBJC_LOAD()

- (void)_xref_dispose {
	_dispatch_source_xref_dispose(self);
	[super _xref_dispose];
}

@end

@implementation DISPATCH_CLASS(queue_runloop)
DISPATCH_OBJC_LOAD()

- (void)_xref_dispose {
	_dispatch_runloop_queue_xref_dispose(self);
	[super _xref_dispose];
}

@end

#define DISPATCH_CLASS_IMPL(name) \
		@implementation DISPATCH_CLASS(name) \
		DISPATCH_OBJC_LOAD() \
		@end

DISPATCH_CLASS_IMPL(semaphore)
DISPATCH_CLASS_IMPL(group)
DISPATCH_CLASS_IMPL(queue_root)
DISPATCH_CLASS_IMPL(queue_mgr)
DISPATCH_CLASS_IMPL(queue_specific_queue)
DISPATCH_CLASS_IMPL(queue_attr)
DISPATCH_CLASS_IMPL(mach)
DISPATCH_CLASS_IMPL(mach_msg)
DISPATCH_CLASS_IMPL(io)
DISPATCH_CLASS_IMPL(operation)
DISPATCH_CLASS_IMPL(disk)

@implementation OS_OBJECT_CLASS(voucher)
DISPATCH_OBJC_LOAD()

- (id)init {
	self = [super init];
	[self release];
	self = nil;
	return self;
}

- (void)_xref_dispose {
	return _voucher_xref_dispose(self); // calls _os_object_release_internal()
}

- (void)_dispose {
	return _voucher_dispose(self); // calls _os_object_dealloc()
}

- (NSString *)debugDescription {
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	char buf[2048];
	_voucher_debug(self, buf, sizeof(buf));
	return [nsstring stringWithFormat:
			[nsstring stringWithUTF8String:"<%s: %s>"],
			class_getName([self class]), buf];
}

@end

#if VOUCHER_ENABLE_RECIPE_OBJECTS
@implementation OS_OBJECT_CLASS(voucher_recipe)
DISPATCH_OBJC_LOAD()

- (id)init {
	self = [super init];
	[self release];
	self = nil;
	return self;
}

- (void)_dispose {

}

- (NSString *)debugDescription {
	return nil; // TODO: voucher_recipe debugDescription
}

@end
#endif

#pragma mark -
#pragma mark dispatch_autorelease_pool

#if DISPATCH_COCOA_COMPAT

void *
_dispatch_autorelease_pool_push(void) {
	return objc_autoreleasePoolPush();
}

void
_dispatch_autorelease_pool_pop(void *context) {
	return objc_autoreleasePoolPop(context);
}

#endif // DISPATCH_COCOA_COMPAT

#pragma mark -
#pragma mark dispatch_client_callout

// Abort on uncaught exceptions thrown from client callouts rdar://8577499
#if DISPATCH_USE_CLIENT_CALLOUT && !__USING_SJLJ_EXCEPTIONS__
// On platforms with zero-cost exceptions, use a compiler-generated catch-all
// exception handler.

DISPATCH_NORETURN extern void objc_terminate(void);

#undef _dispatch_client_callout
void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
	@try {
		return f(ctxt);
	}
	@catch (...) {
		objc_terminate();
	}
}

#undef _dispatch_client_callout2
void
_dispatch_client_callout2(void *ctxt, size_t i, void (*f)(void *, size_t))
{
	@try {
		return f(ctxt, i);
	}
	@catch (...) {
		objc_terminate();
	}
}

#undef _dispatch_client_callout3
bool
_dispatch_client_callout3(void *ctxt, dispatch_data_t region, size_t offset,
		const void *buffer, size_t size, dispatch_data_applier_function_t f)
{
	@try {
		return f(ctxt, region, offset, buffer, size);
	}
	@catch (...) {
		objc_terminate();
	}
}

#undef _dispatch_client_callout4
void
_dispatch_client_callout4(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error,
		dispatch_mach_handler_function_t f)
{
	@try {
		return f(ctxt, reason, dmsg, error);
	}
	@catch (...) {
		objc_terminate();
	}
}

#endif // DISPATCH_USE_CLIENT_CALLOUT

#pragma mark -
#pragma mark _dispatch_block_create

// The compiler hides the name of the function it generates, and changes it if
// we try to reference it directly, but the linker still sees it.
extern void DISPATCH_BLOCK_SPECIAL_INVOKE(void *)
		asm("____dispatch_block_create_block_invoke");
void (*_dispatch_block_special_invoke)(void*) = DISPATCH_BLOCK_SPECIAL_INVOKE;

dispatch_block_t
_dispatch_block_create(dispatch_block_flags_t flags, voucher_t voucher,
		pthread_priority_t pri, dispatch_block_t block)
{
	dispatch_block_t copy_block = _dispatch_Block_copy(block); // 17094902
	struct dispatch_block_private_data_s dbpds =
			DISPATCH_BLOCK_PRIVATE_DATA_INITIALIZER(flags, voucher, pri, copy_block);
	dispatch_block_t new_block = _dispatch_Block_copy(^{
		// Capture object references, which retains copy_block and voucher.
		// All retained objects must be captured by the *block*. We
		// cannot borrow any references, because the block might be
		// called zero or several times, so Block_release() is the
		// only place that can release retained objects.
		(void)copy_block;
		(void)voucher;
		_dispatch_block_invoke(&dbpds);
	});
	Block_release(copy_block);
	return new_block;
}

#endif // USE_OBJC
