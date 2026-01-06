#include <Foundation/Foundation.h>

#if PROTOCOLS
	@protocol myotherprotocol
	- (void) instance_method_myotherprotocol1;
	- (void) instance_method_myotherprotocol2;
	@end

	@interface NSObject(myothercat) < myotherprotocol >
	- (void) instance_method_myothercat;
	+ (void) class_method_myothercat;
	@end

	@implementation NSObject(myothercat)
	- (void) instance_method_myothercat {} 
	+ (void) class_method_myothercat {}
	- (void) instance_method_myotherprotocol1 {} 
	- (void) instance_method_myotherprotocol2 {}
	@end

#else
	@interface NSObject(myothercat)
	- (void) instance_method_myothercat;
	+ (void) class_method_myothercat;
	@end

	@implementation NSObject(myothercat)
	- (void) instance_method_myothercat {} 
	+ (void) class_method_myothercat {}
	@end
#endif


