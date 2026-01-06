
#include <stdio.h>

typedef int weirdType;
typedef int weirdType2;

#include "foo.h"
#include "bar.h"


int deadwood()
{
	if ( BAR_COUNT1_ENABLED() )
		BAR_COUNT1(2);
	return 0;
}


int main() {
	int a = 1;

	while(a) {
		if ( FOO_COUNT1_ENABLED() )
			FOO_COUNT1(1);
		printf("test\n");
		if ( BAR_COUNT1_ENABLED() )
			BAR_COUNT1(2);
		sleep(1);
	}

	return 0;
}
