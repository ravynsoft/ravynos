#include "test.h"
#include "swift-class-def.m"

SWIFT_CLASS(RealSwiftDylib1A, NSObject, nop);
SWIFT_STUB_CLASS(SwiftDylib1A, initSwiftDylib1A);

SWIFT_CLASS(RealSwiftDylib1B, NSObject, nop);
SWIFT_STUB_CLASS(SwiftDylib1B, initSwiftDylib1B);

int Dylib1AInits = 0;

@interface SwiftDylib1A: NSObject @end
@interface SwiftDylib1B: NSObject @end

@implementation SwiftDylib1A (Category)
- (const char *)dylib1ACategoryInSameDylib { return "dylib1ACategoryInSameDylib"; }
@end
@implementation SwiftDylib1B (Category)
- (const char *)dylib1BCategoryInSameDylib { return "dylib1BCategoryInSameDylib"; }
@end

EXTERN_C Class initSwiftDylib1A(Class cls, void *arg)
{
    Dylib1AInits++;
    testassert(arg == nil);
    testassert(cls == RawSwiftDylib1A);
    
    if (Dylib1AInits == 1)
        _objc_realizeClassFromSwift(RawRealSwiftDylib1A, cls);
    
    return RawRealSwiftDylib1A;
}

int Dylib1BInits = 0;

EXTERN_C Class initSwiftDylib1B(Class cls, void *arg)
{
    Dylib1BInits++;
    testassert(arg == nil);
    testassert(cls == RawSwiftDylib1B);
    
    if (Dylib1BInits == 1)
        _objc_realizeClassFromSwift(RawRealSwiftDylib1B, cls);
    
    return RawRealSwiftDylib1B;
}

EXTERN_C Class objc_loadClassref(_Nullable Class * _Nonnull clsref);

void Dylib1Test(void) {
    testassert((uintptr_t)SwiftDylib1AClassref & 1);
    Class SwiftDylib1A = objc_loadClassref(&SwiftDylib1AClassref);
    testassert(((uintptr_t)SwiftDylib1AClassref & 1) == 0);
    testassert(SwiftDylib1A == [SwiftDylib1A class]);
    testassert(SwiftDylib1A == SwiftDylib1AClassref);
    testassert(Dylib1AInits == 2);
    
    testassert((uintptr_t)SwiftDylib1BClassref & 1);
    Class SwiftDylib1B = objc_loadClassref(&SwiftDylib1BClassref);
    testassert(((uintptr_t)SwiftDylib1BClassref & 1) == 0);
    testassert(SwiftDylib1B == [SwiftDylib1B class]);
    testassert(SwiftDylib1B == SwiftDylib1BClassref);
    testassert(Dylib1BInits == 2);
}
