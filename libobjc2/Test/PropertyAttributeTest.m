#include <stdio.h>
#import "Test.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

@protocol X
@optional
@property (readonly) int x;
@end

#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif

@interface helloclass <X> {
	@private int varName;
}
@property (class, retain) id clsProp;
@property (readwrite,assign) int propName;
@end

@implementation helloclass
@synthesize propName = varName;
+ (id)class { return self; }
+ (id)clsProp { return nil; }
+ (void)setClsProp: (id)arg {}
@end

int main()
{
	unsigned int outCount;
	objc_property_t *properties = class_copyPropertyList([helloclass class], &outCount);
	assert(outCount == 1);
	objc_property_t property = properties[0];
	assert(strcmp(property_getName(property), "propName") == 0);
	assert(strcmp(property_getAttributes(property), "Ti,VvarName") == 0);
	free(properties);

	properties = class_copyPropertyList(object_getClass([helloclass class]), &outCount);
	assert(outCount == 1);
	property = properties[0];
	assert(strcmp(property_getName(property), "clsProp") == 0);
	fprintf(stderr, "%s\n", property_getAttributes(property));
	assert(strcmp(property_getAttributes(property), "T@,&") == 0);
	free(properties);

	Method* methods = class_copyMethodList([helloclass class], &outCount);
	assert(outCount == 2);
	free(methods);

	objc_property_attribute_t a = { "V", "varName" };
	assert(class_addProperty([helloclass class], "propName2", &a, 1));
	properties = class_copyPropertyList([helloclass class], &outCount);
	assert(outCount == 2);
	int found = 0;
	for (int i=0 ; i<2 ; i++)
	{
		property = properties[i];
		fprintf(stderr, "Name: %s\n", property_getName(property));
		fprintf(stderr, "Attrs: %s\n", property_getAttributes(property));
		if (strcmp(property_getName(property), "propName2") == 0)
		{
			assert(strcmp(property_getAttributes(property), "VvarName") == 0);
			found++;
		}
	}
	assert(found == 1);
	return 0;
}


