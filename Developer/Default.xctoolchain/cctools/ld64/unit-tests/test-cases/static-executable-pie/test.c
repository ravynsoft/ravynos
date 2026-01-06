
int a;
int b = 5;
int* pa = &a;
int* pb = &b;

int foo()
{
	*pa = 4;
	return a+b;
}


int entry()
{
	return foo();
}


