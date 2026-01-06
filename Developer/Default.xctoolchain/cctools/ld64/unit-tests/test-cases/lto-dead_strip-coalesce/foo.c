
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>

void foo1()
{
	CFStringGetLength(CFSTR("test1"));
	strlen("str1");
}

void foo2()
{
	CFStringGetLength(CFSTR("test2"));
	strlen("str2");
}

void foo3()
{
	CFStringGetLength(CFSTR("test3"));
	strlen("str3");
}
