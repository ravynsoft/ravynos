/*
Disallow some imports into and exports from libobjc.A.dylib.

To debug, re-run libobjc's link command with 
  -Xlinker -dead_strip -Xlinker -why_live -Xlinker SYMBOL_NAME_HERE

Disallowed imports (nm -u):
___cxa_guard_acquire   (C++ function-scope static initializer)
___cxa_guard_release   (C++ function-scope static initializer)
___cxa_atexit          (C++ static destructor)
weak external          (any weak externals, including operators new and delete)

Whitelisted imports:
weak external ____chkstk_darwin (from libSystem)

Disallowed exports (nm -U):
__Z*                   (any C++-mangled export)
weak external          (any weak externals, including operators new and delete)

fixme rdar://13354718 should disallow anything from libc++ (i.e. not libc++abi)
*/

/*
TEST_BUILD
echo $C{XCRUN} nm -m -arch $C{ARCH} $C{TESTLIB}
$C{XCRUN} nm -u -m -arch $C{ARCH} $C{TESTLIB} | grep -v 'weak external ____chkstk_darwin \(from libSystem\)' | egrep '(weak external| external (___cxa_atexit|___cxa_guard_acquire|___cxa_guard_release))' || true
$C{XCRUN} nm -U -m -arch $C{ARCH} $C{TESTLIB} | egrep '(weak external| external __Z)' || true
$C{COMPILE_C} $DIR/imports.c -o imports.exe
END

TEST_BUILD_OUTPUT
.*libobjc.A.dylib
END
 */

#include "test.h"
int main()
{
    succeed(__FILE__);
}
