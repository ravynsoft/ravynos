// TEST_ENV OBJC_DISABLE_TAG_OBFUSCATION=YES

#include "test.h"
#include <objc/objc-internal.h>

#if !OBJC_HAVE_TAGGED_POINTERS

int main()
{
    succeed(__FILE__);
}

#else

int main()
{
    testassert(_objc_getTaggedPointerTag((void *)1) == 0);
    succeed(__FILE__);
}

#endif
