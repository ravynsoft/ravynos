// TEST_CFLAGS -framework CoreFoundation
// TEST_CONFIG MEM=mrc
// not for ARC because ARC memory management doesn't
// work on CF types whose ObjC side is not yet loaded

#include <CoreFoundation/CoreFoundation.h>
#include <objc/runtime.h>

#include "test.h"

#if __has_feature(objc_arc)

int main()
{
    testwarn("rdar://11368528 confused by Foundation");
    succeed(__FILE__);
}

#else

int main()
{
    // rdar://6164781 setAssociatedObject on unresolved future class crashes

    id mp = (id)CFMachPortCreate(0, 0, 0, 0);
    testassert(mp);

    testassert(! objc_getClass("NSMachPort"));

    objc_setAssociatedObject(mp, (void*)1, mp, OBJC_ASSOCIATION_ASSIGN);

    id obj = objc_getAssociatedObject(mp, (void*)1);
    testassert(obj == mp);

    CFRelease((CFTypeRef)mp);

    succeed(__FILE__);
}

#endif
