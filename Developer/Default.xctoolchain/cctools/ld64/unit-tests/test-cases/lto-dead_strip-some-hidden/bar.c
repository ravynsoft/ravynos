
int data[] = { 4, 5, 6 };
int deaddata[] = { 7, 8, 9 };

int func() { return 0; }
int deadfunc() { return 0; }

__attribute__((visibility("default")))
int* foo()
{
	return data;
}

__attribute__((visibility("default")))
void* foo2()
{
	return func;
}

