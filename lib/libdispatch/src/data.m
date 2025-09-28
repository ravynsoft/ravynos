/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#include <Foundation/NSString.h>

@interface DISPATCH_CLASS(data) ()
- (id)initWithBytes:(void *)bytes length:(NSUInteger)length copy:(BOOL)copy
		freeWhenDone:(BOOL)freeBytes bytesAreVM:(BOOL)vm;
- (BOOL)_bytesAreVM;
@end

@interface DISPATCH_CLASS(data_empty) : DISPATCH_CLASS(data)
@end

@implementation DISPATCH_CLASS(data)

+ (id)allocWithZone:(NSZone *) DISPATCH_UNUSED zone {
	return _dispatch_objc_alloc(self, sizeof(struct dispatch_data_s));
}

- (id)init {
    return [self initWithBytes:NULL length:0 copy:NO freeWhenDone:NO
			bytesAreVM:NO];
}

- (id)initWithBytes:(void *)bytes length:(NSUInteger)length copy:(BOOL)copy
		freeWhenDone:(BOOL)freeBytes bytesAreVM:(BOOL)vm {
	dispatch_block_t destructor;
	if (copy) {
		destructor = DISPATCH_DATA_DESTRUCTOR_DEFAULT;
	} else if (freeBytes) {
		if (vm) {
			destructor = DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE;
		} else {
			destructor = DISPATCH_DATA_DESTRUCTOR_FREE;
		}
	} else {
		destructor = DISPATCH_DATA_DESTRUCTOR_NONE;
	}
	dispatch_data_init(self, bytes, length, destructor);
	return self;
}

#define _dispatch_data_objc_dispose(selector) \
	struct dispatch_data_s *dd = (void*)self; \
	_dispatch_data_dispose(self); \
	dispatch_queue_t tq = dd->do_targetq; \
	dispatch_function_t func = dd->finalizer; \
	void *ctxt = dd->ctxt; \
	[super selector]; \
	if (func && ctxt) { \
		if (!tq) { \
			 tq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0);\
		} \
		dispatch_async_f(tq, ctxt, func); \
	} \
	if (tq) { \
		_os_object_release_internal((_os_object_t)tq); \
	}

- (void)dealloc {
	_dispatch_data_objc_dispose(dealloc);
}

- (void)finalize {
	_dispatch_data_objc_dispose(finalize);
}

- (BOOL)_bytesAreVM {
	struct dispatch_data_s *dd = (void*)self;
	return dd->destructor == DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE;
}

- (void)_setContext:(void*)context {
	struct dispatch_data_s *dd = (void*)self;
	dd->ctxt = context;
}

- (void*)_getContext {
	struct dispatch_data_s *dd = (void*)self;
	return dd->ctxt;
}

- (void)_setFinalizer:(dispatch_function_t)finalizer {
	struct dispatch_data_s *dd = (void*)self;
	dd->finalizer = finalizer;
}

- (void)_setTargetQueue:(dispatch_queue_t)queue {
	struct dispatch_data_s *dd = (void*)self;
	_os_object_retain_internal((_os_object_t)queue);
	dispatch_queue_t prev;
	prev = dispatch_atomic_xchg2o(dd, do_targetq, queue, release);
	if (prev) _os_object_release_internal((_os_object_t)prev);
}

- (NSString *)debugDescription {
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	char buf[2048];
	_dispatch_data_debug(self, buf, sizeof(buf));
	return [nsstring stringWithFormat:
			[nsstring stringWithUTF8String:"<%s: %s>"],
			class_getName([self class]), buf];
}

@end

@implementation DISPATCH_CLASS(data_empty)

// Force non-lazy class realization rdar://10640168
+ (void)load {
}

- (id)retain {
	return (id)self;
}

- (oneway void)release {
}

- (id)autorelease {
	return (id)self;
}

- (NSUInteger)retainCount {
	return ULONG_MAX;
}

+ (id)allocWithZone:(NSZone *) DISPATCH_UNUSED zone {
	return (id)&_dispatch_data_empty;
}

- (void)_setContext:(void*) DISPATCH_UNUSED context {
}

- (void*)_getContext {
	return NULL;
}

- (void)_setFinalizer:(dispatch_function_t) DISPATCH_UNUSED finalizer {
}

- (void)_setTargetQueue:(dispatch_queue_t) DISPATCH_UNUSED queue {
}

@end

#endif // USE_OBJC
