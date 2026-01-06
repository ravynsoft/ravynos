#include <CoreFoundation/CFString.h>

extern void bar();

extern CFStringRef OtherCFString;

void foo()
{
	CFStringGetLength(CFSTR("hello"));
	CFStringGetLength(CFSTR("world"));
	CFStringGetLength(OtherCFString);
}


int main() 
{
	CFStringGetLength(CFSTR("live"));
	bar();
	return 0; 
}


