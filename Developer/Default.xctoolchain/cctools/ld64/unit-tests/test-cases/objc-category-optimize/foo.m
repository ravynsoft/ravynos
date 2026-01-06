#include <Foundation/Foundation.h>

@interface Foo : NSObject
#ifndef NO_BASE_METHODS
-(void) instance_method;
+(void) class_method;
#endif
@end


@implementation Foo
#ifndef NO_BASE_METHODS
-(void) instance_method {}
+(void) class_method {}
#endif
@end

