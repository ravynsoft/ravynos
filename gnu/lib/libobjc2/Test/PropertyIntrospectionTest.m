#import "Test.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface Foo
@property (getter=bar, setter=setBar:, nonatomic, copy) id foo;
@end
@interface Foo(Bar)
-(id)bar;
-(void)setBar:(id)b;
@end 
@implementation Foo
@synthesize  foo;
@end

int main(void)
{
	objc_property_t p = class_getProperty(objc_getClass("Foo"), "foo");
	unsigned int count;
	objc_property_attribute_t *l = property_copyAttributeList(p, &count);
	for (unsigned int i=0 ; i<count ; i++)
	{
		switch (l[i].name[0])
		{
			case 'T': assert(0==strcmp(l[i].value, "@")); break;
			case 'C': assert(0==strcmp(l[i].value, "")); break;
			case 'N': assert(0==strcmp(l[i].value, "")); break;
			case 'G': assert(0==strcmp(l[i].value, "bar")); break;
			case 'S': assert(0==strcmp(l[i].value, "setBar:")); break;
			case 'B': assert(0==strcmp(l[i].value, "foo")); break;
		}
	}
	assert(0 == property_copyAttributeList(0, &count));
	return 0;
}
