#include <CoreFoundation/CFString.h>

CFStringRef OtherCFString = CFSTR("other");

void bar()
{
	CFStringGetLength(CFSTR("live"));
}
