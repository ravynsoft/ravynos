#import "Test.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

@protocol Test2 @end
@protocol Test3 @end
@protocol Test4 @end

void checkProtocolMethod(Protocol *p, SEL sel, BOOL isClass, BOOL isOptional)
{
	struct objc_method_description d = protocol_getMethodDescription(p, sel, isClass, isOptional);
	assert(sel_isEqual(d.name, sel));
	assert(strcmp((d.types), "@:") == 0);
	d = protocol_getMethodDescription(p, sel, !isClass, isOptional);
	assert(d.name == NULL);
	assert(d.types == NULL);
	d = protocol_getMethodDescription(p, sel, isClass, !isOptional);
	assert(d.name == NULL);
	assert(d.types == NULL);
	d = protocol_getMethodDescription(p, sel, !isClass, !isOptional);
	assert(d.name == NULL);
	assert(d.types == NULL);
}

int main(void)
{
	__attribute__((unused))
	Protocol *force_reference = @protocol(Test2);
	Protocol *p = objc_allocateProtocol("Test");
	protocol_addMethodDescription(p, @selector(someClassMethod), "@:", YES, NO);
	protocol_addMethodDescription(p, @selector(someOptionalClassMethod), "@:", YES, YES);
	protocol_addMethodDescription(p, @selector(someMethod), "@:", NO, NO);
	protocol_addMethodDescription(p, @selector(someOtherMethod), "@:", NO, NO);
	protocol_addMethodDescription(p, @selector(someOptionalMethod), "@:", NO, YES);
	assert(objc_getProtocol("Test2"));
	protocol_addProtocol(p, objc_getProtocol("Test2"));
	protocol_addProtocol(p, @protocol(Test3));

	// Check that this don't crash
	protocol_addProtocol(p, NULL);
	protocol_addProtocol(NULL, p);
	protocol_addProtocol(NULL, NULL);

	objc_property_attribute_t attrs[] = { {"T", "@" }, {"V", "optional"} };
	protocol_addProperty(p, "optional", attrs, 2, NO, YES);
	attrs[1].value = "required";
	protocol_addProperty(p, "required", attrs, 2, YES, YES);
	attrs[1].value = "required2";
	protocol_addProperty(p, "required2", attrs, 2, YES, YES);
	protocol_addProperty(p, "classOptional", attrs, 1, NO, NO);
	protocol_addProperty(p, "classRequired", attrs, 1, YES, NO);

	checkProtocolMethod(p, @selector(someClassMethod), YES, NO);
	checkProtocolMethod(p, @selector(someOptionalClassMethod), YES, YES);
	checkProtocolMethod(p, @selector(someMethod), NO, NO);
	checkProtocolMethod(p, @selector(someOtherMethod), NO, NO);
	checkProtocolMethod(p, @selector(someOptionalMethod), NO, YES);
	objc_registerProtocol(p);
	// Modifying protocols after they've been registered is not permitted.
	protocol_addProtocol(p, @protocol(Test4));
	protocol_addMethodDescription(p, @selector(someUnsupportedMethod), "@:", NO, NO);
	protocol_addProperty(p, "classRequired2", attrs, 1, NO, NO);

	Protocol *p1 = objc_getProtocol("Test");
	assert(p == p1);

	checkProtocolMethod(p1, @selector(someClassMethod), YES, NO);
	checkProtocolMethod(p1, @selector(someOptionalClassMethod), YES, YES);
	checkProtocolMethod(p1, @selector(someMethod), NO, NO);
	checkProtocolMethod(p1, @selector(someOtherMethod), NO, NO);
	checkProtocolMethod(p1, @selector(someOptionalMethod), NO, YES);
	// Added after the protocol was registered, shouldn't have been allowed.
	struct objc_method_description d = protocol_getMethodDescription(p1, @selector(someUnsupportedMethod), NO, NO);
	assert(d.name == NULL);
	assert(d.types == NULL);

	assert(protocol_conformsToProtocol(p1, objc_getProtocol("Test2")));
	assert(protocol_conformsToProtocol(p1, objc_getProtocol("Test3")));
	// Added after the protocol was registered, shouldn't have been allowed.
	assert(!protocol_conformsToProtocol(p1, objc_getProtocol("Test4")));
	unsigned int count;
	protocol_copyPropertyList(p1, &count);
	assert(count == 2);
	protocol_copyPropertyList2(p1, &count, YES, YES);
	assert(count == 2);
	objc_property_t *props = protocol_copyPropertyList2(p1, &count, NO, YES);
	assert(count == 1);
	assert(strcmp("T@,Voptional", property_getAttributes(*props)) == 0);


	Protocol **list = objc_copyProtocolList(&count);
	assert(count >= 4);
	Protocol *expected[4] = {@protocol(Test2), @protocol(Test3), @protocol(Test4), p};
	const char *expectedNames[4] = {"Test2", "Test3", "Test4", "Test"};
	BOOL found[4];
	for (unsigned i=0 ; i<count ; i++)
	{
		Protocol *f = list[i];
		for (int j=0 ; j<4 ; j++)
		{
			if (strcmp(expectedNames[j], protocol_getName(f)) == 0)
			{
				assert(protocol_isEqual(f, expected[j]));
				found[j] = YES;
			}
		}
	}
	for (int j=0 ; j<4 ; j++)
	{
		assert(found[j]);
	}

	return 0;
}
