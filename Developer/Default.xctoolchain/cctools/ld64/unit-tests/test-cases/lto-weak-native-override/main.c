
#include <stdlib.h>

static void die() { abort(); }


__attribute__((visibility("hidden"),weak)) void foo()
{
	die();
}

int main()
{
   foo();
   return 0;
}

