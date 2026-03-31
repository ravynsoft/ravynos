#include "unload.h"
#include "testroot.i"
#import <objc/objc-api.h>

@implementation SmallClass : TestRoot
-(void)unload2_instance_method { }
@end


@implementation BigClass : TestRoot
@end

OBJC_ROOT_CLASS
@interface UnusedClass { id isa; } @end
@implementation UnusedClass @end


@protocol SmallProtocol
-(void)unload2_category_method;
@end

@interface SmallClass (Category) <SmallProtocol> @end

@implementation SmallClass (Category)
-(void)unload2_category_method { }
@end

// This isn't really weak-import: we link with `-undefined dynamic_lookup`
// instead of providing a valid definition at link time.
// But it looks the same to the runtime.
__attribute__((weak_import))
@interface ClassThatIsWeakImportAndMissing : TestRoot @end

@interface SubclassOfMissingWeakImport : ClassThatIsWeakImportAndMissing <SmallProtocol> @end
@implementation SubclassOfMissingWeakImport
-(void)unload2_category_method { }
@end

@interface ClassThatIsWeakImportAndMissing (Category) <SmallProtocol> @end
@implementation ClassThatIsWeakImportAndMissing (Category)
-(void)unload2_category_method { }
@end
