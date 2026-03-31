/* 
TEST_CRASHES
TEST_BUILD_OUTPUT
.*badTagClass.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
END
TEST_RUN_OUTPUT
objc\[\d+\]: tag index 1 used for two different classes \(was 0x[0-9a-fA-F]+ NSObject, now 0x[0-9a-fA-F]+ TestRoot\)
objc\[\d+\]: HALTED
OR
no tagged pointers
OK: badTagClass.m
END
*/

#include "test.h"
#include "testroot.i"

#include <objc/objc-internal.h>
#include <objc/Protocol.h>

#if OBJC_HAVE_TAGGED_POINTERS

int main()
{
    // re-registration and nil registration allowed
    _objc_registerTaggedPointerClass(OBJC_TAG_1, [NSObject class]);
    _objc_registerTaggedPointerClass(OBJC_TAG_1, [NSObject class]);
    _objc_registerTaggedPointerClass(OBJC_TAG_1, nil);
    _objc_registerTaggedPointerClass(OBJC_TAG_1, [NSObject class]);

    // colliding registration disallowed
    _objc_registerTaggedPointerClass(OBJC_TAG_1, [TestRoot class]);

    fail(__FILE__);
}

#else

int main()
{
    // provoke the same nullability warning as the real test
    objc_getClass(nil);
    
    fprintf(stderr, "no tagged pointers\n");
    succeed(__FILE__);
}

#endif
