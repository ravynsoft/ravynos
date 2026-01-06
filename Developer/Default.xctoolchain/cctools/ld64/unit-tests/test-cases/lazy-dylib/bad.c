#include <stdio.h>
#include <stdlib.h>


extern int data;

static int* pd = &data;

int main()
{
	return *pd;
}
