/* 
TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: tag index 264 is invalid
objc\[\d+\]: HALTED
OR
no tagged pointers
OK: badTagIndex.m
END
*/

#include "test.h"

#include <objc/objc-internal.h>
#include <objc/NSObject.h>

#if OBJC_HAVE_TAGGED_POINTERS

int main()
{
    _objc_registerTaggedPointerClass((objc_tag_index_t)(OBJC_TAG_Last52BitPayload+1), [NSObject class]);
    fail(__FILE__);
}

#else

int main()
{
    fprintf(stderr, "no tagged pointers\n");
    succeed(__FILE__);
}

#endif
