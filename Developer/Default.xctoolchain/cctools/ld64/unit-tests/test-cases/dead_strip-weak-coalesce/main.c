
// baz is in a lazily loaded archive
extern void baz();

int main()
{
	baz();
	return 0;
}


#include "foo.c"

