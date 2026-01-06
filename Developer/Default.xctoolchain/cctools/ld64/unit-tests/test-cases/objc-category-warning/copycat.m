#include <Foundation/Foundation.h>

@interface Foo : NSObject
-(void) method1;
@end

@interface Foo(copycat)
- (void) instance_method_fromcat;
+ (void) class_method_fromcat;
@end

@implementation Foo(copycat)
- (void) instance_method_fromcat {} 
+ (void) class_method_fromcat {}
@end

