/*
TEST_CFLAGS -Xlinker -sectcreate -Xlinker __DATA -Xlinker __objc_rawisa -Xlinker /dev/null
TEST_ENV OBJC_PRINT_RAW_ISA=YES

TEST_RUN_OUTPUT
objc\[\d+\]: RAW ISA: disabling non-pointer isa because the app has a __DATA,__objc_rawisa section
(.* RAW ISA: .*\n)*
OK: rawisa.m(\n.* RAW ISA: .*)*
OR
(.* RAW ISA: .*\n)*
no __DATA,__rawisa support
OK: rawisa.m(\n.* RAW ISA: .*)*
END

"RAW ISA" is allowed after "OK" because of static destructors
that provoke class realization.
*/

#include "test.h"

int main()
{
    fprintf(stderr, "\n");
#if ! (SUPPORT_NONPOINTER_ISA && TARGET_OS_OSX)
    // only 64-bit Mac supports this
    fprintf(stderr, "no __DATA,__rawisa support\n");
#endif
    succeed(__FILE__);
}

