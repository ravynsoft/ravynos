#include <Foundation/Foundation.h>

@interface Foo : NSObject
-(void) method1;
@end

@interface Foo(fromcat)
- (void) instance_method_fromcat;
+ (void) class_method_fromcat;
#if OVERRIDE_CLASS
- (void) instance_method;
+ (void) class_method;
#endif
@end

@implementation Foo(fromcat)
- (void) instance_method_fromcat {} 
+ (void) class_method_fromcat {}
#if OVERRIDE_CLASS
- (void) instance_method {}
+ (void) class_method {}
#endif
@end

