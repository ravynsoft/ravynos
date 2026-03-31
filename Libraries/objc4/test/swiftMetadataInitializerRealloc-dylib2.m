#include "test.h"
#include "swift-class-def.m"

@interface SwiftDylib1A: NSObject @end
@interface SwiftDylib1B: NSObject @end

@interface NSObject (DylibCategories)
- (char *)dylib1ACategoryInSameDylib;
- (char *)dylib1BCategoryInSameDylib;
- (char *)dylib1ACategoryInOtherDylib;
- (char *)dylib1BCategoryInOtherDylib;
- (char *)dylib1ACategoryInApp;
- (char *)dylib1BCategoryInApp;
+ (void)testFromOtherDylib;
@end

@implementation SwiftDylib1A (Category)
- (const char *)dylib1ACategoryInOtherDylib { return "dylib1ACategoryInOtherDylib"; }
@end
@implementation SwiftDylib1B (Category)
- (const char *)dylib1BCategoryInOtherDylib { return "dylib1BCategoryInOtherDylib"; }
@end

SWIFT_STUB_CLASSREF(SwiftDylib1A);
SWIFT_STUB_CLASSREF(SwiftDylib1B);

Class objc_loadClassref(_Nullable Class * _Nonnull clsref);

@implementation SwiftDylib1A (Test)
+ (void)testFromOtherDylib {
    Class SwiftDylib1A = objc_loadClassref(&SwiftDylib1AClassref);
    Class SwiftDylib1B = objc_loadClassref(&SwiftDylib1BClassref);
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInSameDylib], "dylib1ACategoryInSameDylib") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInSameDylib], "dylib1BCategoryInSameDylib") == 0);
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInApp], "dylib1ACategoryInApp") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInApp], "dylib1BCategoryInApp") == 0);
    testassert(strcmp([[SwiftDylib1A new] dylib1ACategoryInOtherDylib], "dylib1ACategoryInOtherDylib") == 0);
    testassert(strcmp([[SwiftDylib1B new] dylib1BCategoryInOtherDylib], "dylib1BCategoryInOtherDylib") == 0);
}
@end
