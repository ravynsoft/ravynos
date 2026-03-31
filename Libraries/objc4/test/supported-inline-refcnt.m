// TEST_CONFIG MEM=mrc
// TEST_CFLAGS -framework CoreFoundation -Weverything

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "test.h"
#pragma clang diagnostic pop
#include <objc/NSObject.h>
#include <objc/objc-internal.h>
#include <CoreFoundation/CoreFoundation.h>


// Some warnings just aren't feasible to work around. We'll disable them instead.
#pragma clang diagnostic ignored "-Watomic-implicit-seq-cst"
#pragma clang diagnostic ignored "-Wdirect-ivar-access"
#pragma clang diagnostic ignored "-Wold-style-cast"

static int deallocCount;
@interface Refcnt: NSObject @end
@implementation Refcnt {
    int _rc;
}

_OBJC_SUPPORTED_INLINE_REFCNT(_rc)

- (void)dealloc {
    deallocCount++;
    [super dealloc];
}

@end

@interface MainRefcnt: NSObject @end
@implementation MainRefcnt {
    int _rc;
}

_OBJC_SUPPORTED_INLINE_REFCNT_WITH_DEALLOC2MAIN(_rc)

- (void)dealloc {
    testassert(pthread_main_np());
    deallocCount++;
    [super dealloc];
}

@end

int main()
{
    Refcnt *obj = [Refcnt new];
    [obj retain];
    [obj retain];
    [obj retain];
    [obj release];
    [obj release];
    [obj release];
    [obj release];
    testassert(deallocCount == 1);

    MainRefcnt *obj2 = [MainRefcnt new];
    [obj2 retain];
    [obj2 retain];
    [obj2 retain];
    
    dispatch_group_t group = dispatch_group_create();
    dispatch_group_async(group, dispatch_get_global_queue(0, 0), ^{
        [obj2 release];
    });
    dispatch_group_async(group, dispatch_get_global_queue(0, 0), ^{
        [obj2 release];
    });
    dispatch_group_async(group, dispatch_get_global_queue(0, 0), ^{
        [obj2 release];
    });
    dispatch_group_async(group, dispatch_get_global_queue(0, 0), ^{
        [obj2 release];
    });
    
    dispatch_group_notify(group, dispatch_get_main_queue(), ^{
        testassert(deallocCount == 2);
        succeed(__FILE__);
    });
    
    CFRunLoopRun();
}
