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

#if DISPATCH_DATA_IS_BRIDGED_TO_NSDATA

#if _OS_OBJECT_OBJC_ARC
#error "Cannot build with ARC"
#endif

#include <Foundation/NSString.h>

// NOTE: this file must not contain any atomic operations

@interface DISPATCH_CLASS(data) () <DISPATCH_CLASS(data)>
@property (readonly,nonatomic) NSUInteger length;
@property (readonly,nonatomic) const void *bytes NS_RETURNS_INNER_POINTER;

- (id)initWithBytes:(void *)bytes length:(NSUInteger)length copy:(BOOL)copy
		freeWhenDone:(BOOL)freeBytes bytesAreVM:(BOOL)vm;
- (BOOL)_bytesAreVM;
- (BOOL)_isCompact;
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
	_dispatch_data_init_with_bytes(self, bytes, length, destructor);
	return self;
}

- (void)dealloc {
	struct dispatch_data_s *dd = (void*)self;
	_dispatch_data_dispose(self, NULL);
	dispatch_queue_t tq = dd->do_targetq;
	dispatch_function_t func = dd->finalizer;
	void *ctxt = dd->ctxt;
	[super dealloc];
	if (func && ctxt) {
		if (!tq) {
			 tq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0);
		}
		dispatch_async_f(tq, ctxt, func);
	}
	if (tq) {
		_os_object_release_internal((_os_object_t)tq);
	}
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
	return _dispatch_data_set_target_queue(dd, queue);
}

- (NSString *)debugDescription {
	Class nsstring = objc_lookUpClass("NSString");
	if (!nsstring) return nil;
	char buf[2048];
	_dispatch_data_debug(self, buf, sizeof(buf));
	NSString *format = [nsstring stringWithUTF8String:"<%s: %s>"];
	if (!format) return nil;
	return [nsstring stringWithFormat:format, object_getClassName(self), buf];
}

- (NSUInteger)length {
	struct dispatch_data_s *dd = (void*)self;
	return dd->size;
}

- (const void *)bytes {
	return dispatch_data_get_flattened_bytes_4libxpc(self);
}

- (BOOL)_isCompact {
	struct dispatch_data_s *dd = (void*)self;
	return !dd->size || _dispatch_data_map_direct(dd, 0, NULL, NULL) != NULL;
}

- (void)_suspend {
}

- (void)_resume {
}

- (void)_activate {
}

@end

OS_OBJECT_NONLAZY_CLASS
@implementation DISPATCH_CLASS(data_empty)
OS_OBJECT_NONLAZY_CLASS_LOAD

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

- (void)_suspend {
}

- (void)_resume {
}

- (void)_activate {
}

@end

#endif // USE_OBJC
