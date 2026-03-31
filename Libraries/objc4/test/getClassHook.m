// TEST_CONFIG

#include "test.h"
#include "testroot.i"

@interface OrdinaryClass : TestRoot @end
@implementation OrdinaryClass @end

objc_hook_getClass OnePreviousHook;
static int HookOneCalls = 0;
BOOL GetClassHookOne(const char *name, Class *outClass)
{
    HookOneCalls++;
    if (0 == strcmp(name, "TwoClass")) {
        fail("other hook should have handled this already");
    } else if (0 == strcmp(name, "OrdinaryClass")) {
        fail("runtime should have handled this already");
    } else if (0 == strcmp(name, "OneClass")) {
        Class cls = objc_allocateClassPair([OrdinaryClass class], "OneClass", 0);
        objc_registerClassPair(cls);
        *outClass = cls;
        return YES;
    } else {
        return OnePreviousHook(name, outClass);
    }
}

objc_hook_getClass TwoPreviousHook;
static int HookTwoCalls = 0;
BOOL GetClassHookTwo(const char *name, Class *outClass)
{
    HookTwoCalls++;
    if (0 == strcmp(name, "OrdinaryClass")) {
        fail("runtime should have handled this already");
    } else if (0 == strcmp(name, "TwoClass")) {
        Class cls = objc_allocateClassPair([OrdinaryClass class], "TwoClass", 0);
        objc_registerClassPair(cls);
        *outClass = cls;
        return YES;
    } else {
        return TwoPreviousHook(name, outClass);
    }
}


objc_hook_getClass ThreePreviousHook;
static int HookThreeCalls = 0;
#define MAXDEPTH 100
BOOL GetClassHookThree(const char *name, Class *outClass)
{
    // Re-entrant hook test.
    // libobjc must prevent re-entrancy when a getClass
    // hook provokes another getClass call.

    static int depth = 0;
    static char *names[MAXDEPTH] = {0};

    if (depth < MAXDEPTH) {
        // Re-entrantly call objc_getClass() with a new class name.
        if (!names[depth]) asprintf(&names[depth], "Reentrant%d", depth);
        const char *reentrantName = names[depth];
        depth++;
        (void)objc_getClass(reentrantName);
        depth--;
    } else if (depth == MAXDEPTH) {
        // We now have maxdepth getClass hooks stacked up.
        // Call objc_getClass() on all of those names a second time.
        // None of those lookups should call this hook again.
        HookThreeCalls++;
        depth = -1;
        for (int i = 0; i < MAXDEPTH; i++) {
            testassert(!objc_getClass(names[i]));
        }
        depth = MAXDEPTH;
    } else {
        fail("reentrancy protection failed");
    }

    // Chain to the previous hook after all of the reentrancy is unwound.
    if (depth == 0) {
        return ThreePreviousHook(name, outClass);
    } else {
        return NO;
    }
}


void testLookup(const char *name, int expectedHookOneCalls,
                int expectedHookTwoCalls, int expectedHookThreeCalls)
{
    HookOneCalls = HookTwoCalls = HookThreeCalls = 0;
    Class cls = objc_getClass(name);
    testassert(HookOneCalls == expectedHookOneCalls  &&
               HookTwoCalls == expectedHookTwoCalls  &&
               HookThreeCalls == expectedHookThreeCalls);
    testassert(cls);
    testassert(0 == strcmp(class_getName(cls), name));
    testassert(cls == [cls self]);
}

int main()
{
    testassert(objc_getClass("OrdinaryClass"));
    testassert(!objc_getClass("OneClass"));
    testassert(!objc_getClass("TwoClass"));
    testassert(!objc_getClass("NoSuchClass"));

    objc_setHook_getClass(GetClassHookOne, &OnePreviousHook);
    objc_setHook_getClass(GetClassHookTwo, &TwoPreviousHook);
    objc_setHook_getClass(GetClassHookThree, &ThreePreviousHook);
    // invocation order: HookThree -> Hook Two -> Hook One

    HookOneCalls = HookTwoCalls = HookThreeCalls = 0;
    testassert(!objc_getClass("NoSuchClass"));
    testassert(HookOneCalls == 1 && HookTwoCalls == 1 && HookThreeCalls == 1);

    testLookup("OneClass", 1, 1, 1);
    testLookup("TwoClass", 0, 1, 1);
    testLookup("OrdinaryClass", 0, 0, 0);

    // Check again. No hooks should be needed this time.

    testLookup("OneClass", 0, 0, 0);
    testLookup("TwoClass", 0, 0, 0);
    testLookup("OrdinaryClass", 0, 0, 0);

    succeed(__FILE__);
}
