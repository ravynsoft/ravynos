#include <Foundation/Foundation.h>

@interface Foo : NSObject
-(void) method1;
@end

#if PROTOCOLS
	@protocol myotherprotocol
	- (void) instance_method_myotherprotocol1;
	- (void) instance_method_myotherprotocol2;
	@end

	@interface Foo(myothercat) < myotherprotocol >
	- (void) instance_method_myothercat;
	+ (void) class_method_myothercat;
	@end

	@implementation Foo(myothercat)
	- (void) instance_method_myothercat {} 
	+ (void) class_method_myothercat {}
	- (void) instance_method_myotherprotocol1 {} 
	- (void) instance_method_myotherprotocol2 {}
	@end

#else
	@interface Foo(myothercat)
	- (void) instance_method_myothercat;
	+ (void) class_method_myothercat;
	@end

	@implementation Foo(myothercat)
	- (void) instance_method_myothercat {} 
	+ (void) class_method_myothercat {}
	@end
#endif


