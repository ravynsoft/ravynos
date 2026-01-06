
#include "foo.h"
#include "bar.h"

void* p;

int main (void)
{
	// non-lazy reference to foo2
	p = &foo2;
	// lazy reference to foo4
	foo4();
	
	// non-lazy reference to bar2
	p = &bar2;
	// lazy reference to bar4 and bar1
	bar4();
	bar1();
   
   return 0;
}

