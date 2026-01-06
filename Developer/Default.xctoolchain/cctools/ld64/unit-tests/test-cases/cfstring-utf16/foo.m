#include <CoreFoundation/CFString.h>

extern void bar();

void foo()
{
	CFStringGetLength(CFSTR("hello"));
	CFStringGetLength(CFSTR("überhund"));
}


int main() 
{
	CFStringGetLength(CFSTR("über"));
	CFStringGetLength(CFSTR("überhund"));
	bar();
	return 0; 
}


