#include <stdio.h>

__attribute__((weak)) void doit()
{
	printf("hello %s\n", "world");
}


void foo() 
{
	doit();
}

