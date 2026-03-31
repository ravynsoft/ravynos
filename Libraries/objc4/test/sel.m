/*
TEST_BUILD_OUTPUT
.*sel.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\](\n.* note: expanded from macro 'testassert')?
END
*/

#include "test.h"
#include <string.h>
#include <objc/objc-runtime.h>
#include <objc/objc-auto.h>

int main()
{
    // Make sure @selector values are correctly fixed up
    testassert(@selector(foo) == sel_registerName("foo"));

    // sel_getName recognizes the zero SEL
    testassert(0 == strcmp("<null selector>", sel_getName(0)));

    succeed(__FILE__);
}
