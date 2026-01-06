


__attribute__((weak)) int foo()
{
	return 0;
}


int entry()
{
	return foo();
}

// pointer to weak function might trigger external relocation
void* pfoo = &foo;
