#pragma clang diagnostic ignored "-Wcomment"
/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/* .
    $C{COMPILE} $DIR/gcenforcer-preflight.m -o gcenforcer-preflight.exe
END
*/

#include "test.h"
#include <dlfcn.h>

void check(int expected, const char *name)
{
    int fd = open(name, O_RDONLY);
    testassert(fd >= 0);

    int result = objc_appRequiresGC(fd);

    close(fd);
    testprintf("want %2d got %2d for %s\n", expected, result, name);
    if (result != expected) {
        fail("want %2d got %2d for %s\n", expected, result, name);
    }
    testassert(result == expected);
}

int main()
{
    int i;
    for (i = 0; i < 1000; i++) {
        // dlopen_preflight

        testassert(dlopen_preflight("libsupportsgc.dylib"));
        testassert(dlopen_preflight("libnoobjc.dylib"));
        testassert(! dlopen_preflight("librequiresgc.dylib"));
        testassert(dlopen_preflight("libnogc.dylib"));

        // objc_appRequiresGC

        // noobjc: no ObjC content
        // nogc:   ordinary not GC
        // aso:    trivial AppleScriptObjC wrapper that can run without GC
        // gc:     -fobjc-gc
        // gconly: -fobjc-gc-only
        // gcaso:  non-trivial AppleScriptObjC with too many classrefs
        // gcaso2: non-trivial AppleScriptObjC with too many class impls

        check(0, "x86_64-noobjc");
        check(0, "x86_64-nogc");
        check(0, "x86_64-aso");
        check(1, "x86_64-gc");
        check(1, "x86_64-gconly");
        check(1, "x86_64-gcaso");
        check(1, "x86_64-gcaso2");
        
        check(0, "i386-noobjc");
        check(0, "i386-nogc");
        check(0, "i386-aso");
        check(1, "i386-gc");
        check(1, "i386-gconly");
        check(1, "i386-gcaso");
        check(1, "i386-gcaso2");
        
        // fat files
        check(0, "i386-aso--x86_64-aso");
        check(0, "i386-nogc--x86_64-nogc");
        check(1, "i386-gc--x86_64-gc");
        check(1, "i386-gc--x86_64-nogc");
        check(1, "i386-nogc--x86_64-gc");
        
        // broken files
        check(-1, "x86_64-broken");
        check(-1, "i386-broken");
        check(-1, "i386-broken--x86_64-gc");
        check(-1, "i386-broken--x86_64-nogc");
        check(-1, "i386-gc--x86_64-broken");
        check(-1, "i386-nogc--x86_64-broken");   

        // evil files
        // evil1:   claims to have 4 billion load commands of size 0
        check(-1, "evil1");
    }

    succeed(__FILE__);
}
