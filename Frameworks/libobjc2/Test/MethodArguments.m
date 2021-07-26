#import "Test.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface Foo
-(id)bar;
-(void)setBar:(id)b;
@end 
@implementation Foo
- (id)bar
{
	return nil;
}

- (void)setBar: (id)b
{
	return;
}
@end

int main(void)
{
	Class foo = objc_getClass("Foo");
	Method barMethod = class_getInstanceMethod(foo, @selector(bar));
	Method setBarMethod = class_getInstanceMethod(foo,@selector(setBar:));
	char arg[16];

	memset(&arg[0], '\0', 16 * sizeof(char));
	method_getReturnType(barMethod, &arg[0], 16);
	assert(0 == strcmp(&arg[0],"@"));

	char* expected[3] = {"@", ":", "" };
	for (int i = 0; i < 3; i++)
	{
	  memset(&arg[0], '\0', 16 * sizeof(char));
	  method_getArgumentType(barMethod, i, &arg[0], 16);
	  assert(0 == strcmp(&arg[0],expected[i]));
	}


	memset(&arg[0], '\0', 16 * sizeof(char));
	method_getReturnType(setBarMethod, &arg[0], 16);
	assert(0 == strcmp(&arg[0],"v"));

        expected[2] = "@";

	for (int i = 0; i < 3; i++)
	{
	  memset(&arg[0], '\0', 16 * sizeof(char));
	  method_getArgumentType(setBarMethod, i, &arg[0], 16);
	  assert(0 == strcmp(&arg[0],expected[i]));
	}

	char *arg_copied = method_copyReturnType(barMethod);
	assert(0 == strcmp(arg_copied,"@"));
	free(arg_copied);
	arg_copied = NULL;

	for (int i = 0; i < 2; i++) 
	{
	  arg_copied = method_copyArgumentType(barMethod, i);
	  assert(0 == strcmp(arg_copied,expected[i]));
	  free(arg_copied);
	}

	arg_copied = method_copyArgumentType(barMethod, 2);
	assert(NULL == arg_copied);



	arg_copied = method_copyReturnType(setBarMethod);
	assert(0 == strcmp(arg_copied,"v"));
	free(arg_copied);

	for (int i = 0; i < 3; i++) 
	{
	  arg_copied = method_copyArgumentType(setBarMethod, i);
	  assert(0 == strcmp(arg_copied,expected[i]));
	  free(arg_copied);
	}


	return 0;
}
