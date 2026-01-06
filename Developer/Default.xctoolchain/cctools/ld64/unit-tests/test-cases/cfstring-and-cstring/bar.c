#include <CoreFoundation/CFString.h>


//
// llvm may make cfstring that has backing store of kTest
//


const char kTest[] = "test";

void bar()
{
	CFStringGetLength(CFSTR("test"));
}
