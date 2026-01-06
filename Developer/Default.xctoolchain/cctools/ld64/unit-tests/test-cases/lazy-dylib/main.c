#include <stdio.h>
#include <stdlib.h>


extern int foo();
extern int bar();

int main()
{
	// two regular external function calls
	void* x = malloc(16);
	free(x);
	// two lazy dylib external function calls
	int result = foo();
	fprintf(stderr, "foo() returned %d\n", result);
	bar();
	return 0;
}
