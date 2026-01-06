#include <stddef.h>

extern void foo();
extern void foo2() __attribute__((weak_import));
extern void bar();

int main()
{
	foo();
	bar();
	if ( &foo2 != NULL )
		foo2();
	return 0;
}
