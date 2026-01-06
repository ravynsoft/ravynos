#include <Foundation/Foundation.h>

@interface Foo : NSObject
-(void) method1;
@end


@interface Foo(mycat)
+(void) load;
@end

@implementation Foo(mycat)
+(void) load {}
@end

