#include "Test.h"
#include <string.h>
#include <stdio.h>

@interface Test (Property)
@property (readonly) int val;
@property (class, readonly) int val2;
@end

@interface Test (Property2)
@property (readonly) int val2;
@end


@implementation Test (Property)
@dynamic val2;
- (int)val { return 0; }
@end

@implementation Test (Property2)
@dynamic val2;
- (int)val2 { return 0; }
@end

int main(int argc, char** argv)
{
	Class test = objc_getClass("Test");
	objc_property_t prop = class_getProperty(test, "val");
	assert(prop);
	assert(strcmp("Ti,R", property_getAttributes(prop)) == 0);
	prop = class_getProperty(test, "val2");
	assert(prop);
	assert(strcmp("Ti,R,D", property_getAttributes(prop)) == 0);
#ifdef GS_RUNTIME_V2
	test = object_getClass(test);
	prop = class_getProperty(test, "val2");
	assert(prop);
	assert(strcmp("Ti,R,D", property_getAttributes(prop)) == 0);
#endif
}

