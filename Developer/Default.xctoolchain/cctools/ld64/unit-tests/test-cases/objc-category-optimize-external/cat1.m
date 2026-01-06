#include <Foundation/Foundation.h>



@interface NSObject(mycat)
-(void) instance_method_mycat1;
-(void) instance_method_mycat2;
+(void) class_method_mycat;
#if PROPERTIES
	@property(readonly) int property1;
	@property(readonly) int property2;
	@property(class,readonly) int property3;
	@property(class,readonly) int property4;
#endif
@end

@implementation NSObject(mycat)
-(void) instance_method_mycat1 {}
-(void) instance_method_mycat2 {}
+(void) class_method_mycat {}
#if PROPERTIES
	-(int) property1 { return 0; }
	-(int) property2 { return 0; }
	+(int) property3 { return 0; }
	+(int) property4 { return 0; }
#endif
@end

