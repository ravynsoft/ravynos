
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>


extern void foo1();
extern void foo2();


void t1()
{
	CFStringGetLength(CFSTR("test1"));
	strlen("str1");
}

void t2()
{
	CFStringGetLength(CFSTR("test2"));
	strlen("str2");
}

void t3()
{
	CFStringGetLength(CFSTR("test3"));
	strlen("str3");
}

// <rdar://problem/12005173> error with LTO and dead strip of non-lazy-pointer
void* foo2p() {
  return &foo2;
}

int main()
{
	t2();
	foo1();
	return 0;
}

