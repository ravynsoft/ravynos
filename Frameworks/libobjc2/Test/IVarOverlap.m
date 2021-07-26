#import <stdio.h>
#import "../objc/runtime.h"
#include "Test.h"

#import <sys/types.h>
#import <sys/stat.h>

@interface Dummy : Test
{
	id objOne;
	struct stat statBuf;
	BOOL flagOne;
}
@end

@implementation Dummy
- (void)test
{
	assert((char*)&statBuf+sizeof(struct stat) <= (char*)&flagOne);
}
@end


int main(int argc, char *argv[])
{
	[[Dummy new] test];
	return 0;
}
