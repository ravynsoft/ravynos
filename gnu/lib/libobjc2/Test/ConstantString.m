#import "Test.h"
#include <string.h>

@implementation NSConstantString (Test)
- (unsigned int)length
{
	return length;
}
- (const char*)cString
{
	return str;
}
@end


int main(void)
{
	assert([@"1234567890" length] == 10);
	assert(strcmp([@"123456789" cString], "123456789") == 0);
	return 0;
}
