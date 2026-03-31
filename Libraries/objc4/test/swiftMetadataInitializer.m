// TEST_CONFIG MEM=mrc

#include "test.h"
#include "swift-class-def.m"


SWIFT_CLASS(SwiftSuper, NSObject, initSuper);
SWIFT_CLASS(SwiftSub, SwiftSuper, initSub);

// _objc_swiftMetadataInitializer hooks for the fake Swift classes

Class initSuper(Class cls __unused, void *arg __unused)
{
    // This test provokes objc's callback out of superclass order.
    // SwiftSub's init is first. SwiftSuper's init is never called.

    fail("SwiftSuper's init should not have been called");
}

bool isRealized(Class cls)
{
    // check the is-realized bits directly

#if __LP64__
# define mask (~(uintptr_t)7)
#else
# define mask (~(uintptr_t)3)
#endif
#define RW_REALIZED (1<<31)
    
    uintptr_t rw = ((uintptr_t *)cls)[4] & mask;  // class_t->data
    return ((uint32_t *)rw)[0] & RW_REALIZED;  // class_rw_t->flags
}

static int SubInits = 0;
Class initSub(Class cls, void *arg)
{
    testprintf("initSub callback\n");
    
    testassert(SubInits == 0);
    SubInits++;
    testassert(arg == nil);
    testassert(0 == strcmp(class_getName(cls), "SwiftSub"));
    testassert(cls == RawSwiftSub);
    testassert(!isRealized(RawSwiftSuper));
    testassert(!isRealized(RawSwiftSub));

    testprintf("initSub beginning _objc_realizeClassFromSwift\n");
    _objc_realizeClassFromSwift(cls, cls);
    testprintf("initSub finished  _objc_realizeClassFromSwift\n");

    testassert(isRealized(RawSwiftSuper));
    testassert(isRealized(RawSwiftSub));
    
    return cls;
}


int main()
{
    testassert(SubInits == 0);
    testprintf("calling [SwiftSub class]\n");
    [SwiftSub class];
    testprintf("finished [SwiftSub class]\n");
    testassert(SubInits == 1);
    [SwiftSuper class];
    succeed(__FILE__);
}
