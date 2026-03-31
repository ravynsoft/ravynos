// TEST_CONFIG MEM=arc
// TEST_CFLAGS -framework Foundation

// Problem: If weak reference operations provoke +initialize, the runtime 
// can deadlock (recursive weak lock, or lock inversion between weak lock
// and +initialize lock).
// Solution: object_setClass() and objc_storeWeak() perform +initialize 
// if needed so that no weakly-referenced object can ever have an 
// un-+initialized isa.

#include <Foundation/Foundation.h>
#include <objc/objc-internal.h>
#include "test.h"

#pragma clang diagnostic ignored "-Warc-unsafe-retained-assign"

// This is StripedMap's pointer hash
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
    enum { StripeCount = 8 };
#else
    enum { StripeCount = 64 };
#endif
uintptr_t stripehash(id obj) {
    uintptr_t addr = (uintptr_t)obj;
    return ((addr >> 4) ^ (addr >> 9)) % StripeCount;
}

bool sameAlignment(id o1, id o2)
{
    return stripehash(o1) == stripehash(o2);
}

// Return a new non-tagged object that uses the same striped weak locks as `obj`
NSObject *newAlignedObject(id obj) 
{
    // Use immutable arrays because their contents are stored inline, 
    // which prevents Guard Malloc from using the same alignment for all of them
    NSArray *result = [NSArray new];
    while (!sameAlignment(obj, result)) {
        result = [result arrayByAddingObject:result];
    }
    return result;
}


__weak NSObject *weak1;
__weak NSObject *weak2;
NSObject *strong2;

@interface A : NSObject @end
@implementation A
+(void)initialize {
    weak2 = strong2;  // weak store #2
    strong2 = nil;
}
@end

void testA() 
{
    // Weak store #1 provokes +initialize which performs weak store #2.
    // Solution: weak store #1 runs +initialize if needed 
    // without holding locks.
    @autoreleasepool {
        A *obj = [A new];
        strong2 = newAlignedObject(obj);
        [obj addObserver:obj forKeyPath:@"foo" options:0 context:0];
        weak1 = obj;  // weak store #1
        [obj removeObserver:obj forKeyPath:@"foo"];
        obj = nil;
    }
}


__weak NSObject *weak3;
__weak NSObject *weak4;
NSObject *strong4;

@interface B : NSObject @end
@implementation B
+(void)initialize {
    weak4 = strong4;  // weak store #4
    strong4 = nil;
}
@end


void testB() 
{
    // Weak load #3 provokes +initialize which performs weak store #4.
    // Solution: object_setClass() runs +initialize if needed 
    // without holding locks.
    @autoreleasepool {
        B *obj = [B new];
        strong4 = newAlignedObject(obj);
        weak3 = obj;
        [obj addObserver:obj forKeyPath:@"foo" options:0 context:0];
        [weak3 self];  // weak load #3
        [obj removeObserver:obj forKeyPath:@"foo"];
        obj = nil;
    }
}


__weak id weak5;

@interface C : NSObject @end
@implementation C
+(void)initialize {
    weak5 = [self new];
}
@end

void testC()
{
    // +initialize performs a weak store of itself. 
    // Make sure the retry in objc_storeWeak() doesn't spin.
    @autoreleasepool {
        [C self];
    }
}


__weak id weak6;
NSObject *strong6;
semaphore_t Dgo;
semaphore_t Ddone;

void *Dthread(void *arg __unused)
{
    @autoreleasepool {
        semaphore_wait(Dgo);
        for (int i = 0; i < 1000; i++) {
            id x = weak6;
            testassert(x == strong6);
        }
        return nil;
    }
}

@interface D : NSObject @end
@implementation D
+(void)initialize {
    strong6 = [self new];
    weak6 = strong6;
    semaphore_signal(Dgo);
    for (int i = 0; i < 1000; i++) {
        id x = weak6;
        testassert(x == strong6);
    }
}
@end

void testD()
{
    // +initialize performs a weak store of itself, then another thread
    // tries to load that weak variable before +initialize completes.
    // Deadlock occurs if the +initialize thread tries to acquire the
    // sidetable lock for another operation and the second thread holds
    // the sidetable lock while waiting for +initialize.
    
    @autoreleasepool {
        semaphore_create(mach_task_self(), &Dgo, 0, 0);
        semaphore_create(mach_task_self(), &Ddone, 0, 0);
        pthread_t th;
        pthread_create(&th, nil, Dthread, nil);
        [D self];
        pthread_join(th, nil);
    }
}

int main()
{
    if (is_guardmalloc() && getenv("MALLOC_PROTECT_BEFORE")) {
        testwarn("fixme malloc guard before breaks this with debug libobjc");
    }
    else {
        alarm(10);  // replace hangs with crashes
        
        testA();
        testB();
        testC();
        testD();
    }

    succeed(__FILE__);
}

