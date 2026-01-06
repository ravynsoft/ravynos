#include <stdio.h>
#include <string.h>
#include <errno.h>
extern int foo4();
int foo3()
{
/*       printf ("%s\n",strerror(errno)); */
	return foo4();
}

