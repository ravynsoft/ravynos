#include <stdio.h>

extern void foo();

int main()
{
	fprintf(stdout, "hello\n");
  foo();
	return 0;
}

void back()
{
}