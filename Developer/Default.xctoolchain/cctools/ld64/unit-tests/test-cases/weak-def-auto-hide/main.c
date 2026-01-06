#include <stdio.h>

void __attribute__((weak)) my_weak()
{
}

extern void my_other_weak();

int main()
{
	my_weak();
	my_other_weak();
	return 0;
}

