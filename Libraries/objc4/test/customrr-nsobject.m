// This file is used in the customrr-nsobject-*.m tests

#include "test.h"
#include <objc/NSObject.h>
#include <objc/objc-internal.h>

#if __has_feature(ptrauth_calls)
typedef IMP __ptrauth_objc_method_list_imp MethodListIMP;
#else
typedef IMP MethodListIMP;
#endif

static int Retains;
static int Releases;
static int Autoreleases;
static int PlusInitializes;
static int Allocs;
static int AllocWithZones;
static int Inits;
static int PlusNew;
static int Self;
static int PlusSelf;

id (*RealRetain)(id self, SEL _cmd);
void (*RealRelease)(id self, SEL _cmd);
id (*RealAutorelease)(id self, SEL _cmd);
id (*RealAlloc)(id self, SEL _cmd);
id (*RealAllocWithZone)(id self, SEL _cmd, void *zone);
id (*RealPlusNew)(id self, SEL _cmd);
id (*RealSelf)(id self);
id (*RealPlusSelf)(id self);

id HackRetain(id self, SEL _cmd) { Retains++; return RealRetain(self, _cmd); }
void HackRelease(id self, SEL _cmd) { Releases++; return RealRelease(self, _cmd); }
id HackAutorelease(id self, SEL _cmd) { Autoreleases++; return RealAutorelease(self, _cmd); }

id HackAlloc(Class self, SEL _cmd) { Allocs++; return RealAlloc(self, _cmd); }
id HackAllocWithZone(Class self, SEL _cmd, void *zone) { AllocWithZones++; return RealAllocWithZone(self, _cmd, zone); }

void HackPlusInitialize(id self __unused, SEL _cmd __unused) { PlusInitializes++; }

id HackInit(id self, SEL _cmd __unused) { Inits++; return self; }

id HackPlusNew(id self, SEL _cmd __unused) { PlusNew++; return RealPlusNew(self, _cmd); }
id HackSelf(id self) { Self++; return RealSelf(self); }
id HackPlusSelf(id self) { PlusSelf++; return RealPlusSelf(self); }


int main(int argc __unused, char **argv)
{
    Class cls = objc_getClass("NSObject");
    Method meth;

    meth = class_getClassMethod(cls, @selector(initialize));
    method_setImplementation(meth, (IMP)HackPlusInitialize);

    // We either swizzle the method normally (testing that it properly 
    // disables optimizations), or we hack the implementation into place 
    // behind objc's back (so we can see whether it got called with the 
    // optimizations still enabled).

    meth = class_getClassMethod(cls, @selector(allocWithZone:));
    RealAllocWithZone = (typeof(RealAllocWithZone))method_getImplementation(meth);
#if SWIZZLE_AWZ
    method_setImplementation(meth, (IMP)HackAllocWithZone);
#else
    ((MethodListIMP *)meth)[2] = (IMP)HackAllocWithZone;
#endif

    meth = class_getClassMethod(cls, @selector(new));
    RealPlusNew = (typeof(RealPlusNew))method_getImplementation(meth);
#if SWIZZLE_CORE
    method_setImplementation(meth, (IMP)HackPlusNew);
#else
    ((MethodListIMP *)meth)[2] = (IMP)HackPlusNew;
#endif

    meth = class_getClassMethod(cls, @selector(self));
    RealPlusSelf = (typeof(RealPlusSelf))method_getImplementation(meth);
#if SWIZZLE_CORE
    method_setImplementation(meth, (IMP)HackPlusSelf);
#else
    ((MethodListIMP *)meth)[2] = (IMP)HackPlusSelf;
#endif

    meth = class_getInstanceMethod(cls, @selector(self));
    RealSelf = (typeof(RealSelf))method_getImplementation(meth);
#if SWIZZLE_CORE
    method_setImplementation(meth, (IMP)HackSelf);
#else
    ((MethodListIMP *)meth)[2] = (IMP)HackSelf;
#endif

    meth = class_getInstanceMethod(cls, @selector(release));
    RealRelease = (typeof(RealRelease))method_getImplementation(meth);
#if SWIZZLE_RELEASE
    method_setImplementation(meth, (IMP)HackRelease);
#else
    ((MethodListIMP *)meth)[2] = (IMP)HackRelease;
#endif

    // These other methods get hacked for counting purposes only

    meth = class_getInstanceMethod(cls, @selector(retain));
    RealRetain = (typeof(RealRetain))method_getImplementation(meth);
    ((MethodListIMP *)meth)[2] = (IMP)HackRetain;

    meth = class_getInstanceMethod(cls, @selector(autorelease));
    RealAutorelease = (typeof(RealAutorelease))method_getImplementation(meth);
    ((MethodListIMP *)meth)[2] = (IMP)HackAutorelease;

    meth = class_getClassMethod(cls, @selector(alloc));
    RealAlloc = (typeof(RealAlloc))method_getImplementation(meth);
    ((MethodListIMP *)meth)[2] = (IMP)HackAlloc;

    meth = class_getInstanceMethod(cls, @selector(init));
    ((MethodListIMP *)meth)[2] = (IMP)HackInit;

    // Verify that the swizzles occurred before +initialize by provoking it now
    testassert(PlusInitializes == 0);
    [NSObject self];
    testassert(PlusInitializes == 1);

    id obj;
    id result;

    Allocs = 0;
    AllocWithZones = 0;
    Inits = 0;
    obj = objc_alloc(cls);
#if SWIZZLE_AWZ
    testprintf("swizzled AWZ should be called\n");
    testassert(Allocs == 1);
    testassert(AllocWithZones == 1);
    testassert(Inits == 0);
#else
    testprintf("unswizzled AWZ should be bypassed\n");
    testassert(Allocs == 0);
    testassert(AllocWithZones == 0);
    testassert(Inits == 0);
#endif
    testassert([obj isKindOfClass:[NSObject class]]);

    Allocs = 0;
    AllocWithZones = 0;
    Inits = 0;
    obj = [NSObject alloc];
#if SWIZZLE_AWZ
    testprintf("swizzled AWZ should be called\n");
    testassert(Allocs == 1);
    testassert(AllocWithZones == 1);
    testassert(Inits == 0);
#else
    testprintf("unswizzled AWZ should be bypassed\n");
    testassert(Allocs == 1);
    testassert(AllocWithZones == 0);
    testassert(Inits == 0);
#endif
    testassert([obj isKindOfClass:[NSObject class]]);

    Allocs = 0;
    AllocWithZones = 0;
    Inits = 0;
    obj = objc_alloc_init(cls);
#if SWIZZLE_AWZ
    testprintf("swizzled AWZ should be called\n");
    testassert(Allocs == 1);
    testassert(AllocWithZones == 1);
    testassert(Inits == 1);
#else
    testprintf("unswizzled AWZ should be bypassed\n");
    testassert(Allocs == 0);
    testassert(AllocWithZones == 0);
    testassert(Inits == 1);  // swizzled init is still called
#endif
    testassert([obj isKindOfClass:[NSObject class]]);

    Retains = 0;
    result = objc_retain(obj);
#if SWIZZLE_RELEASE
    testprintf("swizzled release should force retain\n");
    testassert(Retains == 1);
#else
    testprintf("unswizzled release should bypass retain\n");
    testassert(Retains == 0);
#endif
    testassert(result == obj);

    Releases = 0;
    Autoreleases = 0;
    PUSH_POOL {
        result = objc_autorelease(obj);
#if SWIZZLE_RELEASE
        testprintf("swizzled release should force autorelease\n");
        testassert(Autoreleases == 1);
#else
        testprintf("unswizzled release should bypass autorelease\n");
        testassert(Autoreleases == 0);
#endif
        testassert(result == obj);
    } POP_POOL

#if SWIZZLE_RELEASE
    testprintf("swizzled release should be called\n");
    testassert(Releases == 1);
#else
    testprintf("unswizzled release should be bypassed\n");
    testassert(Releases == 0);
#endif

    PlusNew = 0;
    Self = 0;
    PlusSelf = 0;
    Class nso = objc_opt_self([NSObject class]);
    obj = objc_opt_new(nso);
    obj = objc_opt_self(obj);
#if SWIZZLE_CORE
    testprintf("swizzled Core should be called\n");
    testassert(PlusNew == 1);
    testassert(Self == 1);
    testassert(PlusSelf == 1);
#else
    testprintf("unswizzled CORE should be bypassed\n");
    testassert(PlusNew == 0);
    testassert(Self == 0);
    testassert(PlusSelf == 0);
#endif
    testassert([obj isKindOfClass:nso]);

    succeed(basename(argv[0]));
}
