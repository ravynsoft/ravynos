#if CATEGORY

#include <Foundation/Foundation.h>

@interface Foo : NSObject
@end

@interface Foo(mycat)
@property(readonly) int instanceProperty;
#if CLASS_PROPERTY
@property(class, readonly) int classProperty;
#endif
@end

@implementation Foo(mycat)
-(int) instanceProperty { return 0; }
+(int) classProperty { return 0; }
@end

#else

int x = 0; 

#endif
