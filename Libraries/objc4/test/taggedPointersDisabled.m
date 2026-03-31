/*
TEST_ENV OBJC_DISABLE_TAGGED_POINTERS=YES
TEST_CRASHES

TEST_BUILD_OUTPUT
.*taggedPointersDisabled.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
END

TEST_RUN_OUTPUT
objc\[\d+\]: tagged pointers are disabled
objc\[\d+\]: HALTED
OR
OK: taggedPointersDisabled.m
END
*/

#include "test.h"
#include <objc/objc-internal.h>

#if !OBJC_HAVE_TAGGED_POINTERS

int main()
{
    // provoke the same nullability warning as the real test
    objc_getClass(nil);

    succeed(__FILE__);
}

#else

int main()
{
    testassert(!_objc_taggedPointersEnabled());
    _objc_registerTaggedPointerClass((objc_tag_index_t)0, nil);
    fail("should have crashed in _objc_registerTaggedPointerClass()");
}

#endif
