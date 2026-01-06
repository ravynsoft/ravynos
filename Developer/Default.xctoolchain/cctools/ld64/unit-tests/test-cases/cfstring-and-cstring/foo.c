#include <CoreFoundation/CFString.h>

extern void bar();

int main() 
{
	CFStringGetLength(CFSTR("stuff"));
	bar();
	return 0; 
}


