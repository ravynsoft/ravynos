#include <stddef.h>

#include "foo.h"


int* pdata5 = &data5;
int* pdata6 = &data6;

void* pf3;

int main (void)
{
	// make non-lazy reference to func3 and func4
	pf3 = &func3;
	if ( &func4 == NULL ) {
		// make lazy reference to func1 and func2
		func1();
		func2();
	}
   
   return data1 + data2 + data3 + data4;
}

