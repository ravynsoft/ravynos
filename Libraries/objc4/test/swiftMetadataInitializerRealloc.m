/*
TEST_CONFIG MEM=mrc
TEST_BUILD
    $C{COMPILE} $DIR/swiftMetadataInitializerRealloc-dylib1.m -o libswiftMetadataInitializerRealloc-dylib1.dylib -dynamiclib -Wno-deprecated-objc-pointer-introspection
    $C{COMPILE} $DIR/swiftMetadataInitializerRealloc-dylib2.m -o libswiftMetadataInitializerRealloc-dylib2.dylib -dynamiclib -L. -lswiftMetadataInitializerRealloc-dylib1
    $C{COMPILE} $DIR/swiftMetadataInitializerRealloc.m -o swiftMetadataInitializerRealloc.exe -L. -lswiftMetadataInitializerRealloc-dylib1 -Wno-deprecated-objc-pointer-introspection
END
*/

#include "test.h"
#include "swift-class-def.m"


// _objc_swiftMetadataInitializer hooks for the classes in swift-class-def.m

Class initSuper(Class cls __unused, void *arg __unused)
{
    // This test provokes objc's callback out of superclass order.
    // SwiftSub's init is first. SwiftSuper's init is never called.

    fail("SwiftSuper's init should not have been called");
}

bool isRealized(Class cls)
{
    // check the is-realized bits directly

#if __LP64__
# define mask (~(uintptr_t)7)
#else
# define mask (~(uintptr_t)3)
#endif
#define RW_REALIZED (1<<31)
    
    uintptr_t rw = ((uintptr_t *)cls)[4] & mask;  // class_t->data
    return ((uint32_t *)rw)[0] & RW_REALIZED;  // class_rw_t->flags
}

SWIFT_CLASS(SwiftSuper, NSObject, initSuper);
SWIFT_CLASS(RealSwiftSub, SwiftSuper, initSub);

SWIFT_STUB_CLASS(SwiftSub, initSub);

OBJC_EXPORT _Nullable Class
objc_loadClassref(_Nullable Class * _Nonnull clsref);

static int SubInits = 0;
Class initSub(Class cls, void *arg)
{
    testprintf("initSub callback\n");
    
    testassert(SubInits == 0);
    SubInits++;
    testassert(arg == nil);
    testassert(cls == RawSwiftSub);
    testassert(!isRealized(RawSwiftSuper));

    // Copy the class to the heap to ensure they're registered properly.
    // Classes in the data segment are automatically "known" even if not
    // added as a known class. Swift dynamically allocates classes from
    // a statically allocated space in the dylib, then allocates from
    // the heap after it runs out of room there. Code that only works
    // when the class is in a dylib can fail a long time down the road
    // when something finally exceeds the capacity of that space.
    // Example: rdar://problem/50707074
    Class HeapSwiftSub = (Class)malloc(OBJC_MAX_CLASS_SIZE);
    memcpy(HeapSwiftSub, RawRealSwiftSub, OBJC_MAX_CLASS_SIZE);

    testprintf("initSub beginning _objc_realizeClassFromSwift\n");
    _objc_realizeClassFromSwift(HeapSwiftSub, cls);
    testprintf("initSub finished  _objc_realizeClassFromSwift\n");

    testassert(isRealized(RawSwiftSuper));
    testassert(isRealized(HeapSwiftSub));
    
    testprintf("Returning reallocated class %p\n", HeapSwiftSub);
    
    return HeapSwiftSub;
}


@interface SwiftSub (Addition)
- (int)number;
@end
@implementation SwiftSub (Addition)
- (int)number { return 42; }
@end

@interface NSObject (DylibCategories)
- (const char *)dylib1ACategoryInSameDylib;
- (const char *)dylib1BCategoryInSameDylib;
- (const char *)dylib1ACategoryInOtherDylib;
- (const char *)dylib1BCategoryInOtherDylib;
- (const char *)dylib1ACategoryInApp;
- (const char *)dylib1BCategoryInApp;
+ (const char *)dylib1ACategoryInAppClassMethod;
+ (const char *)dylib1BCategoryInAppClassMethod;
+ (void)testFromOtherDylib;
@end

extern int Dylib1AInits;
extern int Dylib1BInits;
SWIFT_STUB_CLASSREF(SwiftDylib1A);
SWIFT_STUB_CLASSREF(SwiftDylib1B);
void Dylib1Test(void);

@interface SwiftDylib1A: NSObject @end
@interface SwiftDylib1B: NSObject @end

@implementation SwiftDylib1A (Category)
- (const char *)dylib1ACategoryInApp { return "dylib1ACategoryInApp"; }
+ (const char *)dylib1ACategoryInAppClassMethod { return "dylib1ACategoryInAppClassMethod"; }
@end
@implementation SwiftDylib1B (Category)
- (const char *)dylib1BCategoryInApp { return "dylib1BCategoryInApp"; }
+ (const char *)dylib1BCategoryInAppClassMethod { return "dylib1BCategoryInAppClassMethod"; }
@end


int main()
{
#define LOG(fmt, expr) testprintf(#expr " is " #fmt "\n", expr);
    LOG(%p, SwiftSubClassref);
    Class loadedSwiftSub = objc_loadClassref(&SwiftSubClassref);
    LOG(%p, SwiftSubClassref);
    LOG(%p, loadedSwiftSub);
    LOG(%p, [loadedSwiftSub class]);
    LOG(%p, [loadedSwiftSub superclass]);
    LOG(%p, [RawSwiftSuper class]);
    
    id obj = [[loadedSwiftSub alloc] init];
    LOG(%p, obj);
    LOG(%d, [obj number]);
    
    LOG(%p, SwiftDylib1AClassref);
    testassert(Dylib1AInits == 0);
    testassert((uintptr_t)SwiftDylib1AClassref & 1);
    Class SwiftDylib1A = objc_loadClassref(&SwiftDylib1AClassref);
    testassert(((uintptr_t)SwiftDylib1AClassref & 1) == 0);
    testassert(SwiftDylib1A == [SwiftDylib1A class]);
    testassert(SwiftDylib1A == SwiftDylib1AClassref);
    testassert(Dylib1AInits == 1);
    LOG(%p, SwiftDylib1A);
    
    LOG(%p, SwiftDylib1BClassref);
    testassert(Dylib1BInits == 0);
    testassert((uintptr_t)SwiftDylib1BClassref & 1);
    Class SwiftDylib1B = objc_loadClassref(&SwiftDylib1BClassref);
    testassert(((uintptr_t)SwiftDylib1BClassref & 1) == 0);
    testassert(SwiftDylib1B == [SwiftDylib1B class]);
    testassert(SwiftDylib1B == SwiftDylib1BClassref);
    testassert(Dylib1BInits == 1);
    LOG(%p, SwiftDylib1B);
    
    Dylib1Test();
    
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInSameDylib], "dylib1ACategoryInSameDylib") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInSameDylib], "dylib1BCategoryInSameDylib") == 0);
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInApp], "dylib1ACategoryInApp") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInApp], "dylib1BCategoryInApp") == 0);
    
    void *handle = dlopen("libswiftMetadataInitializerRealloc-dylib2.dylib", RTLD_LAZY);
    testassert(handle);
    
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInOtherDylib], "dylib1ACategoryInOtherDylib") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInOtherDylib], "dylib1BCategoryInOtherDylib") == 0);
    testassert(strcmp([SwiftDylib1A dylib1ACategoryInAppClassMethod], "dylib1ACategoryInAppClassMethod") == 0);
    testassert(strcmp([SwiftDylib1B dylib1BCategoryInAppClassMethod], "dylib1BCategoryInAppClassMethod") == 0);
    [SwiftDylib1A testFromOtherDylib];
    
    testassert(objc_getClass("RealSwiftSub"));
    testassert(objc_getClass("RealSwiftDylib1A"));
    testassert(objc_getClass("RealSwiftDylib1B"));
    
    succeed(__FILE__);
}
