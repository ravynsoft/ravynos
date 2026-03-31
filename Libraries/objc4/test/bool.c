// TEST_CFLAGS -funsigned-char
// (verify -funsigned-char doesn't change the definition of BOOL)

#include "test.h"
#include <objc/objc.h>

#if TARGET_OS_OSX
#   define RealBool 0
#elif TARGET_OS_IOS || TARGET_OS_BRIDGE
#   if (__arm__ && !__armv7k__) || __i386__
#       define RealBool 0
#   else
#       define RealBool 1
#   endif
#else
#   define RealBool 1
#endif

#if __OBJC__ && !defined(__OBJC_BOOL_IS_BOOL)
#   error no __OBJC_BOOL_IS_BOOL
#endif

#if RealBool != OBJC_BOOL_IS_BOOL
#   error wrong OBJC_BOOL_IS_BOOL
#endif

#if RealBool == OBJC_BOOL_IS_CHAR
#   error wrong OBJC_BOOL_IS_CHAR
#endif

int main()
{
    const char *expected __unused =
#if RealBool
        "B"
#else
        "c"
#endif
        ;
#if __OBJC__
    const char *enc = @encode(BOOL);
    testassert(0 == strcmp(enc, expected));
#endif
    succeed(__FILE__);
}
