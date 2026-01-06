#include <stddef.h>

#include "foo.h"

void* pf2 = &func2;
int* pd2 = &data2;

int main (void)
{
	if ( &func2 != NULL )
		func2();
	
	if ( &data2 != NULL )
		data2 = 1;
   
   return 0;
}

