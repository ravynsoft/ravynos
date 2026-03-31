// testroot.i
// Implementation of class TestRoot
// Include this file into your main test file to use it.

#include "test.h"
#include <dlfcn.h>
#include <objc/objc-internal.h>

atomic_int TestRootLoad;
atomic_int TestRootInitialize;
atomic_int TestRootAlloc;
atomic_int TestRootAllocWithZone;
atomic_int TestRootCopy;
atomic_int TestRootCopyWithZone;
atomic_int TestRootMutableCopy;
atomic_int TestRootMutableCopyWithZone;
atomic_int TestRootInit;
atomic_int TestRootDealloc;
atomic_int TestRootRetain;
atomic_int TestRootRelease;
atomic_int TestRootAutorelease;
atomic_int TestRootRetainCount;
atomic_int TestRootTryRetain;
atomic_int TestRootIsDeallocating;
atomic_int TestRootPlusRetain;
atomic_int TestRootPlusRelease;
atomic_int TestRootPlusAutorelease;
atomic_int TestRootPlusRetainCount;


@implementation TestRoot

// These all use void* pending rdar://9310005.

static void *
retain_fn(void *self, SEL _cmd __unused) {
    atomic_fetch_add_explicit(&TestRootRetain, 1, memory_order_relaxed);
    void * (*fn)(void *) = (typeof(fn))_objc_rootRetain;
    return fn(self); 
}

static void 
release_fn(void *self, SEL _cmd __unused) {
    atomic_fetch_add_explicit(&TestRootRelease, 1, memory_order_relaxed);
    void (*fn)(void *) = (typeof(fn))_objc_rootRelease;
    fn(self); 
}

static void *
autorelease_fn(void *self, SEL _cmd __unused) { 
    atomic_fetch_add_explicit(&TestRootAutorelease, 1, memory_order_relaxed);
    void * (*fn)(void *) = (typeof(fn))_objc_rootAutorelease;
    return fn(self); 
}

static unsigned long 
retaincount_fn(void *self, SEL _cmd __unused) { 
    atomic_fetch_add_explicit(&TestRootRetainCount, 1, memory_order_relaxed);
    unsigned long (*fn)(void *) = (typeof(fn))_objc_rootRetainCount;
    return fn(self); 
}

static void *
copywithzone_fn(void *self, SEL _cmd __unused, void *zone) { 
    atomic_fetch_add_explicit(&TestRootCopyWithZone, 1, memory_order_relaxed);
    void * (*fn)(void *, void *) =
        (typeof(fn))dlsym(RTLD_DEFAULT, "object_copy");
    return fn(self, zone); 
}

static void *
plusretain_fn(void *self __unused, SEL _cmd __unused) {
    atomic_fetch_add_explicit(&TestRootPlusRetain, 1, memory_order_relaxed);
    return self;
}

static void 
plusrelease_fn(void *self __unused, SEL _cmd __unused) {
    atomic_fetch_add_explicit(&TestRootPlusRelease, 1, memory_order_relaxed);
}

static void * 
plusautorelease_fn(void *self, SEL _cmd __unused) { 
    atomic_fetch_add_explicit(&TestRootPlusAutorelease, 1, memory_order_relaxed);
    return self;
}

static unsigned long 
plusretaincount_fn(void *self __unused, SEL _cmd __unused) { 
    atomic_fetch_add_explicit(&TestRootPlusRetainCount, 1, memory_order_relaxed);
    return ULONG_MAX;
}

+(void) load {
    atomic_fetch_add_explicit(&TestRootLoad, 1, memory_order_relaxed);
    
    // install methods that ARC refuses to compile
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");
    class_addMethod(self, sel_registerName("autorelease"), (IMP)autorelease_fn, "");
    class_addMethod(self, sel_registerName("retainCount"), (IMP)retaincount_fn, "");
    class_addMethod(self, sel_registerName("copyWithZone:"), (IMP)copywithzone_fn, "");

    class_addMethod(object_getClass(self), sel_registerName("retain"), (IMP)plusretain_fn, "");
    class_addMethod(object_getClass(self), sel_registerName("release"), (IMP)plusrelease_fn, "");
    class_addMethod(object_getClass(self), sel_registerName("autorelease"), (IMP)plusautorelease_fn, "");
    class_addMethod(object_getClass(self), sel_registerName("retainCount"), (IMP)plusretaincount_fn, "");
}


+(void) initialize {
    atomic_fetch_add_explicit(&TestRootInitialize, 1, memory_order_relaxed);
}

-(id) self {
    return self;
}

+(Class) class {
    return self;
}

-(Class) class {
    return object_getClass(self);
}

+(Class) superclass {
    return class_getSuperclass(self);
}

-(Class) superclass {
    return class_getSuperclass([self class]);
}

+(id) new {
    return [[self alloc] init];
}

+(id) alloc {
    atomic_fetch_add_explicit(&TestRootAlloc, 1, memory_order_relaxed);
    void * (*fn)(id __unsafe_unretained) = (typeof(fn))_objc_rootAlloc;
    return (__bridge_transfer id)(fn(self));
}

+(id) allocWithZone:(void *)zone {
    atomic_fetch_add_explicit(&TestRootAllocWithZone, 1, memory_order_relaxed);
    void * (*fn)(id __unsafe_unretained, void *) = (typeof(fn))_objc_rootAllocWithZone;
    return (__bridge_transfer id)(fn(self, zone));
}

+(id) copy {
    return self;
}

+(id) copyWithZone:(void *) __unused zone {
    return self;
}

-(id) copy {
    atomic_fetch_add_explicit(&TestRootCopy, 1, memory_order_relaxed);
    return [self copyWithZone:NULL];
}

+(id) mutableCopyWithZone:(void *) __unused zone {
    fail("+mutableCopyWithZone: called");
}

-(id) mutableCopy {
    atomic_fetch_add_explicit(&TestRootMutableCopy, 1, memory_order_relaxed);
    return [self mutableCopyWithZone:NULL];
}

-(id) mutableCopyWithZone:(void *) __unused zone {
    atomic_fetch_add_explicit(&TestRootMutableCopyWithZone, 1, memory_order_relaxed);
    void * (*fn)(id __unsafe_unretained) = (typeof(fn))_objc_rootAlloc;
    return (__bridge_transfer id)(fn(object_getClass(self)));
}

-(id) init {
    atomic_fetch_add_explicit(&TestRootInit, 1, memory_order_relaxed);
    return _objc_rootInit(self);
}

+(void) dealloc {
    fail("+dealloc called");
}

-(void) dealloc {
    atomic_fetch_add_explicit(&TestRootDealloc, 1, memory_order_relaxed);
    _objc_rootDealloc(self);
}

+(BOOL) _tryRetain {
    return YES;
}

-(BOOL) _tryRetain {
    atomic_fetch_add_explicit(&TestRootTryRetain, 1, memory_order_relaxed);
    return _objc_rootTryRetain(self);
}

+(BOOL) _isDeallocating {
    return NO;
}

-(BOOL) _isDeallocating {
    atomic_fetch_add_explicit(&TestRootIsDeallocating, 1, memory_order_relaxed);
    return _objc_rootIsDeallocating(self);
}

-(BOOL) allowsWeakReference {
    return ! [self _isDeallocating]; 
}

-(BOOL) retainWeakReference { 
    return [self _tryRetain]; 
}


@end
