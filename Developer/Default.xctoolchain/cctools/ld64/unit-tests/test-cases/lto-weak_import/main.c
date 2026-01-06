
#include <stdlib.h>

extern void foo() __attribute__((weak_import));


int main()
{
	if ( &foo != NULL )
		foo();
	return 0;
}

