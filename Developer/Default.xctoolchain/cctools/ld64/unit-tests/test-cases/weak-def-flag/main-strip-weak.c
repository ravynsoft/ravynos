#include <stdio.h>

// the 'l' prefix makes this an auto-strip symbol
void my_auto_strip_weak() __asm ( "lautostrip" );

void __attribute__((weak)) my_auto_strip_weak()
{

}

int main()
{
	my_auto_strip_weak();
	return 0;
}

