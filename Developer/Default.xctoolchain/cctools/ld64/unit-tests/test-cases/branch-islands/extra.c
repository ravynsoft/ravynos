#include <stdio.h>

extern void back();

void foo()
{
	fprintf(stdout, "foo\n");
  back();
}

