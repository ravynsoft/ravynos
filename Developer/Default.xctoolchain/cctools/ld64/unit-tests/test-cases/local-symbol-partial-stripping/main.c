#include <stdio.h>

extern int myglobal;
extern void myfunction(int);

int main()
{
	myfunction(myglobal);
	return 0;
}

