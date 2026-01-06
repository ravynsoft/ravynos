
#include "foo.h"


int* pdata5 = &data5;
int* pdata6 = &data6;


int main (void)
{
	// make non-lazy reference to func3 and func4
	if ( &func3 == &func4 ) {
		// make lazy reference to func3 and func4
		func1();
		func2();
	}
   
   return data1 + data2 + data3 + data4;
}

